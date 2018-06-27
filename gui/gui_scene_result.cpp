#include "gui_scene_result.hpp"

namespace os2cx {

GuiSceneResultDisplacement::GuiSceneResultDisplacement(
    QWidget *parent, const Project *project, const std::string &result_name_
) :
    GuiSceneMesh(parent, project), result_name(result_name_)
{
    max_displacement = 0;
    const ContiguousMap<NodeId, PureVector> &result =
        project->results->node_vectors.at(result_name);
    for (NodeId node_id = result.key_begin();
            node_id != result.key_end(); ++node_id) {
        max_displacement =
            std::max(max_displacement, result[node_id].magnitude().val);
    }

    create_widget_label(tr("Displacement"));
    combo_box_exaggeration = new QComboBox(this);
    layout->addWidget(combo_box_exaggeration);
    populate_combo_box_exaggeration();
}

void GuiSceneResultDisplacement::populate_combo_box_exaggeration() {
    double max_exaggeration_factor =
        (project->approx_scale.val / 20) / max_displacement;
    if (max_exaggeration_factor < 1) {
        max_exaggeration_factor = 1;
    }

    int max_exaggeration_num = round(log10(max_exaggeration_factor) * 3);
    std::set<int> exaggeration_nums;
    exaggeration_nums.insert(max_exaggeration_num);
    exaggeration_nums.insert(max_exaggeration_num - 1);
    exaggeration_nums.insert(max_exaggeration_num - 2);
    exaggeration_nums.insert(0);

    combo_box_exaggeration->addItem(
        tr("0\u00D7 (undisplaced shape)"), QVariant(static_cast<double>(0)));

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

        combo_box_exaggeration->addItem(text, QVariant(factor));
    }
}

void GuiSceneResultDisplacement::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    PureVector *displacement_out
) {
    (void)element_id;
    (void)face_index;
    *color_out = QColor(0x80, 0x80, 0x80);
    const ContiguousMap<NodeId, PureVector> &result =
        project->results->node_vectors.at(result_name);
    *displacement_out = result[node_id];
}

} /* namespace os2cx */
