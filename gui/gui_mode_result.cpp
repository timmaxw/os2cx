#include "gui_mode_result.hpp"

#include <QHeaderView>

namespace os2cx {

GuiModeResult::GuiModeResult(
    QWidget *parent,
    std::shared_ptr<const Project> project,
    const Results::Result *result
) :
    GuiModeAbstract(parent, project),
    result(result),
    checkbox_animate(nullptr),
    animate_active(false)
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

    maybe_setup_measurements();
}

double GuiModeResult::subvariable_value(
    const Results::Dataset &dataset,
    SubVariable subvar,
    NodeId node_id
) {
    switch (subvar) {
    case SubVariable::ScalarValue:
        return (*dataset.node_scalar)[node_id];
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
    case SubVariable::MatrixVonMisesStress:
        return von_mises_stress((*dataset.node_matrix)[node_id]);
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
    if (dataset_name == "DISP"
            || dataset_name == "DISPI"
            || dataset_name == "PDISP") {
        return UnitType::Length;
    } else if (dataset_name == "STRESS" || dataset_name == "ISTRESS") {
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
        combo_box_frequency = nullptr;
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
        refresh_animate_hz();
        refresh_measurements();
        emit refresh_scene();
    });

    step_index = 0;
}

void GuiModeResult::maybe_setup_disp() {
    if (first_step()->datasets.count("DISP")) {
        disp_key = "DISP";
        if (first_step()->datasets.count("DISPI")) {
            dispi_key = "DISPI";
        } else {
            dispi_key = "";
        }
    } else {
        disp_key = dispi_key = "";
        return;
    }

    create_widget_label(tr("Scale displacement"));
    combo_box_disp_scale = new QComboBox(this);
    layout->addWidget(combo_box_disp_scale);

    /* We want to heuristically suggest reasonable values for the displacement
    exaggeration factor. Typically, we want to choose exaggeration factors such
    that the displacement is around 1/10 to 1/3 of the model's overall scale. */

    /* In a multi-step calculation, the displacement for the different steps
    might differ by an order of magnitude. Start by calculating the displacement
    for each step. */
    double min_disp = std::numeric_limits<double>::max();
    double max_disp = 0;
    for (const Results::Result::Step &step : result->steps) {
        double step_disp = 0;
        const ContiguousMap<NodeId, Vector> *disp =
            step.datasets.at(disp_key).node_vector.get();
        const ContiguousMap<NodeId, Vector> *dispi =
            dispi_key.empty()
            ? nullptr
            : step.datasets.at(dispi_key).node_vector.get();
        for (NodeId ni = disp->key_begin(); ni != disp->key_end(); ++ni) {
            double mag;
            if (dispi == nullptr) {
                mag = (*disp)[ni].magnitude();
            } else {
                mag = ComplexVector((*disp)[ni], (*dispi)[ni]).magnitude();
            }
            if (isnan(mag)) {
                continue;
            }
            step_disp = std::max(step_disp, mag);
        }
        if (step_disp != 0) {
            min_disp = std::min(min_disp, step_disp);
        }
        max_disp = std::max(max_disp, step_disp);
    }

    if (max_disp == 0) {
        /* The displacement is always zero in every step, so the displacement
        exaggeration factor is meaningless. */
        combo_box_disp_scale->addItem(
            tr("N/A (displacement is always zero)"),
            QVariant(static_cast<double>(1)));
        combo_box_disp_scale->setEnabled(false);
        disp_scale = 1.0;
        return;
    }

    /* Choose exaggeration factors that make the min/max disps work out to about
    1/3 the overall size of the model. */
    double max_exaggeration_factor = (project->approx_scale / 3) / min_disp;
    double min_exaggeration_factor = (project->approx_scale / 3) / max_disp;

    /* Discretize the exaggeration factors on a log scale */
    int max_exaggeration_num = round(log10(max_exaggeration_factor) * 3);
    int min_exaggeration_num = round(log10(min_exaggeration_factor) * 3);

    /* Move the minimum two steps down, in case the user wants to view the
    displacement at <1/3 the overall size of the model. */
    min_exaggeration_num -= 2;

    /* Ensure we're never "un-exaggerating" the displacement to be smaller than
    it actually is */
    max_exaggeration_num = std::max(0, max_exaggeration_num);
    min_exaggeration_num = std::max(0, min_exaggeration_num);

    /* We'll let the user pick any discrete exaggeration factor in the range */
    std::set<int> exaggeration_nums;
    for (int i = min_exaggeration_num; i <= max_exaggeration_num; ++i) {
        exaggeration_nums.insert(i);
    }

    /* Make sure to allow "no exaggeration" as an option */
    exaggeration_nums.insert(0);

    /* Default to "no exaggeration", to avoid misleading novice users */
    disp_scale = 1.0;

    /* Also allow a special option of "0x exaggeration", which would correspond
    to an exaggeration number of "-infinity". */
    combo_box_disp_scale->addItem(
        tr(u8"0\u00D7 (undisplaced shape)"), QVariant(static_cast<double>(0)));

    for (int exaggeration_num : exaggeration_nums) {
        /* Convert exaggeration nums into nice round factors like 2x, 5x, 10x */
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

    checkbox_animate = new QCheckBox(this);
    layout->addWidget(checkbox_animate);
    connect(checkbox_animate, &QCheckBox::stateChanged,
    [this](int new_state) {
        animate_active = (new_state == Qt::Checked);
        emit refresh_scene();
    });
    if (combo_box_frequency == nullptr) {
        checkbox_animate->setText("Animate");
        animate_mode_if_active = GuiOpenglScene::AnimateMode::Sawtooth;
        animate_hz = 1.0;
    } else {
        refresh_animate_hz();
        animate_mode_if_active = GuiOpenglScene::AnimateMode::Sine;
    }
}

void GuiModeResult::refresh_animate_hz() {
    if (checkbox_animate == nullptr) {
        return;
    }

    double true_hz = result->steps[step_index].frequency;
    if (true_hz > 5) {
        double divider = 1;
        while (true_hz / divider > 5) divider *= 10;
        animate_hz = true_hz / divider;
        checkbox_animate->setText(
            tr(u8"Animate (%1\u00D7 slowed)").arg(divider));
    } else if (true_hz < 0.2) {
        double multiplier = 1;
        while (true_hz * multiplier < 0.2) multiplier *= 10;
        animate_hz = true_hz * multiplier;
        checkbox_animate->setText(
            tr(u8"Animate (%1\u00D7 sped up)").arg(multiplier));
    } else {
        animate_hz = true_hz;
        checkbox_animate->setText(tr(u8"Animate (real-time)"));
    }
}

void GuiModeResult::set_color_variable(const std::string &new_var) {
    color_variable = new_var;

    const Results::Dataset &dataset = first_step()->datasets.at(color_variable);

    combo_box_color_subvariable->clear();
    if (dataset.node_scalar) {
        combo_box_color_subvariable->addItem(tr("Value"),
            QVariant(static_cast<int>(SubVariable::ScalarValue)));
    } else if (dataset.node_vector) {
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
        if (color_variable == "STRESS" || color_variable == "ISTRESS") {
            combo_box_color_subvariable->addItem(tr("Von Mises Stress"),
                QVariant(static_cast<int>(SubVariable::MatrixVonMisesStress)));
        }
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
            double datum = subvariable_value(dataset, color_subvariable, ni);
            if (!isnan(datum)) {
                min_datum = std::min(min_datum, datum);
                max_datum = std::max(max_datum, datum);
            }
        }
    }

    if (min_datum == std::numeric_limits<double>::max() &&
            max_datum == std::numeric_limits<double>::lowest()) {
        /* All values in dataset are NaN; just make up some dummy values */
        min_datum = -1;
        max_datum = 1;
    }

    if (color_subvariable == SubVariable::VectorMagnitude ||
            color_subvariable == SubVariable::ComplexVectorMagnitude ||
            color_subvariable == SubVariable::MatrixVonMisesStress) {
        min_datum = 0;
    }

    color_scale->set_range(
        min_datum,
        max_datum,
        &project->unit_system,
        guess_unit_type(color_variable));

    emit refresh_scene();
}

