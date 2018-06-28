#include "gui_scene_abstract.hpp"

#include <QLabel>

namespace os2cx {

GuiSceneAbstract::GuiSceneAbstract(QWidget *parent, const Project *project_) :
    QWidget(parent), project(project_)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    /* Disable the inner margins because we'll be embedded in the 'left_panel'
    widget that has its own margins */
    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

void GuiSceneAbstract::create_widget_label(const QString &text) {
    layout->addSpacing(10);
    QLabel *label = new QLabel(text, this);
    layout->addWidget(label);
}

void GuiSceneAbstract::add_triangle(
    const Point *points, const QColor *colors
) {
    Vector normal = triangle_normal(points[0], points[1], points[2]);
    ++num_triangles;
    for (int i = 0; i < 3; ++i) {
        triangle_vertices.push_back(points[i].x);
        triangle_vertices.push_back(points[i].y);
        triangle_vertices.push_back(points[i].z);
        triangle_colors.push_back(colors[i].red());
        triangle_colors.push_back(colors[i].green());
        triangle_colors.push_back(colors[i].blue());
        triangle_normals.push_back(normal.x);
        triangle_normals.push_back(normal.y);
        triangle_normals.push_back(normal.z);
    }
}

void GuiSceneAbstract::add_line(const Point *points) {
    ++num_lines;
    for (int i = 0; i < 2; ++i) {
        line_vertices.push_back(points[i].x);
        line_vertices.push_back(points[i].y);
        line_vertices.push_back(points[i].z);
    }
}

void GuiSceneAbstract::clear() {
    num_triangles = 0;
    triangle_vertices.clear();
    triangle_colors.clear();
    triangle_normals.clear();
    num_lines = 0;
    line_vertices.clear();
}

} /* namespace os2cx */
