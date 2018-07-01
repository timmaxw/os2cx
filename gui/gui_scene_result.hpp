#ifndef OS2CX_GUI_SCENE_RESULT_HPP_
#define OS2CX_GUI_SCENE_RESULT_HPP_

#include <QComboBox>

#include "gui_color_scale.hpp"
#include "gui_scene_mesh.hpp"

namespace os2cx {

class GuiSceneResultStatic : public GuiSceneMesh
{
public:
    GuiSceneResultStatic(
        QWidget *parent,
        const Project *project,
        const std::string &result_name);

private:
    enum class SubVariable {
        VectorMagnitude, VectorX, VectorY, VectorZ,
        MatrixXX, MatrixYY, MatrixZZ, MatrixXY, MatrixYZ, MatrixZX
    };

    static double subvariable_value(
        const Results::Dataset &dataset,
        SubVariable subvar,
        NodeId node_id);

    void construct_combo_box_disp_scale();

    void set_color_variable(const std::string &new_var);

    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out);

    std::string result_name;
    std::map<std::string, double> variable_maxima;

    QComboBox *combo_box_disp_scale;
    double disp_scale;

    QComboBox *combo_box_color_variable;
    std::string color_variable;

    SubVariable color_subvariable;

    GuiColorScale *color_scale;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_SCENE_RESULT_HPP_ */
