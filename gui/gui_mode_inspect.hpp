#ifndef OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_
#define OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_

#include <QListWidget>
#include <QRadioButton>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiModeInspect : public GuiModeAbstract
{
public:
    GuiModeInspect(
        QWidget *parent,
        std::shared_ptr<const Project> project);

public slots:
    void project_updated();

protected:
    friend class GuiModeInspectPoly3Callback;

    enum class Focus {
        None,
        Mesh,
        SelectVolume,
        SelectSurface,
        SelectOrCreateNode,
        LoadVolume,
        LoadSurface
    };
    Focus focus_type;
    std::string focus_name;
    QColor focus_color;

private:
    static const Qt::ItemDataRole focus_type_role =
        static_cast<Qt::ItemDataRole>(Qt::UserRole + 1);
    static const Qt::ItemDataRole focus_name_role =
        static_cast<Qt::ItemDataRole>(Qt::UserRole + 2);

    void add_focus(const QString &text, Focus ft, const std::string &fn);

    void current_item_changed(
        QListWidgetItem *new_item,
        QListWidgetItem *old_item);

    std::shared_ptr<const GuiOpenglScene> make_scene_poly3();
    std::shared_ptr<const GuiOpenglScene> make_scene_mesh();
    std::shared_ptr<const GuiOpenglScene> make_scene();

    QRadioButton *radiobutton_poly3, *radiobutton_mesh;

    QListWidget *list;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_ */