void GuiModeResult::maybe_setup_measurements() {
    if (project->measure_objects.empty()) {
        return;
    }

    create_widget_label(tr("Measurements"));
    measurement_table = new QTableWidget(0, 2, this);
    measurement_table->verticalHeader()->hide();
    measurement_table->horizontalHeader()->hide();
    measurement_table->setSelectionMode(QAbstractItemView::NoSelection);
    measurement_table->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::Stretch);
    measurement_table->horizontalHeader()
        ->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    measurement_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    measurement_table->setDragEnabled(false);
    layout->addWidget(measurement_table);

    refresh_measurements();
}

void GuiModeResult::refresh_measurements() {
    if (project->measure_objects.empty()) {
        return;
    }

    const Results::Result::Step &step = result->steps[step_index];

    measurement_table->setRowCount(project->measure_objects.size());
    int row = 0;
    for (const auto &measure_pair : project->measure_objects) {
        measurement_table->setItem(row, 0, new QTableWidgetItem(
            QString(measure_pair.first.c_str())));

        double max_datum;
        auto dataset_it = step.datasets.find(measure_pair.second.dataset);
        if (dataset_it == step.datasets.end()) {
            max_datum = NAN;
        } else {
            max_datum = 0;
            const Results::Dataset &dataset = dataset_it->second;
            SubVariable measure_subvariable;
            if (dataset.node_scalar) {
                measure_subvariable = SubVariable::ScalarValue;
            } else if (dataset.node_vector) {
                measure_subvariable = SubVariable::VectorMagnitude;
            } else if (dataset.node_complex_vector) {
                measure_subvariable = SubVariable::ComplexVectorMagnitude;
            } else if (dataset.node_matrix) {
                measure_subvariable = SubVariable::MatrixVonMisesStress;
            } else {
                assert(false);
            }

            std::string subject = measure_pair.second.subject;
            std::shared_ptr<const NodeSet> node_set;
            const Project::VolumeObject *volume;
            const Project::SurfaceObject *surface;
            const Project::NodeObject *node;
            if ((volume = project->find_volume_object(subject))) {
                node_set = volume->node_set;
            } else if ((surface = project->find_surface_object(subject))) {
                node_set = surface->node_set;
            } else if ((node = project->find_node_object(subject))) {
                node_set.reset(new NodeSet(
                    compute_node_set_singleton(node->node_id)));
            }

            for (NodeId node_id : node_set->nodes) {
                double datum = subvariable_value(
                    dataset,
                    measure_subvariable,
                    node_id);
                if (isnan(datum)) {
                    max_datum = NAN;
                    break;
                } else {
                    max_datum = std::max(datum, max_datum);
                }
            }
        }

        if (isnan(max_datum)) {
            measurement_table->setItem(row, 1, new QTableWidgetItem(tr("N/A")));
        } else {
            Unit unit = project->unit_system.suggest_unit(
                guess_unit_type(measure_pair.second.dataset),
                max_datum);
            WithUnit<double> max_datum_2 =
                project->unit_system.system_to_unit(unit, max_datum);
            measurement_table->setItem(row, 1, new QTableWidgetItem(
                QString("%1%2")
                    .arg(max_datum_2.value_in_unit)
                    .arg(unit.name.c_str())));
        }
        ++row;
    }
}

