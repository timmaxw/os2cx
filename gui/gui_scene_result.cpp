#include "gui_scene_result.hpp"

namespace os2cx {

GuiSceneResultStatic::GuiSceneResultStatic(
    QWidget *parent, const Project *project, const std::string &result_name_
) :
    GuiSceneMesh(parent, project), result_name(result_name_)
{
    const Results::VariableSet &step =
        project->results->static_steps.at(result_name);

    for (const auto &pair : step.vars) {
        double maximum = 0;
        if (pair.second.node_vector) {
            const ContiguousMap<NodeId, Vector> &nv = *pair.second.node_vector;
            for (NodeId ni = nv.key_begin(); ni != nv.key_end(); ++ni) {
                maximum = std::max(maximum, nv[ni].magnitude());
            }
        } else if (pair.second.node_matrix) {
            const ContiguousMap<NodeId, Matrix> &nm = *pair.second.node_matrix;
            for (NodeId ni = nm.key_begin(); ni != nm.key_end(); ++ni) {
                maximum = std::max(maximum, std::abs(nm[ni].cols[0].z));
            }
        }
        variable_maxima[pair.first] = maximum;
    }

    if (!step.disp_key.empty()) {
        construct_combo_box_disp_scale();
    }

    create_widget_label(tr("Color by variable"));
    combo_box_color_variable = new QComboBox(this);
    layout->addWidget(combo_box_color_variable);
    for (const auto &pair : step.vars) {
        combo_box_color_variable->addItem(QString(pair.first.c_str()));
    }
    connect(combo_box_color_variable,
        QOverload<int>::of(&QComboBox::activated),
    [this](int new_index) {
        set_color_variable(
            combo_box_color_variable->itemText(new_index).toStdString());
    });

    create_widget_label(tr("Color scale"));
    color_scale = new GuiColorScale(this);
    layout->addWidget(color_scale);

    set_color_variable(step.vars.begin()->first);
}

double GuiSceneResultStatic::subvariable_value(
    const Results::Variable &var,
    SubVariable subvar,
    NodeId node_id
) {
    switch (subvar) {
    case SubVariable::VectorMagnitude:
        return (*var.node_vector)[node_id].magnitude();
    case SubVariable::VectorX:
        return (*var.node_vector)[node_id].x;
    case SubVariable::VectorY:
        return (*var.node_vector)[node_id].y;
    case SubVariable::VectorZ:
        return (*var.node_vector)[node_id].z;
    case SubVariable::MatrixXX:
        return (*var.node_matrix)[node_id].cols[0].x;
    case SubVariable::MatrixYY:
        return (*var.node_matrix)[node_id].cols[1].y;
    case SubVariable::MatrixZZ:
        return (*var.node_matrix)[node_id].cols[2].z;
    case SubVariable::MatrixXY:
        return (*var.node_matrix)[node_id].cols[0].y;
    case SubVariable::MatrixYZ:
        return (*var.node_matrix)[node_id].cols[1].z;
    case SubVariable::MatrixZX:
        return (*var.node_matrix)[node_id].cols[2].x;
    default: assert(false);
    }
}

void GuiSceneResultStatic::construct_combo_box_disp_scale() {
    create_widget_label(tr("Scale displacement"));
    combo_box_disp_scale = new QComboBox(this);
    layout->addWidget(combo_box_disp_scale);

    const Results::VariableSet &step =
        project->results->static_steps.at(result_name);
    const ContiguousMap<NodeId, Vector> &disp =
        *step.vars.at(step.disp_key).node_vector;
    double max_disp;
    for (NodeId ni = disp.key_begin(); ni != disp.key_end(); ++ni) {
        max_disp = std::max(max_disp, disp[ni].magnitude());
    }

    double max_exaggeration_factor = (project->approx_scale / 3) / max_disp;
    if (max_exaggeration_factor < 1) {
        max_exaggeration_factor = 1;
    }

    int max_exaggeration_num = round(log10(max_exaggeration_factor) * 3);
    std::set<int> exaggeration_nums;
    exaggeration_nums.insert(max_exaggeration_num);
    exaggeration_nums.insert(max_exaggeration_num - 1);
    exaggeration_nums.insert(max_exaggeration_num - 2);
    exaggeration_nums.insert(0);

    combo_box_disp_scale->addItem(
        tr("0\u00D7 (undisplaced shape)"), QVariant(static_cast<double>(0)));

    /* Default to not exaggerating, to avoid misleading novice users */
    disp_scale = 1.0;

    for (int exaggeration_num : exaggeration_nums) {
        int exp_part = exaggeration_num / 3;
        int digit_part = exaggeration_num - exp_part * 3;
        if (digit_part < 0) {
            exp_part -= 1;
            digit_part += 3;
        }

        assert(digit_part == 0 || digit_part == 1 || digit_part == 2);
        int digit = (digit_part == 0) ? 1 : ((digit_part == 1) ? 2 : 5);

        double factor = ::pow(10, exp_part) * digit;

        QString text;
        if (exaggeration_num == 0) {
            text = tr("1\u00D7 (actual displacement)");
        } else if (exp_part >= 0) {
            text = tr("%1%2\u00D7")
                .arg(digit)
                .arg(QString("0").repeated(exp_part));
        } else {
            text = tr(".%1%2\u00D7")
                .arg(QString("0").repeated(-exp_part - 1))
                .arg(digit);
        }

        combo_box_disp_scale->addItem(text, QVariant(factor));

        if (exaggeration_num == 0) {
            combo_box_disp_scale->setCurrentIndex(
                combo_box_disp_scale->count() - 1);
        }
    }

    connect(combo_box_disp_scale, QOverload<int>::of(&QComboBox::activated),
    [this](int new_index) {
        disp_scale =
            combo_box_disp_scale->itemData(new_index).value<double>();
        clear();
        initialize_scene();
        emit rerender();
    });
}

void GuiSceneResultStatic::set_color_variable(const std::string &new_var) {
    color_variable = new_var;

    const Results::VariableSet &step =
        project->results->static_steps.at(result_name);
    const Results::Variable &var =
        step.vars.at(color_variable);
    if (var.node_vector) {
        color_subvariable = SubVariable::VectorMagnitude;
    } else {
        color_subvariable = SubVariable::MatrixXX;
    }

    double min_var = std::numeric_limits<double>::max();
    double max_var = std::numeric_limits<double>::min();
    NodeId node_begin = var.node_vector ?
        var.node_vector->key_begin() : var.node_matrix->key_begin();
    NodeId node_end = var.node_vector ?
        var.node_vector->key_end() : var.node_matrix->key_end();
    for (NodeId ni = node_begin; ni != node_end; ++ni) {
        double value = subvariable_value(var, color_subvariable, ni);
        min_var = std::min(min_var, value);
        max_var = std::max(max_var, value);
    }

    GuiColorScale::Anchor anchor;
    if (color_subvariable == SubVariable::VectorMagnitude) {
        anchor = GuiColorScale::Anchor::Zero;
    } else {
        anchor = GuiColorScale::Anchor::Balanced;
    }

    color_scale->set_range(anchor, min_var, max_var);
    clear();
    initialize_scene();
    emit rerender();
}

void GuiSceneResultStatic::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    Vector *displacement_out
) {
    (void)element_id;
    (void)face_index;

    const Results::VariableSet &step =
        project->results->static_steps.at(result_name);

    double color_value = subvariable_value(
        step.vars.at(color_variable), color_subvariable, node_id);
    *color_out = color_scale->color(color_value);

    if (!step.disp_key.empty()) {
        Vector disp = (*step.vars.at(step.disp_key).node_vector)[node_id];
        *displacement_out = disp * disp_scale;
    } else {
        *displacement_out = Vector(0, 0, 0);
    }
}

} /* namespace os2cx */
