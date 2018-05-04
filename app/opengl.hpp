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
    void reserve(int n) {
        vertices.reserve(n * 9);
        normals.reserve(n * 9);
    }
    void add_triangle(
        const Point &p1, const Point &p2, const Point &p3,
        const PureVector &normal
    ) {
        ++num_triangles;
        vertices.push_back(p1.vector.x.val);
        vertices.push_back(p1.vector.y.val);
        vertices.push_back(p1.vector.z.val);
        vertices.push_back(p2.vector.x.val);
        vertices.push_back(p2.vector.y.val);
        vertices.push_back(p2.vector.z.val);
        vertices.push_back(p3.vector.x.val);
        vertices.push_back(p3.vector.y.val);
        vertices.push_back(p3.vector.z.val);
        for (int i = 0; i < 3; ++i) {
            normals.push_back(normal.x.val);
            normals.push_back(normal.y.val);
            normals.push_back(normal.z.val);
        }
    }
    void draw() const {
        glEnable(GL_VERTEX_ARRAY);
        glEnable(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        glNormalPointer(GL_FLOAT, 0, normals.data());
        glDrawArrays(GL_TRIANGLES, 0, 3 * num_triangles);
        glDisable(GL_NORMAL_ARRAY);
        glDisable(GL_VERTEX_ARRAY);
    }

private:
    int num_triangles;
    std::vector<GLfloat> vertices, normals;
};

OpenglTriangles region_map_surface_to_opengl_triangles(
    const RegionMap3 &region_map,
    RegionMap3::SurfaceId surface,
    /* outside_volume_index should be 0 or 1, indicating which of the volumes
    adjacent to the surface is considered the "outside" for rendering
    purposes */
    int outside_volume_index);

OpenglTriangles region_to_opengl_triangles(
    const Region3 *region);

OpenglTriangles mesh_surface_to_opengl_triangles(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index);

class OpenglLines {
public:
    OpenglLines() : num_lines(0) { }
    void add_line(const Point &p1, const Point &p2) {
        ++num_lines;
        vertices.push_back(p1.vector.x.val);
        vertices.push_back(p1.vector.y.val);
        vertices.push_back(p1.vector.z.val);
        vertices.push_back(p2.vector.x.val);
        vertices.push_back(p2.vector.y.val);
        vertices.push_back(p2.vector.z.val);
    }
    void draw() const {
        glEnable(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        glDrawArrays(GL_LINES, 0, 2 * num_lines);
        glDisable(GL_VERTEX_ARRAY);
    }

private:
    int num_lines;
    std::vector<GLfloat> vertices;
};

OpenglLines mesh_surface_to_opengl_lines(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index);

/* Returns the approximate physical size scale of the project, expressed as a
radius from the origin. The project must have `region_map_index` already
calculated, or else the return value will be zero. */
Length approx_scale_of_project(const Project &project);

void initialize_opengl();
void resize_opengl(int viewport_width, int viewport_height);

class OpenglDrawSettings {
public:
    OpenglDrawSettings() : yaw(40), pitch(20) { }

    void setup_for_draw(Length approx_scale) const;

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
