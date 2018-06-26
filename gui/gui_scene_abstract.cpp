#include "gui_scene_abstract.hpp"

namespace os2cx {

GuiSceneAbstract::GuiSceneAbstract(QWidget *parent, const Project *project_) :
    QWidget(parent), project(project_)
{ }

void GuiSceneAbstract::add_triangle(
    const Point *points, const QColor *colors
) {
    PureVector normal = triangle_normal(points[0], points[1], points[2]);
    ++num_triangles;
    for (int i = 0; i < 3; ++i) {
        triangle_vertices.push_back(points[i].vector.x.val);
        triangle_vertices.push_back(points[i].vector.y.val);
        triangle_vertices.push_back(points[i].vector.z.val);
        triangle_colors.push_back(colors[i].red());
        triangle_colors.push_back(colors[i].green());
        triangle_colors.push_back(colors[i].blue());
        triangle_normals.push_back(normal.x.val);
        triangle_normals.push_back(normal.y.val);
        triangle_normals.push_back(normal.z.val);
    }
}

void GuiSceneAbstract::add_line(const Point *points) {
    ++num_lines;
    for (int i = 0; i < 2; ++i) {
        line_vertices.push_back(points[i].vector.x.val);
        line_vertices.push_back(points[i].vector.y.val);
        line_vertices.push_back(points[i].vector.z.val);
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
