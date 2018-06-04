#ifndef OS2CX_GUI_FOCUS_COMBO_BOX_HPP_
#define OS2CX_GUI_FOCUS_COMBO_BOX_HPP_

#include <QComboBox>

#include "project.hpp"

namespace os2cx {

class GuiFocus {
public:
    enum class Type {
        All, // no target
        Mesh, // target = mesh name
        SelectVolume, // target = select volume name
        Load, // target = load name
        Result, // target = dataset name
    };

    GuiFocus() : type(Type::All) { }
    bool operator==(const GuiFocus &other) const {
        return type == other.type && target == other.target;
    }
    bool operator!=(const GuiFocus &other) const {
        return !(*this == other);
    }

    Type type;
    std::string target;
};

class GuiFocusComboBox : public QComboBox
{
    Q_OBJECT
public:
    GuiFocusComboBox(QWidget *parent, const Project *project);

    GuiFocus get_focus();

signals:
    void focus_changed();

public slots:
    void regenerate_options();

private:
    void push_option(
        const QString &text,
        GuiFocus::Type type,
        const std::string &target
    );
    const Project *project;
    std::vector<GuiFocus> focuses_by_index;
};

} /* namespace os2cx */

#endif
