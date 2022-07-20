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
    AttrBitset combined_attrs(
        const Project::MeshObject &mesh_object,
        const Plc3::Surface &surface
    ) const {
        AttrBitset attrs = surface.attrs;
        attrs |= mesh_object.plc->volumes[surface.volumes[0]].attrs;
        attrs |= mesh_object.plc->volumes[surface.volumes[1]].attrs;
        attrs.reset(attr_bit_solid());
        return attrs;
    }

    void calculate_xrays(
        std::set<std::pair<std::string, Plc3::SurfaceId> > *xray_surfaces_out,
        std::set<std::string> *xray_node_object_names_out
    ) const {
        for (const auto &pair : project->mesh_objects) {
            if (!pair.second.plc) {
                continue;
            }
            for (Plc3::SurfaceId sid = 0; sid < static_cast<int>(
                     pair.second.plc->surfaces.size()); ++sid) {
                const Plc3::Surface &surface =
                    pair.second.plc->surfaces[sid];
                AttrBitset attrs = combined_attrs(pair.second, surface);
                bool xray = (pair.first == focus_mesh)
                    || (attrs & focus_attrs).any();
                if (xray) {
                    xray_surfaces_out->insert(std::make_pair(pair.first, sid));
                }
            }
        }

        if (!focus_node_object.empty()) {
            xray_node_object_names_out->insert(focus_node_object);
        }
    }

    void calculate_surface_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out
    ) const {
        const Project::MeshObject &mesh_object =
            project->mesh_objects.at(mesh_object_name);
        const Plc3::Surface &surface = mesh_object.plc->surfaces[surface_id];
        AttrBitset attrs = combined_attrs(mesh_object, surface);
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
        QColor *color_out
    ) const {
        bool focus_vertex = (node_object_name == focus_node_object);
        if (focus_vertex) {
            *color_out = focus_color;
        } else {
            *color_out = QColor(0x99, 0x99, 0x99);
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
    GuiModeInspectMeshCallback() :
        focus_element_begin(ElementId::invalid()),
        focus_element_end(ElementId::invalid())
    { }

    void calculate_xrays(
        FaceSet *xray_faces_out,
        std::set<std::string> *xray_node_object_names_out
    ) const {
        if (focus_element_begin != ElementId::invalid()) {
            for (FaceId face_id : project->mesh_index->unmatched_faces) {
                if (!(face_id.element_id < focus_element_begin) &&
                        face_id.element_id < focus_element_end) {
                    xray_faces_out->faces.insert(face_id);
                }
            }
        } else if (focus_element_set) {
            for (ElementId eid : focus_element_set->elements) {
                const Element3 &element = project->mesh->elements[eid];
                const ElementTypeShape *shape =
                        &element_type_shape(element.type);
                for (int face = 0; face < static_cast<int>(shape->faces.size());
                        ++face) {
                    FaceId face_id(eid, face);
                    FaceId peer = project->mesh_index->matching_face(face_id);
                    if (peer != FaceId::invalid() &&
                            focus_element_set->elements.count(
                                peer.element_id)) {
                        continue;
                    }
                    xray_faces_out->faces.insert(face_id);
                }
            }
        } else if (focus_face_set) {
            *xray_faces_out = *focus_face_set;
        } else if (!focus_node_object.empty()) {
            xray_node_object_names_out->insert(focus_node_object);
        }
    }

    void calculate_face_attributes(
        FaceId face_id,
        NodeId node_id,
        ComplexVector *displacement_out,
        QColor *color_out
    ) const {
        (void)node_id;

        bool focus_surface;
        if (focus_element_begin != ElementId::invalid()) {
            focus_surface = !(face_id.element_id < focus_element_begin) &&
                face_id.element_id < focus_element_end;
        } else if (focus_element_set) {
            focus_surface = focus_element_set->elements.count(face_id.element_id);
        } else if (focus_face_set) {
            focus_surface = focus_face_set->faces.count(face_id);
        } else {
            focus_surface = false;
        }

        bool has_any_attribute = false;
        for (const auto &pair : project->select_volume_objects) {
            if (pair.second.element_set->elements.count(face_id.element_id)) {
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

        *displacement_out = ComplexVector::zero();

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
        ComplexVector *displacement_out,
        QColor *color_out
    ) const {
        bool focus_vertex = (node_object_name == focus_node_object);
        *displacement_out = ComplexVector::zero();
        if (focus_vertex) {
            *color_out = focus_color;
        } else {
            *color_out = QColor(0x99, 0x99, 0x99);
        }
    }

    const Project *project;
    ElementId focus_element_begin, focus_element_end;
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
        callback.focus_element_begin =
            project->mesh_objects.at(focus_name).element_begin;
        callback.focus_element_end =
            project->mesh_objects.at(focus_name).element_end;
        break;
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
