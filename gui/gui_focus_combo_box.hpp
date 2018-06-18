#ifndef OS2CX_GUI_FOCUS_COMBO_BOX_HPP_
#define OS2CX_GUI_FOCUS_COMBO_BOX_HPP_

#include <functional>

#include <QComboBox>

#include "gui_scene_abstract.hpp"
#include "project.hpp"

namespace os2cx {

class GuiFocusComboBox : public QComboBox
{
    Q_OBJECT
public:
    GuiFocusComboBox(QWidget *parent, const Project *project);

    GuiSceneAbstract *make_focus_scene(
        const GuiSceneAbstract::SceneParams &params);

signals:
    void focus_changed();

public slots:
    void regenerate_options();

private:
    typedef std::function<
        GuiSceneAbstract *(
            const GuiSceneAbstract::SceneParams &
        )> SceneCallback;

    void push_option(const QString &text, const SceneCallback &scene_callback);
    const Project *project;
    std::vector<SceneCallback> scene_callbacks;
};

} /* namespace os2cx */

#endif
