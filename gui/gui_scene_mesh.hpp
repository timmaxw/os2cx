#ifndef OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiSceneMesh : public GuiSceneAbstract
{
public:
    using GuiSceneAbstract::GuiSceneAbstract;

protected:
    void initialize_scene();

    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
};

class GuiSceneMeshResultDisplacement : public GuiSceneMesh
{
public:
    GuiSceneMeshResultDisplacement(
        QWidget *parent,
        const Project *project,
        const std::string &result_name);

private:
    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::string result_name;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_ */
