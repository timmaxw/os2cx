#include "gui_mode_inspect.hpp"

#include "gui_opengl_mesh.hpp"
#include "gui_opengl_poly3.hpp"

namespace os2cx {

GuiModeInspect::GuiModeInspect(
    QWidget *parent,
    std::shared_ptr<const Project> project
) :
    GuiModeAbstract(parent, project)
{
    create_widget_label(tr("Geometry"));

    radiobutton_poly3 = new QRadioButton(this);
    layout->addWidget(radiobutton_poly3);
    connect(
        radiobutton_poly3, &QRadioButton::toggled,
        [this]() { emit refresh_scene(); });

    radiobutton_mesh = new QRadioButton(this);
    layout->addWidget(radiobutton_mesh);
    connect(
        radiobutton_mesh, &QRadioButton::toggled,
        [this]() { emit refresh_scene(); });

    create_widget_label(tr("Entities"));

    list = new QListWidget(this);
    layout->addWidget(list, 0);

    for (const auto &pair : project->mesh_objects) {
        add_focus(tr("Mesh '%1'").arg(pair.first.c_str()),
            Focus::Mesh, pair.first);
    }
    for (const auto &pair : project->select_volume_objects) {
        add_focus(tr("Volume '%1'").arg(pair.first.c_str()),
            Focus::SelectVolume, pair.first);
    }
    for (const auto &pair : project->select_surface_objects) {
        add_focus(tr("Surface '%1'").arg(pair.first.c_str()),
            Focus::SelectSurface, pair.first);
    }
    for (const auto &pair : project->select_node_objects) {
        add_focus(tr("Node '%1'").arg(pair.first.c_str()),
            Focus::SelectOrCreateNode, pair.first);
    }
    for (const auto &pair : project->create_node_objects) {
        add_focus(tr("Node '%1'").arg(pair.first.c_str()),
            Focus::SelectOrCreateNode, pair.first);
    }
    for (const auto &pair : project->load_volume_objects) {
        add_focus(tr("Load '%1'").arg(pair.first.c_str()),
            Focus::LoadVolume, pair.first);
    }
    for (const auto &pair : project->load_surface_objects) {
        add_focus(tr("Load '%1'").arg(pair.first.c_str()),
            Focus::LoadSurface, pair.first);
    }

    connect(
        list, &QListWidget::currentItemChanged,
                this, &GuiModeInspect::current_item_changed);
    current_item_changed(nullptr, nullptr);

    /* Update to reflect project's initial state */
    project_updated();
}

void GuiModeInspect::project_updated() {
    if (project->progress < Project::Progress::PolyAttrsDone) {
        radiobutton_poly3->setEnabled(false);
        radiobutton_poly3->setText(tr("OpenSCAD (calculating...)"));
    } else {
        radiobutton_poly3->setEnabled(true);
        radiobutton_poly3->setText(tr("OpenSCAD"));

        if (!radiobutton_poly3->isChecked() &&
                !radiobutton_mesh->isChecked()) {
            radiobutton_poly3->setChecked(true);
        }
    }

    if (project->progress < Project::Progress::MeshAttrsDone) {
        radiobutton_mesh->setEnabled(false);
        radiobutton_mesh->setText(tr("Mesh (calculating...)"));
    } else {
        radiobutton_mesh->setEnabled(true);
        radiobutton_mesh->setText(tr("Mesh"));
    }
}

void GuiModeInspect::add_focus(
    const QString &text,
    Focus ft,
    const std::string &fn
) {
    QListWidgetItem *item = new QListWidgetItem(text, list);
    item->setData(focus_type_role, QVariant(static_cast<int>(ft)));
    item->setData(focus_name_role, QVariant(QString::fromStdString(fn)));
}

void GuiModeInspect::current_item_changed(
    QListWidgetItem *new_item,
    QListWidgetItem *old_item
) {
    (void)old_item;
    if (new_item == nullptr) {
        focus_type = Focus::None;
    } else {
        focus_type = static_cast<Focus>(
            new_item->data(focus_type_role).toInt());
        focus_name = new_item->data(focus_name_role)
            .toString().toStdString();
    }

    switch (focus_type) {
    case Focus::None:
        focus_color = QColor(0x00, 0x00, 0x00);
        break;
    case Focus::Mesh:
        focus_color = QColor(0x00, 0x00, 0xFF);
        break;
    case Focus::SelectVolume:
    case Focus::SelectSurface:
    case Focus::SelectOrCreateNode:
        focus_color = QColor(0x00, 0xFF, 0x00);
        break;
    case Focus::LoadVolume:
    case Focus::LoadSurface:
        focus_color = QColor(0xFF, 0x00, 0x00);
        break;
    default: assert(false);
    }

    emit refresh_scene();
}

class GuiModeInspectPoly3Callback : public GuiOpenglPoly3Callback {
public:
    void calculate_surface_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out
    ) const {
        const Project::MeshObject &mesh_object =
            project->mesh_objects.at(mesh_object_name);
        const Plc3::Surface &surface = mesh_object.plc->surfaces[surface_id];
        AttrBitset attrs = surface.attrs;
        attrs |= mesh_object.plc->volumes[surface.volumes[0]].attrs;
        attrs |= mesh_object.plc->volumes[surface.volumes[1]].attrs;
        attrs.reset(attr_bit_solid());

        bool focus_surface = (mesh_object_name == focus_mesh)
            || (attrs & focus_attrs).any();
        bool has_any_attribute = attrs.any();

        if (focus_surface) {
            *color_out = focus_color;
        } else {
            *color_out = QColor(0xAA, 0xAA, 0xAA);
        }

        if (has_any_attribute) {
            *color_out = color_out->darker(120);
        }
    }

