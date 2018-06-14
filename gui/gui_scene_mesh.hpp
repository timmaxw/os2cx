#ifndef OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiSceneMesh : public GuiSceneAbstract
{
public:
    explicit GuiSceneMesh(const SceneParams &params);

protected:
    virtual void initialize_scene();

    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
};

class GuiSceneMeshVolume : public GuiSceneMesh
{
public:
    GuiSceneMeshVolume(
        const SceneParams &params,
        const Project::VolumeObjectName &volume);

private:
    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::shared_ptr<const ElementSet> element_set;
};

class GuiSceneMeshSurface : public GuiSceneMesh
{
public:
    GuiSceneMeshSurface(
        const SceneParams &params,
        const Project::SurfaceObjectName &surface);

private:
    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::shared_ptr<const FaceSet> face_set;
};

class GuiSceneMeshResultDisplacement : public GuiSceneMesh
{
public:
    GuiSceneMeshResultDisplacement(
        const SceneParams &params,
        const std::string &result_name);

private:
    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        PureVector *displacement_out);
    std::string result_name;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MESH_OPENGL_WIDGET_HPP_ */
