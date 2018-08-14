#ifndef GUI_COLOR_SCALE_HPP
#define GUI_COLOR_SCALE_HPP

#include <QWidget>

#include "units.hpp"

namespace os2cx {

class GuiColorScale : public QWidget
{
    Q_OBJECT
public:
    explicit GuiColorScale(QWidget *parent = nullptr);

    void set_range(
        double val_min,
        double val_max,
        const UnitSystem *unit_system,
        UnitType unit_type);

    QColor color(double val) const;

signals:

public slots:

private:
    static const int bar_size_px = 20;

    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);

    double range_min, range_max;
    const UnitSystem *unit_system;
    UnitType unit_type;

    std::map<double, QColor> colors;
};

} /* namespace os2cx */

#endif // GUI_COLOR_SCALE_HPP
