#include "opengl.hpp"

#include "poly.internal.hpp"

namespace os2cx {

void OpenglTriangles::reserve(int n) {
    vertices.reserve(n * 9);
    normals.reserve(n * 9);
}

void OpenglTriangles::add_triangle(
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

void OpenglTriangles::draw() const {
    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glNormalPointer(GL_FLOAT, 0, normals.data());
    glDrawArrays(GL_TRIANGLES, 0, 3 * num_triangles);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_VERTEX_ARRAY);
}

OpenglTriangles poly3_map_surface_to_opengl_triangles(
    const Poly3Map &poly3_map,
    Poly3Map::SurfaceId sid,
    int outside_volume_index
) {
    const Poly3Map::Surface &surf = poly3_map.surfaces[sid];
    OpenglTriangles triangles;
    triangles.reserve(surf.triangles.size());
    for (const Poly3Map::Surface::Triangle &tri : surf.triangles) {
        Point ps[3];
        for (int i = 0; i < 3; ++i) {
            ps[i] = poly3_map.vertices[tri.vertices[i]].point;
        }
        if (outside_volume_index == 0) {
            triangles.add_triangle(ps[0], ps[1], ps[2], tri.normal);
        } else {
            triangles.add_triangle(ps[0], ps[2], ps[1], tri.normal);
        }
    }
    return triangles;
}

OpenglTriangles poly3_to_opengl_triangles(const Poly3 *poly3) {
    const os2cx::CgalPolyhedron3 &p = poly3->i->p;
    assert(p.is_pure_triangle());
    OpenglTriangles triangles;
    triangles.reserve(p.size_of_facets());
    for (auto it = p.facets_begin(); it != p.facets_end(); ++it) {
        os2cx::CgalPolyhedron3::Halfedge_around_facet_const_circulator hc =
            it->facet_begin();
        CGAL::Point_3<K> ps1[3];
        Point ps2[3];
        for (int i = 0; i < 3; ++i, ++hc) {
            ps1[i] = hc->vertex()->point();
            ps2[i] = Point::raw(ps1[i].x(), ps1[i].y(), ps1[i].z());
        }
        CGAL::Plane_3<K> plane(ps1[0], ps1[1], ps1[2]);
        CGAL::Vector_3<K> n1 = plane.orthogonal_vector();
        double m = sqrt(n1.x() * n1.x() + n1.y() * n1.y() + n1.z() * n1.z());
        PureVector n2 = PureVector::raw(n1.x() / m, n1.y() / m, n1.z() / m);
        triangles.add_triangle(ps2[0], ps2[1], ps2[2], n2);
    }
    return triangles;
}

OpenglTriangles mesh_surface_to_opengl_triangles(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index,
    const ContiguousMap<NodeId, PureVector> *disps
) {
    OpenglTriangles triangles;
    for (const FaceId &fi : mesh_index.unmatched_faces) {
        const Element3 &element = mesh.elements[fi.element_id];
        const ElementTypeInfo &type = ElementTypeInfo::get(element.type);
        Point ps[3];
        for (int i = 0; i < 3; ++i) {
            int vertex = type.shape->faces[fi.face].vertices[i];
            NodeId node_id = element.nodes[vertex];
            ps[i] = mesh.nodes[node_id].point;
            if (disps != nullptr) {
                PureVector disp = (*disps)[node_id];
                if (disp == disp) {   // filter out NaN
                    ps[i] += disp * Length(1.0);
                }
            }
        }
        PureVector normal = triangle_normal(ps[0], ps[1], ps[2]);
        triangles.add_triangle(ps[0], ps[1], ps[2], normal);
    }
    return triangles;
}

void OpenglLines::add_line(const Point &p1, const Point &p2) {
    ++num_lines;
    vertices.push_back(p1.vector.x.val);
    vertices.push_back(p1.vector.y.val);
    vertices.push_back(p1.vector.z.val);
    vertices.push_back(p2.vector.x.val);
    vertices.push_back(p2.vector.y.val);
    vertices.push_back(p2.vector.z.val);
}

void OpenglLines::draw() const {
    glEnable(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glDrawArrays(GL_LINES, 0, 2 * num_lines);
    glDisable(GL_VERTEX_ARRAY);
}

OpenglLines mesh_surface_to_opengl_lines(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index,
    const ContiguousMap<NodeId, PureVector> *disps
) {
    OpenglLines lines;
    for (const FaceId &fi : mesh_index.unmatched_faces) {
        const Element3 &element = mesh.elements[fi.element_id];
        const ElementTypeInfo &type = ElementTypeInfo::get(element.type);
        NodeId node_ids[3];
        for (int i = 0; i < 3; ++i) {
            node_ids[i] = element.nodes[type.shape->faces[fi.face].vertices[i]];
        }
        for (int i = 0; i < 3; ++i) {
            int j = (i + 1) % 3;
            if (node_ids[i].to_int() > node_ids[j].to_int()) {
                /* Every line is part of two triangles, but the two triangles
                traverse it in opposite directions. Discard one of the two
                directions so that we don't draw each line twice. */
                continue;
            }
            Point pi = mesh.nodes[node_ids[i]].point;
            Point pj = mesh.nodes[node_ids[j]].point;
            if (disps != nullptr) {
                PureVector dispi = (*disps)[node_ids[i]];
                if (dispi == dispi) {   // filter out NaN
                    pi += dispi * Length(1.0);
                }
                PureVector dispj = (*disps)[node_ids[j]];
                if (dispj == dispj) {   // filter out NaN
                    pj += dispj * Length(1.0);
                }
            }
            lines.add_line(pi, pj);
        }
    }
    return lines;
}


Length approx_scale_of_project(const Project &project) {
    Length scale(0);
    for (const auto &pair : project.mesh_objects) {
        if (!pair.second.poly3_map_index) {
            return Length(0);
        }
        Length subscale = pair.second.poly3_map_index->approx_scale();
        if (subscale > scale) scale = subscale;
    }
    return scale;
}

void initialize_opengl() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Grey Background
    glClearDepth(1.0f); // Depth Buffer Setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_ambient[4] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    GLfloat light_diffuse[4] = {1, 1, 1, 1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    GLfloat light_specular[4] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    GLfloat light_model_ambient[4] = {0.3, 0.3, 0.3, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat material_specular[4] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);

    /* TODO: Back-face culling */
}

static const float fov_slope_min = 0.5;

void resize_opengl(int viewport_width, int viewport_height) {
    glViewport(0, 0, viewport_width, viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    float ratio_w_h = viewport_width / static_cast<float>(viewport_height);
    float fov_slope_y = (viewport_width > viewport_height)
        ? fov_slope_min
        : (fov_slope_min / ratio_w_h);
    float fov_angle_y = 2 * atan(fov_slope_y) / M_PI * 180;
    gluPerspective(
        fov_angle_y,
        ratio_w_h,
        0.1 /*clip close*/,
        200 /*clip far*/
    );
}

void OpenglDrawSettings::setup_for_draw(Length approx_scale) const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light_dir[4] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);

