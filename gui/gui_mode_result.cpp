#include "gui_mode_result.hpp"

namespace os2cx {

GuiModeResult::GuiModeResult(
    QWidget *parent,
    std::shared_ptr<const Project> project,
    const Results::Result *result
) :
    GuiModeAbstract(parent, project), result(result)
{
    maybe_setup_frequency();

    maybe_setup_disp();

    create_widget_label(tr("Color by variable"));
    combo_box_color_variable = new QComboBox(this);
    layout->addWidget(combo_box_color_variable);
    for (const auto &pair : first_step()->datasets) {
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

    set_color_variable(first_step()->datasets.begin()->first);
}

double GuiModeResult::subvariable_value(
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
    case SubVariable::ComplexVectorMagnitude:
        return (*dataset.node_complex_vector)[node_id].magnitude();
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

UnitType GuiModeResult::guess_unit_type(
    const std::string &dataset_name
) {
    if (dataset_name == "DISP") {
        return UnitType::Length;
    } else if (dataset_name == "STRESS") {
        return UnitType::Pressure;
    } else {
        /* This is a sane fallback for values of any unit type, because they
        will simply be printed unscaled (i.e. in the project's unit system) with
        no unit label attached, which is guaranteed correct, even if vague. */
        return UnitType::Dimensionless;
    }
}

void GuiModeResult::maybe_setup_frequency() {
    if (result->type != Results::Result::Type::Eigenmode &&
            result->type != Results::Result::Type::ModalDynamic) {
        step_index = 0;
        return;
    }

    create_widget_label(tr("Frequency"));
    combo_box_frequency = new QComboBox(this);
    layout->addWidget(combo_box_frequency);

    for (const Results::Result::Step &step : result->steps) {
        double period = 1 / step.frequency;
        Unit unit_s("s", UnitType::Time, 1.0, Unit::Style::Metric);
        WithUnit<double> period_in_s =
            project->unit_system.system_to_unit(unit_s, period);
        double frequency_in_Hz = 1 / period_in_s.value_in_unit;
        combo_box_frequency->addItem(
            tr("%1 Hz").arg(frequency_in_Hz));
    }

    connect(combo_box_frequency, QOverload<int>::of(&QComboBox::activated),
    [this](int new_index) {
        step_index = new_index;
        emit refresh_scene();
    });

    step_index = 0;
}

void GuiModeResult::maybe_setup_disp() {
    if (first_step()->datasets.count("DISP")) {
        disp_key = "DISP";
    } else {
        disp_key = "";
        return;
    }

    create_widget_label(tr("Scale displacement"));
    combo_box_disp_scale = new QComboBox(this);
    layout->addWidget(combo_box_disp_scale);

    double max_disp = 0;
    for (const Results::Result::Step &step : result->steps) {
        const ContiguousMap<NodeId, Vector> &disp =
            *step.datasets.at(disp_key).node_vector;
        for (NodeId ni = disp.key_begin(); ni != disp.key_end(); ++ni) {
            max_disp = std::max(max_disp, disp[ni].magnitude());
        }
    }

    if (max_disp == 0) {
        combo_box_disp_scale->addItem(
            tr("N/A (displacement is zero everywhere))"),
            QVariant(static_cast<double>(1)));
        combo_box_disp_scale->setEnabled(false);
        disp_scale = 1.0;
        return;
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
        tr(u8"0\u00D7 (undisplaced shape)"), QVariant(static_cast<double>(0)));

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
            text = tr(u8"1\u00D7 (actual displacement)");
        } else if (exp_part >= 0) {
            text = tr(u8"%1%2\u00D7")
                .arg(digit)
                .arg(QString("0").repeated(exp_part));
        } else {
            text = tr(u8".%1%2\u00D7")
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

void GuiModeResult::set_color_variable(const std::string &new_var) {
    color_variable = new_var;

    const Results::Dataset &dataset = first_step()->datasets.at(color_variable);

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
    } else if (dataset.node_complex_vector) {
        combo_box_color_subvariable->addItem(tr("Magnitude"),
            QVariant(static_cast<int>(SubVariable::ComplexVectorMagnitude)));
    } else if (dataset.node_matrix) {
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
    } else {
        assert(false);
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

void GuiModeResult::set_color_subvariable(SubVariable new_subvar) {
    color_subvariable = new_subvar;

    double min_datum = std::numeric_limits<double>::max();
    double max_datum = std::numeric_limits<double>::lowest();
    for (const Results::Result::Step &step : result->steps) {
        const Results::Dataset &dataset = step.datasets.at(color_variable);
        for (NodeId ni = dataset.node_begin(); ni != dataset.node_end(); ++ni) {
            double value = subvariable_value(dataset, color_subvariable, ni);
            min_datum = std::min(min_datum, value);
            max_datum = std::max(max_datum, value);
        }
    }

    if (color_subvariable == SubVariable::VectorMagnitude) {
        min_datum = 0;
    }

    color_scale->set_range(
        min_datum,
        max_datum,
        &project->unit_system,
        guess_unit_type(color_variable));

    emit refresh_scene();
}

void GuiModeResult::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    Vector *displacement_out
) const {
    (void)element_id;
    (void)face_index;

    const Results::Result::Step &step = result->steps[step_index];

    double color_datum = subvariable_value(
        step.datasets.at(color_variable), color_subvariable, node_id);
    if (isnan(color_datum)) {
        *color_out = QColor(30, 30, 30);
    } else {
        *color_out = color_scale->color(color_datum);
    }

    if (!disp_key.empty()) {
        Vector disp = (*step.datasets.at(disp_key).node_vector)[node_id];
        if (isnan(disp.x) || isnan(disp.y) || isnan(disp.z)) {
            *displacement_out = Vector::zero();
        } else {
            *displacement_out = disp * disp_scale;
        }
    } else {
        *displacement_out = Vector::zero();
    }
}

std::shared_ptr<const GuiOpenglScene> GuiModeResult::make_scene() {
    return gui_opengl_scene_mesh(*project, this);
}

} /* namespace os2cx */
