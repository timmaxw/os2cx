#include "gui_mode_result.hpp"

namespace os2cx {

GuiModeResultStatic::GuiModeResultStatic(
    QWidget *parent, const Project *project, const std::string &result_name_
) :
    GuiModeMesh(parent, project), result_name(result_name_)
{
    const Results::StaticStep &step =
        project->results->static_steps.at(result_name);

    for (const auto &pair : step.datasets) {
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
    for (const auto &pair : step.datasets) {
        combo_box_color_variable->addItem(QString(pair.first.c_str()));
    }
    connect(combo_box_color_variable,
        QOverload<int>::of(&QComboBox::activated),
    [this](int new_index) {
        set_color_variable(
            combo_box_color_variable->itemText(new_index).toStdString());
    });

    create_widget_label(tr("Color by subvariable"));
    combo_box_color_subvariable = new QComboBox(this);
    layout->addWidget(combo_box_color_subvariable);
    connect(combo_box_color_subvariable,
        QOverload<int>::of(&QComboBox::activated),
    [this](int new_index) {
        set_color_subvariable(static_cast<SubVariable>(
            combo_box_color_subvariable->itemData(new_index).value<int>()));
    });

    create_widget_label(tr("Color scale"));
    color_scale = new GuiColorScale(this);
    layout->addWidget(color_scale);

    set_color_variable(step.datasets.begin()->first);
}

double GuiModeResultStatic::subvariable_value(
    const Results::Dataset &dataset,
    SubVariable subvar,
    NodeId node_id
) {
    switch (subvar) {
    case SubVariable::VectorMagnitude:
        return (*dataset.node_vector)[node_id].magnitude();
    case SubVariable::VectorX:
        return (*dataset.node_vector)[node_id].x;
    case SubVariable::VectorY:
        return (*dataset.node_vector)[node_id].y;
    case SubVariable::VectorZ:
        return (*dataset.node_vector)[node_id].z;
    case SubVariable::MatrixXX:
        return (*dataset.node_matrix)[node_id].cols[0].x;
    case SubVariable::MatrixYY:
        return (*dataset.node_matrix)[node_id].cols[1].y;
    case SubVariable::MatrixZZ:
        return (*dataset.node_matrix)[node_id].cols[2].z;
    case SubVariable::MatrixXY:
        return (*dataset.node_matrix)[node_id].cols[0].y;
    case SubVariable::MatrixYZ:
        return (*dataset.node_matrix)[node_id].cols[1].z;
    case SubVariable::MatrixZX:
        return (*dataset.node_matrix)[node_id].cols[2].x;
    default: assert(false);
    }
}

void GuiModeResultStatic::construct_combo_box_disp_scale() {
    create_widget_label(tr("Scale displacement"));
    combo_box_disp_scale = new QComboBox(this);
    layout->addWidget(combo_box_disp_scale);

    const Results::StaticStep &step =
        project->results->static_steps.at(result_name);
    const ContiguousMap<NodeId, Vector> &disp =
        *step.datasets.at(step.disp_key).node_vector;
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
        emit refresh_scene();
    });
}

void GuiModeResultStatic::set_color_variable(const std::string &new_var) {
    color_variable = new_var;

    const Results::StaticStep &step =
        project->results->static_steps.at(result_name);
    const Results::Dataset &dataset =
        step.datasets.at(color_variable);

    combo_box_color_subvariable->clear();
    if (dataset.node_vector) {
        combo_box_color_subvariable->addItem(tr("Magnitude"),
            QVariant(static_cast<int>(SubVariable::VectorMagnitude)));
        combo_box_color_subvariable->addItem(tr("X Component"),
            QVariant(static_cast<int>(SubVariable::VectorX)));
        combo_box_color_subvariable->addItem(tr("Y Component"),
            QVariant(static_cast<int>(SubVariable::VectorY)));
        combo_box_color_subvariable->addItem(tr("Z Component"),
            QVariant(static_cast<int>(SubVariable::VectorZ)));
    } else {
        combo_box_color_subvariable->addItem(tr("XX Component"),
            QVariant(static_cast<int>(SubVariable::MatrixXX)));
        combo_box_color_subvariable->addItem(tr("YY Component"),
            QVariant(static_cast<int>(SubVariable::MatrixYY)));
        combo_box_color_subvariable->addItem(tr("ZZ Component"),
            QVariant(static_cast<int>(SubVariable::MatrixZZ)));
        combo_box_color_subvariable->addItem(tr("XY Component"),
            QVariant(static_cast<int>(SubVariable::MatrixXY)));
        combo_box_color_subvariable->addItem(tr("YZ Component"),
            QVariant(static_cast<int>(SubVariable::MatrixYZ)));
        combo_box_color_subvariable->addItem(tr("ZX Component"),
            QVariant(static_cast<int>(SubVariable::MatrixZX)));
    }

    combo_box_color_subvariable->setCurrentIndex(0);
    for (int i = 0; i < combo_box_color_subvariable->count(); ++i) {
        SubVariable i_subvar = static_cast<SubVariable>(
            combo_box_color_subvariable->itemData(i).value<int>());
        if (i_subvar == color_subvariable) {
            combo_box_color_subvariable->setCurrentIndex(i);
            break;
        }
    }
    set_color_subvariable(static_cast<SubVariable>(
        combo_box_color_subvariable->currentData().value<int>()));
}

void GuiModeResultStatic::set_color_subvariable(SubVariable new_subvar) {
    color_subvariable = new_subvar;

    const Results::StaticStep &step =
        project->results->static_steps.at(result_name);
    const Results::Dataset &dataset =
        step.datasets.at(color_variable);

    double min_datum = std::numeric_limits<double>::max();
    double max_datum = std::numeric_limits<double>::min();
    NodeId node_begin = dataset.node_vector ?
        dataset.node_vector->key_begin() : dataset.node_matrix->key_begin();
    NodeId node_end = dataset.node_vector ?
        dataset.node_vector->key_end() : dataset.node_matrix->key_end();
    for (NodeId ni = node_begin; ni != node_end; ++ni) {
        double value = subvariable_value(dataset, color_subvariable, ni);
        min_datum = std::min(min_datum, value);
        max_datum = std::max(max_datum, value);
    }

    GuiColorScale::Anchor anchor;
    if (color_subvariable == SubVariable::VectorMagnitude) {
        anchor = GuiColorScale::Anchor::Zero;
    } else {
        anchor = GuiColorScale::Anchor::Balanced;
    }

    color_scale->set_range(anchor, min_datum, max_datum);
    emit refresh_scene();
}

void GuiModeResultStatic::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    Vector *displacement_out
) {
    (void)element_id;
    (void)face_index;

    const Results::StaticStep &step =
        project->results->static_steps.at(result_name);

    double color_datum = subvariable_value(
        step.datasets.at(color_variable), color_subvariable, node_id);
    *color_out = color_scale->color(color_datum);

    if (!step.disp_key.empty()) {
        Vector disp = (*step.datasets.at(step.disp_key).node_vector)[node_id];
        *displacement_out = disp * disp_scale;
    } else {
        *displacement_out = Vector(0, 0, 0);
    }
}

} /* namespace os2cx */
