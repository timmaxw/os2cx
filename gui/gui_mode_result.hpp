#ifndef OS2CX_GUI_MODE_RESULT_HPP_
#define OS2CX_GUI_MODE_RESULT_HPP_

#include <QComboBox>

#include "gui_color_scale.hpp"
#include "gui_mode_abstract.hpp"
#include "gui_opengl_mesh.hpp"

namespace os2cx {

class GuiModeResult :
    public GuiModeAbstract,
    private GuiOpenglMeshCallback
{
public:
    GuiModeResult(
        QWidget *parent,
        std::shared_ptr<const Project> project,
        const Results::Result *result);

private:
    enum class SubVariable {
        VectorMagnitude, VectorX, VectorY, VectorZ,
        ComplexVectorMagnitude,
        MatrixXX, MatrixYY, MatrixZZ, MatrixXY, MatrixYZ, MatrixZX
    };

    static double subvariable_value(
        const Results::Dataset &dataset,
        SubVariable subvar,
        NodeId node_id);
    static UnitType guess_unit_type(const std::string &dataset_name);

    /* All the steps have the same datasets, so we often use the first step as
    a "prototypical step" to see which datasets exist. */
    const Results::Result::Step *first_step() {
        return &result->steps.front();
    }

    void maybe_setup_frequency();
    void maybe_setup_disp();

    void set_color_variable(const std::string &new_var);
    void set_color_subvariable(SubVariable new_subvar);

    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();

    const Results::Result *result;

    QComboBox *combo_box_frequency;
    int step_index;

    std::string disp_key;
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
