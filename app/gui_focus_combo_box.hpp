#ifndef OS2CX_GUI_FOCUS_COMBO_BOX_HPP_
#define OS2CX_GUI_FOCUS_COMBO_BOX_HPP_

#include <QComboBox>

#include "opengl.hpp"

namespace os2cx {

class GuiFocusComboBox : public QComboBox
{
    Q_OBJECT
public:
    GuiFocusComboBox(QWidget *parent, const Project *project);

    OpenglFocus get_focus();

signals:
    void focus_changed();

public slots:
    void regenerate_options();

private:
    void push_option(
        const QString &text,
        OpenglFocus::Type type,
        const std::string &target
    );
    const Project *project;
    std::vector<OpenglFocus> focuses_by_index;
};

} /* namespace os2cx */

#endif
