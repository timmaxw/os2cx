#include "gui_color_scale.hpp"

#include <assert.h>

#include <QPainter>

GuiColorScale::GuiColorScale(QWidget *parent) :
    QWidget(parent)
{
    set_range(Anchor::Zero, 0.0, 1.0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void GuiColorScale::set_range(Anchor a, double val_min, double val_max) {
    anchor = a;
    colors.clear();
    assert(val_min <= val_max);
    if (anchor == Anchor::Floating) {
        range_min = val_min;
        range_max = val_max;
        colors[range_min]                   = QColor::fromHsl(210, 0xFF, 0x99);
        colors[(range_min + range_max) / 2] = QColor::fromHsl(285, 0x33, 0x66);
        colors[range_max]                   = QColor::fromHsl(  0, 0xFF, 0x99);
    } else if (anchor == Anchor::Zero) {
        assert(0 <= val_min);
        range_min = 0;
        range_max = val_max;
        colors[0]             = QColor::fromHsl( 60, 0x00, 0xEE);
        colors[range_max / 3] = QColor::fromHsl( 60, 0xEE, 0x99);
        colors[range_max]     = QColor::fromHsl(  0, 0xFF, 0x66);
    } else if (anchor == Anchor::Balanced) {
        range_min = -std::max(std::abs(val_min), std::abs(val_max));
        range_max = -range_min;
        colors[range_min]     = QColor::fromHsl(240, 0xFF, 0x66);
        colors[range_min / 3] = QColor::fromHsl(180, 0xEE, 0x99);
        colors[0]             = QColor::fromHsl(120, 0x00, 0xEE);
        colors[range_max / 3] = QColor::fromHsl( 60, 0xEE, 0x99);
        colors[range_max]     = QColor::fromHsl(  0, 0xFF, 0x66);
    } else {
        assert(false);
    }
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

    QRect label_rect(0, 0, width(), fontMetrics().height());
    painter.drawText(label_rect, Qt::AlignLeft|Qt::AlignBottom,
        QString("%1").arg(range_min));
    painter.drawText(label_rect, Qt::AlignRight|Qt::AlignBottom,
        QString("%1").arg(range_max));

    /* Draw the main color gradient */
    QRect bar_rect(0, label_rect.bottom(), width(), bar_size_px);
    painter.setPen(Qt::NoPen);
    QLinearGradient gradient(bar_rect.left(), 0, bar_rect.right(), 0);
    for (const auto &pair : colors) {
        double normalized = (pair.first - range_min) / (range_max - range_min);
        gradient.setColorAt(normalized, pair.second);
    }
    painter.setBrush(gradient);
    painter.drawRoundedRect(bar_rect, bar_size_px / 10, bar_size_px / 10);

    /* Draw shading on top of the main color gradient */
    QLinearGradient vgradient(0, bar_rect.top(), 0, bar_rect.bottom());
    vgradient.setColorAt(0.0, QColor(0x00, 0x00, 0x00, 0x00));
    vgradient.setColorAt(1.0, QColor(0x00, 0x00, 0x00, 0x30));
    painter.setBrush(vgradient);
    painter.drawRoundedRect(bar_rect, bar_size_px / 10, bar_size_px / 10);
}
