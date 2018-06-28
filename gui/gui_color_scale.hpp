#ifndef GUI_COLOR_SCALE_HPP
#define GUI_COLOR_SCALE_HPP

#include <QWidget>

class GuiColorScale : public QWidget
{
    Q_OBJECT
public:
    /* Suppose the data values range from val_min=1.2 to val_max=3.4. Then:
     - With Anchor::Floating, the color scale will range from 1.2 to 3.4.
     - With Anchor::Zero, it will range from 0.0 to 3.4.
     - With Anchor::Balanced, it will range from -3.4 to 3.4. */
    enum class Anchor { Floating, Zero, Balanced };
    GuiColorScale(
        QWidget *parent,
        Anchor anchor,
        double val_min,
        double val_max);

    QColor color(double val) const;

signals:

public slots:

private:
    static const int bar_size_px = 20;

    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);

    Anchor anchor;
    double range_min, range_max;
    std::map<double, QColor> colors;
};

#endif // GUI_COLOR_SCALE_HPP