    /* Position camera such that entire project is visible */
    float fov_slop_factor = 1.5;
    glTranslatef(0, 0, -approx_scale.val / fov_slope_min * fov_slop_factor);

    glRotatef(90 + pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 0.0f, 1.0f);
}

void OpenglDrawSettings::drag(float delta_x, float delta_y) {
    /* This scaling factor is chosen such that dragging from the left side of
    the view to the right side will rotate the model 180 degrees */
    yaw += delta_x * 180;
    pitch += delta_y * 180;

    yaw -= 360 * floor(yaw / 360);
    if (pitch > 90) pitch = 90;
    if (pitch < -90) pitch = -90;
}

class OpenglTriangleScene : public OpenglScene {
public:
    void draw(const OpenglDrawSettings &settings) {
        settings.setup_for_draw(approx_scale);

        glColor3f(0.5, 0.5, 0.5);
        for (const OpenglTriangles &triangles : normal) {
            triangles.draw();
        }

        glColor3f(1.0, 0.0, 0.0);
        for (const OpenglTriangles &triangles : focused) {
            triangles.draw();
        }

        glColor3f(0.0, 0.0, 0.0);
        for (const OpenglLines &l : lines) {
            l.draw();
        }
    }

    Length approx_scale;
    std::vector<OpenglTriangles> normal, focused;
    std::vector<OpenglLines> lines;
};

