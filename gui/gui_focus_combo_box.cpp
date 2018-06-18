#include "gui_focus_combo_box.hpp"

#include "gui_scene_mesh.hpp"
#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiFocusComboBox::GuiFocusComboBox(QWidget *parent, const Project *p) :
    QComboBox(parent), project(p)
{
    setSizeAdjustPolicy(AdjustToContents);

    regenerate_options();

    connect(this, QOverload<int>::of(&QComboBox::activated),
        [this](int) {
            emit focus_changed();
        }
    );
}

GuiSceneAbstract *GuiFocusComboBox::make_focus_scene(
    const GuiSceneAbstract::SceneParams &params
) {
    int index = currentIndex();
    if (index == -1) {
        return new GuiScenePoly3(params);
    } else {
        return scene_callbacks[index](params);
    }
}

void GuiFocusComboBox::regenerate_options() {
    clear();
    scene_callbacks.clear();

    push_option(
        tr("Pre-meshing geometry"),
        [](const GuiSceneAbstract::SceneParams &params) {
            return new GuiScenePoly3(params);
        }
    );

    push_option(
        tr("Post-meshing geometry"),
        [](const GuiSceneAbstract::SceneParams &params) {
            return new GuiSceneMesh(params);
        }
    );

    if (project->results) {
        for (const auto &pair : project->results->node_vectors) {
            std::string name = pair.first;
            push_option(
                tr("Result ") + QString(pair.first.c_str()),
                [name](const GuiSceneAbstract::SceneParams &params) {
                    return new GuiSceneMeshResultDisplacement(params, name);
                }
            );
        }
    }
}

void GuiFocusComboBox::push_option(
    const QString &text,
    const SceneCallback &scene_callback
) {
    addItem(text);
    scene_callbacks.push_back(scene_callback);
}

} /* namespace os2cx */
