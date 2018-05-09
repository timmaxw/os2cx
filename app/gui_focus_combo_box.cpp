#include "gui_focus_combo_box.hpp"

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

OpenglFocus GuiFocusComboBox::get_focus() {
    int index = currentIndex();
    if (index == -1) {
        return OpenglFocus();
    } else {
        return focuses_by_index[index];
    }
}

void GuiFocusComboBox::regenerate_options() {
    OpenglFocus old_focus = get_focus();

    clear();
    focuses_by_index.clear();

    push_option(tr("Overview"), OpenglFocus::Type::All, "");

    for (const auto &pair : project->mesh_objects) {
        push_option(
            tr("Mesh ") + QString(pair.first.c_str()),
            OpenglFocus::Type::Mesh,
            pair.first
        );
    }

    for (const auto &pair : project->select_volume_objects) {
        push_option(
            tr("Volume ") + QString(pair.first.c_str()),
            OpenglFocus::Type::SelectVolume,
            pair.first
        );
    }

    for (const auto &pair : project->load_objects) {
        push_option(
            tr("Load ") + QString(pair.first.c_str()),
            OpenglFocus::Type::Load,
            pair.first
        );
    }

    if (project->results) {
        for (const auto &pair : project->results->node_vectors) {
            push_option(
                tr("Result ") + QString(pair.first.c_str()),
                OpenglFocus::Type::Result,
                pair.first
            );
        }
    }

    int new_index = 0;
    for (int i = 0; i < static_cast<int>(focuses_by_index.size()); ++i) {
        if (focuses_by_index[i] == old_focus) {
            new_index = i;
            break;
        }
    }
    setCurrentIndex(new_index);
    if (focuses_by_index[new_index] != old_focus) {
        emit focus_changed();
    }
}

void GuiFocusComboBox::push_option(
    const QString &text,
    OpenglFocus::Type type,
    const std::string &target
) {
    addItem(text);
    OpenglFocus focus;
    focus.type = type;
    focus.target = target;
    focuses_by_index.push_back(focus);
}

} /* namespace os2cx */
