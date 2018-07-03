#ifndef GUI_COLOR_SCALE_HPP
#define GUI_COLOR_SCALE_HPP

#include <QWidget>

class GuiColorScale : public QWidget
{
    Q_OBJECT
public:
    explicit GuiColorScale(QWidget *parent = nullptr);

    void set_range(double val_min, double val_max);

    QColor color(double val) const;

signals:

public slots:

private:
    static const int bar_size_px = 20;

    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);

    double range_min, range_max;
    std::map<double, QColor> colors;
};

#endif // GUI_COLOR_SCALE_HPP
