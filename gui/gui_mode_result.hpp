#ifndef OS2CX_GUI_MODE_RESULT_HPP_
#define OS2CX_GUI_MODE_RESULT_HPP_

#include <QComboBox>

#include "gui_color_scale.hpp"
#include "gui_mode_abstract.hpp"
#include "gui_opengl_mesh.hpp"

namespace os2cx {

class GuiModeResultStatic :
    public GuiModeAbstract,
    private GuiOpenglMeshCallback
{
public:
    GuiModeResultStatic(
        QWidget *parent,
        std::shared_ptr<const Project> project,
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
    static UnitType guess_unit_type(const std::string &dataset_name);

    void construct_combo_box_disp_scale();

    void set_color_variable(const std::string &new_var);
    void set_color_subvariable(SubVariable new_subvar);

    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();

    std::string result_name;

    QComboBox *combo_box_disp_scale;
    double disp_scale;

    QComboBox *combo_box_color_variable;
    std::string color_variable;

    QComboBox *combo_box_color_subvariable;
    SubVariable color_subvariable;

    GuiColorScale *color_scale;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_RESULT_HPP_ */