void GuiModeResult::calculate_face_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    ComplexVector *displacement_out
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

    if (!dispi_key.empty()) {
        Vector disp = (*step.datasets.at(disp_key).node_vector)[node_id];
        Vector dispi = (*step.datasets.at(dispi_key).node_vector)[node_id];
        if (isnan(disp.x) || isnan(disp.y) || isnan(disp.z)
            || isnan(dispi.x) || isnan(dispi.y) || isnan(dispi.z)) {
            *displacement_out = ComplexVector::zero();
        } else {
            *displacement_out = ComplexVector(disp, dispi) * disp_scale;
        }
    } else if (!disp_key.empty()) {
        Vector disp = (*step.datasets.at(disp_key).node_vector)[node_id];
        if (isnan(disp.x) || isnan(disp.y) || isnan(disp.z)) {
            *displacement_out = ComplexVector::zero();
        } else {
            *displacement_out =
                ComplexVector(disp * disp_scale, Vector::zero());
        }
    } else {
        *displacement_out = ComplexVector::zero();
    }
}

void GuiModeResult::calculate_vertex_attributes(
    const std::string &node_object_name,
    QColor *vertex_color_out,
    bool *xray_out,
    ComplexVector *displacement_out
) const {
    NodeId node_id = project->find_node_object(node_object_name)->node_id;
    calculate_face_attributes(
        ElementId::invalid(),
        -1,
        node_id,
        vertex_color_out,
        displacement_out);
    *xray_out = false;
}

std::shared_ptr<const GuiOpenglScene> GuiModeResult::make_scene() {
    return gui_opengl_scene_mesh(
        *project,
        this,
        animate_active
            ? animate_mode_if_active
            : GuiOpenglScene::AnimateMode::None,
        animate_hz
    );
}

} /* namespace os2cx */