void poly3_map_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings,
    OpenglTriangleScene *scene
) {
    for (const auto &pair : project.mesh_objects) {
        const Poly3Map *poly3_map = pair.second.poly3_map.get();
        if (poly3_map == nullptr) {
            continue;
        }
        for (Poly3Map::SurfaceId sid = 0;
                sid < static_cast<int>(poly3_map->surfaces.size()); ++sid) {
            const Poly3Map::Surface &surface = poly3_map->surfaces[sid];

            int outside_volume_index;
            if (surface.volumes[0] == poly3_map->volume_outside) {
                outside_volume_index = 0;
            } else if (surface.volumes[1] == poly3_map->volume_outside) {
                outside_volume_index = 1;
            } else {
                continue;
            }
            OpenglTriangles triangles = poly3_map_surface_to_opengl_triangles(
                *poly3_map, sid, outside_volume_index);

            Poly3Map::VolumeId inside_volume_id =
                surface.volumes[1 - outside_volume_index];

            bool focused;
            switch (scene_settings.focus.type) {
            case OpenglFocus::Type::All:
                focused = false;
                break;
            case OpenglFocus::Type::Result:
                focused = false;
                break;
            case OpenglFocus::Type::Mesh:
                focused = (pair.first == scene_settings.focus.target);
                break;
            case OpenglFocus::Type::SelectVolume: {
                focused = project
                    .volume_objects
                    .at(scene_settings.focus.target)
                    .poly3_map_volumes.at(pair.first)
                    .count(inside_volume_id);
                break;
            }
            case OpenglFocus::Type::Load: {
                std::string load_volume =
                    project.load_objects.at(scene_settings.focus.target).volume;
                focused = project
                    .volume_objects
                    .at(load_volume)
                    .poly3_map_volumes.at(pair.first)
                    .count(inside_volume_id);
                break;
            }
            }

            if (focused) {
                scene->focused.push_back(std::move(triangles));
            } else {
                scene->normal.push_back(std::move(triangles));
            }
        }
    }
}

void mesh_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings,
    OpenglTriangleScene *scene
) {
    const Mesh3 *mesh = project.mesh.get();
    const Mesh3Index *mesh_index = project.mesh_index.get();
    if (mesh == nullptr || mesh_index == nullptr) {
        return;
    }
    switch (scene_settings.focus.type) {
    case OpenglFocus::Type::Result: {
        const ContiguousMap<NodeId, PureVector> *result =
            &project.results->node_vectors.at(scene_settings.focus.target);
        scene->normal.push_back(
            mesh_surface_to_opengl_triangles(*mesh, *mesh_index, result));
        if (scene_settings.show_elements) {
            scene->lines.push_back(
                mesh_surface_to_opengl_lines(*mesh, *mesh_index, result));
        }
        break;
    }
    // TODO: Allow focus on non-results too
    default:
        scene->normal.push_back(
            mesh_surface_to_opengl_triangles(*mesh, *mesh_index, nullptr));
        if (scene_settings.show_elements) {
            scene->lines.push_back(
                mesh_surface_to_opengl_lines(*mesh, *mesh_index, nullptr));
        }
        break;
    }
}

std::unique_ptr<OpenglScene> project_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings
) {
    std::unique_ptr<OpenglTriangleScene> scene(new OpenglTriangleScene);
    scene->approx_scale = approx_scale_of_project(project);

    if (scene_settings.show_elements ||
            scene_settings.focus.type == OpenglFocus::Type::Result) {
        mesh_to_opengl_scene(project, scene_settings, scene.get());
    } else {
        poly3_map_to_opengl_scene(project, scene_settings, scene.get());
    }

    return scene;
}

} /* namespace os2cx */
