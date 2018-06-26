#ifndef OS2CX_GUI_COMBO_BOX_SCENES_HPP_
#define OS2CX_GUI_COMBO_BOX_SCENES_HPP_

#include <QComboBox>

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiComboBoxScenes : public QComboBox
{
    Q_OBJECT

public:
    typedef std::vector<
        std::pair<
            QString,
            std::function<GuiSceneAbstract *()>
        > > SceneVector;

    explicit GuiComboBoxScenes(QWidget *parent);

    void set_scenes(const SceneVector &new_scenes);
    void set_current_scene(const QString &name);

signals:
    void current_scene_changed(GuiSceneAbstract *current);
};

} /* namespace os2cx */

#endif // OS2CX_GUI_COMBO_BOX_SCENES_HPP_
