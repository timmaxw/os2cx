#include "gui_combo_box_scenes.hpp"

#include <QStandardItemModel>

namespace os2cx {

GuiComboBoxScenes::GuiComboBoxScenes(QWidget *parent) :
    QComboBox(parent)
{
    connect(this, QOverload<int>::of(&QComboBox::activated),
    [this](int new_current_index) {
        GuiSceneAbstract *new_current_scene =
            itemData(new_current_index).value<GuiSceneAbstract *>();
        emit current_scene_changed(new_current_scene);
    });
}

void GuiComboBoxScenes::set_scenes(
    const SceneVector &new_scenes
) {
    /* Make sure that all activity names are unique, because we use activity
    names as unique identifiers. */
    std::set<QString> new_names;
    for (auto &pair : new_scenes) {
        assert(new_names.count(pair.first) == 0);
        new_names.insert(pair.first);
    }

    QString old_current_name = currentText();
    int new_current_index = -1;

    QStandardItemModel *item_model = static_cast<QStandardItemModel *>(model());

    for (int new_index = 0; new_index < static_cast<int>(new_scenes.size());
            ++new_index) {
        const QString &name = new_scenes[new_index].first;
        QList<QStandardItem *> old_items = item_model->findItems(name);
        if (old_items.empty()) {
            /* There is no existing item, so create and insert a new item */
            GuiSceneAbstract *new_scene = new_scenes[new_index].second
                ? new_scenes[new_index].second()
                : nullptr;
            QStandardItem *new_item = new QStandardItem(name);
            new_item->setData(QVariant::fromValue(new_scene), Qt::UserRole);
            new_item->setEnabled(new_scene != nullptr);
            item_model->insertRow(new_index, new_item);

        } else {
            /* There is an existing item with the same name; we only need to
            make sure it's in the right place in the list and the right
            enabled-ness. */
            assert(old_items.length() == 1);
            QStandardItem *old_item = old_items.first();

            GuiSceneAbstract *old_scene =
                old_item->data(Qt::UserRole).value<GuiSceneAbstract *>();
            if (old_scene == nullptr && new_scenes[new_index].second) {
                GuiSceneAbstract *new_scene = new_scenes[new_index].second();
                old_item->setData(
                    QVariant::fromValue(new_scene), Qt::UserRole);
                old_item->setEnabled(true);
            } else if (old_scene != nullptr && !new_scenes[new_index].second) {
                delete old_scene;
                GuiSceneAbstract *new_scene = nullptr;
                old_item->setData(
                    QVariant::fromValue(new_scene), Qt::UserRole);
                old_item->setEnabled(false);
            }

            int old_index = item_model->indexFromItem(old_item).row();
            if (old_index != new_index) {
                QList<QStandardItem *> old_row = item_model->takeRow(old_index);
                assert(old_row.length() == 1);
                assert(old_row.first() == old_item);
                item_model->insertRow(new_index, old_row);
            }
        }
        if (name == old_current_name) {
            new_current_index = new_index;
        }
    }

    /* Remove any remaining old activities that weren't reused. They will all be
    after the new activities we've just created, so we repeatedly delete the
    item in index 'new_activities.size()'. */
    while (item_model->rowCount() > static_cast<int>(new_scenes.size())) {
        QList<QStandardItem *> old_row =
            item_model->takeRow(new_scenes.size());
        assert(old_row.length() == 1);
        QVariant old_data = old_row.first()->data(Qt::UserRole);
        GuiSceneAbstract *old_scene = old_data.value<GuiSceneAbstract *>();
        delete old_scene; /* note old_scene may be nullptr */
    }

    /* Make sure the same thing as before is still selected */
    if (new_current_index != -1) {
        setCurrentIndex(new_current_index);
    } else {
        /* The thing we had selected before no longer exists; fall back to the
        first thing in the list. */
        setCurrentIndex(0);
        GuiSceneAbstract *new_current_scene =
            currentData().value<GuiSceneAbstract *>();
        emit current_scene_changed(new_current_scene);
    }
}

void GuiComboBoxScenes::set_current_scene(const QString &name) {
    int index = findText(name);
    if (index != currentIndex()) {
        setCurrentIndex(index);
        GuiSceneAbstract *new_current_scene =
            currentData().value<GuiSceneAbstract *>();
        emit current_scene_changed(new_current_scene);
    }
}

} /* namespace os2cx */
