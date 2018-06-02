#ifndef OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiScenePoly3 : public GuiSceneAbstract
{
public:
    GuiScenePoly3(QWidget *parent, GuiSceneSettings *scene_settings);

protected:
    virtual void initialize_scene();

    virtual QColor surface_color(
        const Project::MeshObjectName &mesh_object_name,
        Poly3Map::VolumeId volume_id);
};

class GuiScenePoly3Volume : public GuiScenePoly3
{
public:
    GuiScenePoly3Volume(
        QWidget *parent,
        GuiSceneSettings *scene_settings,
        const Project::VolumeObjectName &volume);

protected:
    virtual QColor surface_color(
        const Project::MeshObjectName &mesh_object_name,
        Poly3Map::VolumeId volume_id);
    Project::VolumeObjectName volume;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_ */
