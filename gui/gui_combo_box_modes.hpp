#ifndef OS2CX_GUI_COMBO_BOX_MODES_HPP_
#define OS2CX_GUI_COMBO_BOX_MODES_HPP_

#include <QComboBox>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiComboBoxModes : public QComboBox
{
    Q_OBJECT

public:
    typedef std::vector<
        std::pair<
            QString,
            std::function<GuiModeAbstract *()>
        > > ModeVector;

    explicit GuiComboBoxModes(QWidget *parent);

    void set_modes(const ModeVector &new_modes);
    void set_current_mode(const QString &name);

signals:
    void current_mode_changed(GuiModeAbstract *current);
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_COMBO_BOX_MODES_HPP_ */
