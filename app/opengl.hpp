#ifndef OS2CX_OPENGL_HPP_
#define OS2CX_OPENGL_HPP_

#include "project.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>

namespace os2cx {

/* The GUI code calls project_to_opengl_scene() to get an OpenglScene
describing a project; then calls setup_and_draw() on the scene to render it.
OpenglSceneSettings contains all settings for which the OpenglScene needs to
be recreated if they change. OpenglDrawSettings contains the settings that can
be applied to an existing OpenglScene. */

class OpenglTriangles {
public:
    OpenglTriangles() : num_triangles(0) { }
    void reserve(int n);
    void add_triangle(
        const Point &p1, const Point &p2, const Point &p3,
        const PureVector &normal
    );
    void draw() const;

private:
    int num_triangles;
    std::vector<GLfloat> vertices, normals;
};

OpenglTriangles poly3_map_surface_to_opengl_triangles(
    const Poly3Map &poly3_map,
    Poly3Map::SurfaceId surface,
    /* outside_volume_index should be 0 or 1, indicating which of the volumes
    adjacent to the surface is considered the "outside" for rendering
    purposes */
    int outside_volume_index);

OpenglTriangles poly3_to_opengl_triangles(const Poly3 *poly3);

OpenglTriangles mesh_surface_to_opengl_triangles(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index);

class OpenglLines {
public:
    OpenglLines() : num_lines(0) { }
    void add_line(const Point &p1, const Point &p2);
    void draw() const;

private:
    int num_lines;
    std::vector<GLfloat> vertices;
};

OpenglLines mesh_surface_to_opengl_lines(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index);

/* Returns the approximate physical size scale of the project, expressed as a
radius from the origin. The project must have `poly3_map_index` already
calculated, or else the return value will be zero. */
Length approx_scale_of_project(const Project &project);

void initialize_opengl();
void resize_opengl(int viewport_width, int viewport_height);

class OpenglDrawSettings {
public:
    OpenglDrawSettings() : yaw(40), pitch(20) { }

    void setup_for_draw(Length approx_scale) const;

    /* Updates the draw settings in response to a mouse drag. delta_x is the
    relative mouse movement as a fraction of the view width. delta_y is the
    relative mouse movement at the same scale as delta_x (regardless of the view
    height). */
    void drag(float delta_x, float delta_y);

    float yaw, pitch; /* in degrees */
};

class OpenglScene {
public:
    virtual ~OpenglScene() { }
    virtual void draw(const OpenglDrawSettings &draw_settings) = 0;
};

class OpenglFocus {
public:
    enum class Type {
        All, // no target
        Element, // target = element name
        NSet, // target = nset name
        VolumeLoad, // target = volume load name
        Result, // target = dataset name
    };

    OpenglFocus() : type(Type::All) { }
    bool operator==(const OpenglFocus &other) const {
        return type == other.type && target == other.target;
    }
    bool operator!=(const OpenglFocus &other) const {
        return !(*this == other);
    }

    Type type;
    std::string target;
};

class OpenglSceneSettings {
public:
    OpenglSceneSettings() : show_elements(false) { }
    bool show_elements;
    OpenglFocus focus;
};

std::unique_ptr<OpenglScene> project_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings);

} /* namespace os2cx */

#endif
