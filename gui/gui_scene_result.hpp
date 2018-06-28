#ifndef OS2CX_GUI_SCENE_RESULT_HPP_
#define OS2CX_GUI_SCENE_RESULT_HPP_

#include <QComboBox>

#include "gui_color_scale.hpp"
#include "gui_scene_mesh.hpp"

namespace os2cx {

class GuiSceneResultDisplacement : public GuiSceneMesh
{
public:
    GuiSceneResultDisplacement(
        QWidget *parent,
        const Project *project,
        const std::string &result_name);

private:
    void construct_combo_box_exaggeration();

    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out);

    std::string result_name;
    double max_displacement;

    QComboBox *combo_box_exaggeration;
    double displacement_exaggeration;

    GuiColorScale *color_scale;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_SCENE_RESULT_HPP_ */