    void calculate_vertex_attributes(
        const std::string &node_object_name,
        QColor *vertex_color_out,
        bool *xray_out
    ) const {
        bool focus_vertex = (node_object_name == focus_node_object);
        if (focus_vertex) {
            *vertex_color_out = focus_color;
            *xray_out = true;
        } else {
            *vertex_color_out = QColor(0x99, 0x99, 0x99);
            *xray_out = false;
        }
    }

    const Project *project;
    std::string focus_mesh;
    AttrBitset focus_attrs;
    std::string focus_node_object;
    QColor focus_color;
};

std::shared_ptr<const GuiOpenglScene> GuiModeInspect::make_scene_poly3() {
    GuiModeInspectPoly3Callback callback;
    callback.project = project.get();
    callback.focus_color = focus_color;

    switch (focus_type) {
    case Focus::None:
        break;
    case Focus::Mesh:
        callback.focus_mesh = focus_name;
        break;
    case Focus::SelectVolume:
        callback.focus_attrs[
            project->select_volume_objects.at(focus_name).bit_index] = true;
        break;
    case Focus::SelectSurface:
        callback.focus_attrs[
            project->select_surface_objects.at(focus_name).bit_index] = true;
        break;
    case Focus::SelectOrCreateNode:
        callback.focus_node_object = focus_name;
        break;
    case Focus::LoadVolume: {
        Project::VolumeObjectName volume =
            project->load_volume_objects.at(focus_name).volume;
        if (project->mesh_objects.count(volume)) {
            callback.focus_mesh = volume;
        } else {
            callback.focus_attrs[
                project->select_volume_objects.at(volume).bit_index] = true;
        }
        break;
    }
    case Focus::LoadSurface: {
        Project::SurfaceObjectName surface =
            project->load_surface_objects.at(focus_name).surface;
        callback.focus_attrs[
            project->select_surface_objects.at(surface).bit_index] = true;
        break;
    }
    default: assert(false);
    }

    return gui_opengl_scene_poly3(*project, &callback);
}

class GuiModeInspectMeshCallback : public GuiOpenglMeshCallback {
public:
    void calculate_face_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        ComplexVector *displacement_out
    ) const {
        (void)node_id;

        FaceId face_id(element_id, face_index);
        bool focus_surface =
            (focus_element_set && focus_element_set->elements.count(element_id))
            || (focus_face_set && focus_face_set->faces.count(face_id));

        bool has_any_attribute = false;
        for (const auto &pair : project->select_volume_objects) {
            if (pair.second.element_set->elements.count(element_id)) {
                has_any_attribute = true;
                break;
            }
        }
        for (const auto &pair : project->select_surface_objects) {
            if (pair.second.face_set->faces.count(face_id)) {
                has_any_attribute = true;
                break;
            }
        }

        if (focus_surface) {
            *color_out = focus_color;
        } else {
            *color_out = QColor(0xAA, 0xAA, 0xAA);
        }

        if (has_any_attribute) {
            *color_out = color_out->darker(120);
        }

        *displacement_out = ComplexVector::zero();
    }

    void calculate_vertex_attributes(
        const std::string &node_object_name,
        QColor *vertex_color_out,
        bool *xray_out,
        ComplexVector *displacement_out
    ) const {
        bool focus_vertex = (node_object_name == focus_node_object);
        if (focus_vertex) {
            *vertex_color_out = focus_color;
            *xray_out = true;
        } else {
            *vertex_color_out = QColor(0x99, 0x99, 0x99);
            *xray_out = false;
        }
        *displacement_out = ComplexVector::zero();
    }

    const Project *project;
    std::shared_ptr<const ElementSet> focus_element_set;
    std::shared_ptr<const FaceSet> focus_face_set;
    std::string focus_node_object;
    QColor focus_color;
};

std::shared_ptr<const GuiOpenglScene> GuiModeInspect::make_scene_mesh() {
    GuiModeInspectMeshCallback callback;
    callback.project = project.get();
    callback.focus_color = focus_color;

    switch (focus_type) {
    case Focus::None:
        break;
    case Focus::Mesh:
    case Focus::SelectVolume:
        callback.focus_element_set =
            project->find_volume_object(focus_name)->element_set;
        break;
    case Focus::SelectSurface:
        callback.focus_face_set =
            project->find_surface_object(focus_name)->face_set;
        break;
    case Focus::SelectOrCreateNode:
        callback.focus_node_object = focus_name;
        break;
    case Focus::LoadVolume:
        callback.focus_element_set =
            project->find_volume_object(
                project->load_volume_objects.at(focus_name).volume
            )->element_set;
        break;
    case Focus::LoadSurface:
        callback.focus_face_set =
            project->find_surface_object(
                project->load_surface_objects.at(focus_name).surface
            )->face_set;
        break;
    default: assert(false);
    }

    return gui_opengl_scene_mesh(*project, &callback);
}

std::shared_ptr<const GuiOpenglScene> GuiModeInspect::make_scene() {
    if (radiobutton_poly3->isChecked()) {
        return make_scene_poly3();
    } else if (radiobutton_mesh->isChecked()) {
        return make_scene_mesh();
    } else {
        return std::make_shared<const GuiOpenglScene>();
    }
}

} /* namespace os2cx */
