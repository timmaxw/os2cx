#include "opengl.hpp"

#include "region.internal.hpp"

namespace os2cx {

OpenglTriangles region_map_surface_to_opengl_triangles(
    const RegionMap3 &region_map,
    RegionMap3::SurfaceId sid,
    int outside_volume_index
) {
    const RegionMap3::Surface &surf = region_map.surfaces[sid];
    OpenglTriangles triangles;
    triangles.reserve(surf.triangles.size());
    for (const RegionMap3::Surface::Triangle &tri : surf.triangles) {
        Point ps[3];
        for (int i = 0; i < 3; ++i) {
            ps[i] = region_map.vertices[tri.vertices[i]].point;
        }
        if (outside_volume_index == 0) {
            triangles.add_triangle(ps[0], ps[1], ps[2], tri.normal);
        } else {
            triangles.add_triangle(ps[0], ps[2], ps[1], tri.normal);
        }
    }
    return triangles;
}

OpenglTriangles region_to_opengl_triangles(const Region3 *region) {
    const os2cx::CgalPolyhedron3 &p = region->i->p;
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
            NodeId node_id = element.nodes[type.shape->faces[fi.face][i]];
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

OpenglLines mesh_surface_to_opengl_lines(
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index,
    const ContiguousMap<NodeId, PureVector> *disps
) {
    (void)disps;
    OpenglLines lines;
    for (const FaceId &fi : mesh_index.unmatched_faces) {
        const Element3 &element = mesh.elements[fi.element_id];
        const ElementTypeInfo &type = ElementTypeInfo::get(element.type);
        NodeId node_ids[3];
        for (int i = 0; i < 3; ++i) {
            node_ids[i] = element.nodes[type.shape->faces[fi.face][i]];
        }
        for (int i = 0; i < 3; ++i) {
            int j = (i + 1) % 3;
            if (node_ids[i].to_int() > node_ids[j].to_int()) {
                /* Every line is part of two triangles, but the two triangles
                traverse it in opposite directions. Discard one of the two
                directions so that we don't draw each line twice. */
                continue;
            }
            lines.add_line(
                mesh.nodes[node_ids[i]].point,
                mesh.nodes[node_ids[j]].point);
        }
    }
    return lines;
}

Length approx_scale_of_project(const Project &project) {
    Length scale(0);
    for (const auto &pair : project.element_directives) {
        if (!pair.second.region_map_index) {
            return Length(0);
        }
        Length subscale = pair.second.region_map_index->approx_scale();
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

void region_map_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings,
    OpenglTriangleScene *scene
) {
    for (const auto &pair : project.element_directives) {
        const RegionMap3 *region_map = pair.second.region_map.get();
        if (region_map == nullptr) {
            continue;
        }
        for (RegionMap3::SurfaceId sid = 0;
                sid < static_cast<int>(region_map->surfaces.size()); ++sid) {
            const RegionMap3::Surface &surface = region_map->surfaces[sid];

            int outside_volume_index;
            if (surface.volumes[0] == region_map->volume_outside) {
                outside_volume_index = 0;
            } else if (surface.volumes[1] == region_map->volume_outside) {
                outside_volume_index = 1;
            } else {
                continue;
            }
            OpenglTriangles triangles = region_map_surface_to_opengl_triangles(
                *region_map, sid, outside_volume_index);

            const RegionMap3::Volume &inside_volume =
                region_map->volumes[surface.volumes[1 - outside_volume_index]];

            bool focused;
            switch (scene_settings.focus.type) {
            case OpenglFocus::Type::All:
                focused = false;
                break;
            case OpenglFocus::Type::Result:
                focused = false;
                break;
            case OpenglFocus::Type::Element:
                focused = (pair.first == scene_settings.focus.target);
                break;
            case OpenglFocus::Type::NSet: {
                const Region3 *mask = project.nset_directives.
                    find(scene_settings.focus.target)->second.mask.get();
                focused = inside_volume.masks.find(mask)->second == true;
                break;
            }
            case OpenglFocus::Type::VolumeLoad: {
                const Region3 *mask = project.volume_load_directives.
                    find(scene_settings.focus.target)->second.mask.get();
                focused = inside_volume.masks.find(mask)->second == true;
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
    (void)scene_settings; /* TODO */
    for (const auto &pair : project.element_directives) {
        const Mesh3 *mesh = pair.second.mesh.get();
        const Mesh3Index *mesh_index = pair.second.mesh_index.get();
        if (mesh == nullptr || mesh_index == nullptr) {
            continue;
        }
        switch (scene_settings.focus.type) {
        case OpenglFocus::Type::Result: {
            const ContiguousMap<NodeId, PureVector> *result =
                &project.results->node_vectors.at(scene_settings.focus.target);
            scene->normal.push_back(
                mesh_surface_to_opengl_triangles(*mesh, *mesh_index, result));
            scene->lines.push_back(
                mesh_surface_to_opengl_lines(*mesh, *mesh_index, result));
            break;
        }
        default:
            scene->normal.push_back(
                mesh_surface_to_opengl_triangles(*mesh, *mesh_index, nullptr));
            scene->lines.push_back(
                mesh_surface_to_opengl_lines(*mesh, *mesh_index, nullptr));
            break;
        }
    }
}

std::unique_ptr<OpenglScene> project_to_opengl_scene(
    const Project &project,
    const OpenglSceneSettings &scene_settings
) {
    std::unique_ptr<OpenglTriangleScene> scene(new OpenglTriangleScene);
    scene->approx_scale = approx_scale_of_project(project);

    if (scene_settings.show_elements) {
        mesh_to_opengl_scene(project, scene_settings, scene.get());
    } else {
        region_map_to_opengl_scene(project, scene_settings, scene.get());
    }

    return scene;
}

} /* namespace os2cx */

