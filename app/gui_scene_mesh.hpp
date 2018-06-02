#ifndef OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiSceneMesh : public GuiSceneAbstract
{
public:
    GuiSceneMesh(
        QWidget *parent,
        GuiSceneSettings *scene_settings);

protected:
    virtual void initialize_scene();

    virtual void calculate_attributes(
        ElementId element_id,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
};

class GuiSceneMeshVolume : public GuiSceneMesh
{
public:
    GuiSceneMeshVolume(
        QWidget *parent,
        GuiSceneSettings *scene_settings,
        const Project::VolumeObjectName &volume);

private:
    virtual void calculate_attributes(
        ElementId element_id,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::shared_ptr<const ElementSet> element_set;
};

class GuiSceneMeshResultDisplacement : public GuiSceneMesh
{
public:
    GuiSceneMeshResultDisplacement(
        QWidget *parent,
        GuiSceneSettings *scene_settings,
        const std::string &result_name);

private:
    virtual void calculate_attributes(
        ElementId element_id,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::string result_name;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_ */
