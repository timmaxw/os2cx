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

    if (step.vars.count("DISP")) {
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

void GuiSceneResultStatic::construct_combo_box_disp_scale() {
    create_widget_label(tr("Scale displacement"));
    combo_box_disp_scale = new QComboBox(this);
    layout->addWidget(combo_box_disp_scale);

    double max_exaggeration_factor =
        (project->approx_scale / 3) / variable_maxima["DISP"];
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
    color_scale->set_range(
        GuiColorScale::Anchor::Zero,
        0.0,
        variable_maxima[color_variable]);
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

    double color_value;
    const Results::Variable &color_var = step.vars.at(color_variable);
    if (color_var.node_vector) {
        color_value = (*color_var.node_vector)[node_id].magnitude();
    } else {
        color_value = std::abs((*color_var.node_matrix)[node_id].cols[0].z);
    }
    *color_out = color_scale->color(color_value);

    auto it = step.vars.find("DISP");
    if (it != step.vars.end() && it->second.node_vector) {
        Vector disp = (*it->second.node_vector)[node_id];
        *displacement_out = disp * disp_scale;
    } else {
        *displacement_out = Vector(0, 0, 0);
    }
}

} /* namespace os2cx */
