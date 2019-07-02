#include "gui_color_scale.hpp"

#include <assert.h>

#include <QPainter>

namespace os2cx {

GuiColorScale::GuiColorScale(QWidget *parent) :
    QWidget(parent)
{
    static const UnitSystem si_system("m", "kg", "s"); /* placeholder */
    set_range(0.0, 1.0, &si_system, UnitType::Dimensionless);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void GuiColorScale::set_range(
    double val_min,
    double val_max,
    const UnitSystem *new_unit_system,
    UnitType new_unit_type
) {
    assert(val_min <= val_max);
    range_min = val_min;
    range_max = val_max;
    double scale = std::max(std::abs(val_min), std::abs(val_max));
    colors.clear();
    colors[-scale]     = QColor::fromHsl(240, 0xFF, 0x66);
    colors[ scale]     = QColor::fromHsl(  0, 0xFF, 0x66);
    colors[-scale / 3] = QColor::fromHsl(180, 0xEE, 0x99);
    colors[ scale / 3] = QColor::fromHsl( 60, 0xEE, 0x99);
    colors[0]          = QColor::fromHsl(120, 0x00, 0xEE);

    unit_system = new_unit_system;
    unit_type = new_unit_type;

    update();
}

QColor GuiColorScale::color(double val) const {
    std::map<double, QColor>::const_iterator it = colors.lower_bound(val);
    if (it == colors.begin()) {
        /* 'val' is below 'range_min' */
        return it->second;
    } else if (it == colors.end()) {
        /* 'val' is above 'range_max' */
        return colors.rbegin()->second;
    } else {
        double val_above = it->first;
        QColor color_above = it->second;
        --it;
        double val_below = it->first;
        QColor color_below = it->second;
        double f = (val - val_below) / (val_above - val_below);
        return QColor(
            color_below.red() * (1 - f) + color_above.red() * f,
            color_below.green() * (1 - f) + color_above.green() * f,
            color_below.blue() * (1 - f) + color_above.blue() * f);
    }
}

QSize GuiColorScale::sizeHint() const {
    return QSize(200, bar_size_px + fontMetrics().height());
}

void GuiColorScale::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    Unit unit = unit_system->suggest_unit(
        unit_type,
        std::max(std::abs(range_min), std::abs(range_max)));
    WithUnit<double> range_min2 = unit_system->system_to_unit(unit, range_min);
    WithUnit<double> range_max2 = unit_system->system_to_unit(unit, range_max);

    QRect label_rect(0, 0, width(), fontMetrics().height());
    painter.drawText(label_rect, Qt::AlignLeft|Qt::AlignBottom,
        QString("%1%2").arg(range_min2.value_in_unit).arg(unit.name.c_str()));
    painter.drawText(label_rect, Qt::AlignRight|Qt::AlignBottom,
        QString("%1%2").arg(range_max2.value_in_unit).arg(unit.name.c_str()));

    /* Draw the main color gradient */
    QRect bar_rect(0, label_rect.bottom(), width(), bar_size_px);
    painter.setPen(Qt::NoPen);
    QLinearGradient gradient(bar_rect.left(), 0, bar_rect.right(), 0);
    gradient.setColorAt(0.0, color(range_min));
    for (const auto &pair : colors) {
        if (range_max == range_min) continue;
        double normalized = (pair.first - range_min) / (range_max - range_min);
        if (normalized <= 0.0 || normalized >= 1.0) continue;
        gradient.setColorAt(normalized, pair.second);
    }
    gradient.setColorAt(1.0, color(range_max));
    painter.setBrush(gradient);
    painter.drawRoundedRect(bar_rect, bar_size_px / 10, bar_size_px / 10);

    /* Draw shading on top of the main color gradient */
    QLinearGradient vgradient(0, bar_rect.top(), 0, bar_rect.bottom());
    vgradient.setColorAt(0.0, QColor(0x00, 0x00, 0x00, 0x00));
    vgradient.setColorAt(1.0, QColor(0x00, 0x00, 0x00, 0x30));
    painter.setBrush(vgradient);
    painter.drawRoundedRect(bar_rect, bar_size_px / 10, bar_size_px / 10);
}

} /* namespace os2cx */
