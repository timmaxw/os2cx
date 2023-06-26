#ifndef OS2CX_GUI_MODE_RESULT_HPP_
#define OS2CX_GUI_MODE_RESULT_HPP_

#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>

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


    /* All the steps have the same datasets, so we often use the first step as
    a "prototypical step" to see which datasets exist. */
    const Results::Result::Step *first_step() {
        return &result->steps.front();
    }

    void maybe_setup_frequency();

    void maybe_setup_disp();
    void refresh_animate_hz();

    void set_color_variable(const std::string &new_var);
    void set_color_subvariable(SubVariable new_subvar);

    void maybe_setup_measurements();
    void refresh_measurements();

    void calculate_xrays(
        FaceSet *xray_faces_out,
        std::set<std::string> *xray_node_object_names_out) const;

    void calculate_face_attributes(
        FaceId face_id,
        NodeId node_id,
        ComplexVector *displacement_out,
        QColor *color_out) const;

    void calculate_vertex_attributes(
        const std::string &node_object_name,
        ComplexVector *displacement_out,
        QColor *color_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();

    const Results::Result *result;

    QComboBox *combo_box_frequency;
    int step_index;

    std::string disp_key, dispi_key;
    QComboBox *combo_box_disp_scale;
    double disp_scale;
    QCheckBox *checkbox_animate;
    bool animate_active;
    GuiOpenglScene::AnimateMode animate_mode_if_active;
    double animate_hz;

    QComboBox *combo_box_color_variable;
    std::string color_variable;

    QComboBox *combo_box_color_subvariable;
    SubVariable color_subvariable;

    GuiColorScale *color_scale;

    QTableWidget *measurement_table;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_RESULT_HPP_ */
