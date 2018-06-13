#ifndef OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiScenePoly3 : public GuiSceneAbstract
{
public:
    explicit GuiScenePoly3(const SceneParams &params);

protected:
    virtual void initialize_scene();

    virtual QColor surface_color(
        const Project::MeshObjectName &mesh_object_name,
        Plc3::VolumeId volume_id);
};

class GuiScenePoly3Mesh : public GuiScenePoly3
{
public:
    GuiScenePoly3Mesh(
        const SceneParams &params,
        const Project::MeshObjectName &mesh);

protected:
    virtual QColor surface_color(
        const Project::MeshObjectName &mesh_object_name,
        Plc3::VolumeId volume_id);
    Project::MeshObjectName mesh;
};

class GuiScenePoly3SelectVolume : public GuiScenePoly3
{
public:
    GuiScenePoly3SelectVolume(
        const SceneParams &params,
        const Project::SelectVolumeObjectName &volume);

protected:
    virtual QColor surface_color(
        const Project::MeshObjectName &mesh_object_name,
        Plc3::VolumeId volume_id);
    Plc3::BitIndex bit_index;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_ */
