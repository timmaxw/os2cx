#include "mesher_naive_bricks.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>

#include "mesher_tetgen.hpp"

/* uncomment to enable debug output */
// #define NAIVE_BRICKS_DEBUG(x) x
#define NAIVE_BRICKS_DEBUG(x) (void)0

namespace os2cx {

/* A pointer to a specific triangle on a specific surface in the Plc3 */
class TriangleRef {
public:
    TriangleRef() { }
    TriangleRef(Plc3::SurfaceId sid, int tix) :
        surface_id(sid), triangle_ix(tix) { }
    Plc3::SurfaceId surface_id;
    int triangle_ix;
};

static const Plc3::VolumeId VOLUME_ID_UNSET = -1;
static const Plc3::SurfaceId SURFACE_ID_UNSET = -1;

/* Tracks a mapping between X/Y/Z coordinates and grid indexes. */
class GridAxis {
public:
    typedef std::map<double, int>::const_iterator const_iterator;
    GridAxis(const std::map<double, std::vector<TriangleRef> > &triangles) {
        for (const auto &pair : triangles) {
            points[pair.first] = points.size();
            points_by_index.push_back(pair.first);
        }
    }
    int num_points() const {
        return points.size();
    }
    int num_intervals() const {
        return points.size() - 1;
    }
    const_iterator begin_points() const {
        return points.begin();
    }
    const_iterator end_points() const {
        return points.end();
    }
    const_iterator find_point_lte(double p) const {
        auto it = points.lower_bound(p);
        if (it != points.end() && it->first == p) return it;
        assert(it != points.begin());
        --it;
        return it;
    }
    const_iterator find_point_gte(double p) const {
        auto it = points.lower_bound(p);
        assert(it != points.end());
        return it;
    }
    double point_at_index(int index) const {
        return points_by_index.at(index);
    }
    std::map<double, int> points;
    std::vector<double> points_by_index;
};

/* Given a grid (expressed as a map of X/Y/Z coordinates to lists of triangles)
applies max_element_size and min_subdivision to that grid. */
void subdivide_grid(
    double max_element_size,
    int min_subdivision,
    std::map<double, std::vector<TriangleRef> > *grid
) {
    auto lower = grid->begin();
    while (true) {
        auto upper = lower;
        ++upper;
        if (upper == grid->end()) break;

        int num_parts = std::max(
            static_cast<int>(ceil(
                (upper->first - lower->first) / max_element_size)),
            min_subdivision);
        double part_size = (upper->first - lower->first) / num_parts;
        for (int i = 1; i < num_parts; ++i) {
            double split_point = lower->first + part_size * i;
            /* This mutates 'grid', but our existing 'upper' and 'lower'
            iterators remain valid. Because we've already calculated
            'upper', iteration will continue after the newly added
            points. */
            (*grid)[split_point];
        }

        lower = upper;
    }
}

/* Confirms that all triangles and borders in the Plc3 are aligned with the
X/Y/Z axes. Emits x/y/z_triangles_out, where keys are grid locations and values
are the triangles in the given plane. */
void setup_grid(
    const Plc3 &plc,
    double max_element_size,
    double min_subdivision,
    std::map<double, std::vector<TriangleRef> > *x_triangles_out,
    std::map<double, std::vector<TriangleRef> > *y_triangles_out,
    std::map<double, std::vector<TriangleRef> > *z_triangles_out
) {
    for (Plc3::SurfaceId sid = 0;
            sid < static_cast<int>(plc.surfaces.size()); ++sid) {
        const Plc3::Surface &surface = plc.surfaces[sid];
        for (int tix = 0;
                tix < static_cast<int>(surface.triangles.size()); ++tix) {
            const Plc3::Surface::Triangle &tri = surface.triangles[tix];
            Point p0 = plc.vertices[tri.vertices[0]].point;
            Point p1 = plc.vertices[tri.vertices[1]].point;
            Point p2 = plc.vertices[tri.vertices[2]].point;
            if (p0.x == p1.x && p0.x == p2.x) {
                (*x_triangles_out)[p0.x].push_back(TriangleRef(sid, tix));
            } else if (p0.y == p1.y && p0.y == p2.y) {
                (*y_triangles_out)[p0.y].push_back(TriangleRef(sid, tix));
            } else if (p0.z == p1.z && p0.z == p2.z) {
                (*z_triangles_out)[p0.z].push_back(TriangleRef(sid, tix));
            } else {
                throw NaiveBricksAlignmentError(p0, p1, p2);
            }
        }
    }

    for (const Plc3::Border &border : plc.borders) {
        for (int i = 0; i < static_cast<int>(border.vertices.size() - 1); ++i) {
            Point p0 = plc.vertices[border.vertices[i]].point;
            Point p1 = plc.vertices[border.vertices[i + 1]].point;
            if (p0.x == p1.x && p0.y == p1.y) {
                (*x_triangles_out)[p0.x]; /* make sure entry in map exists */
                (*y_triangles_out)[p0.y];
            } else if (p0.x == p1.x && p0.z == p1.z) {
                (*x_triangles_out)[p0.x];
                (*z_triangles_out)[p0.z];
            } else if (p0.y == p1.y && p0.z == p1.z) {
                (*y_triangles_out)[p0.y];
                (*z_triangles_out)[p0.z];
            } else {
                throw NaiveBricksAlignmentError(p0, p1);
            }
        }
    }

    /* This matters for vertices created by os2cx_select_node(), which may be
    isolated in the middle of a surface or volume */
    for (const Plc3::Vertex &vertex : plc.vertices) {
        Point p = vertex.point;
        (*x_triangles_out)[p.x]; /* make sure entry in map exists */
        (*y_triangles_out)[p.y];
        (*z_triangles_out)[p.z];
    }

    subdivide_grid(max_element_size, min_subdivision, x_triangles_out);
    subdivide_grid(max_element_size, min_subdivision, y_triangles_out);
    subdivide_grid(max_element_size, min_subdivision, z_triangles_out);
}

/* Helper class for tracking the minimum and maximum of a set of numbers */
class MinMaxSet {
public:
    MinMaxSet() :
        current_min(std::numeric_limits<double>::max()),
        current_max(std::numeric_limits<double>::lowest())
        { }
    void insert(double s) {
        current_min = std::min(current_min, s);
        current_max = std::max(current_max, s);
    }
    double min() const { return current_min; }
    double max() const { return current_max; }
    double current_min, current_max;
};

/* Consider the line segment from p0 to p1. Some subset of that line segment
will have U-coordinate between u_lower and u_upper. interpolate_v_from_u()
calculates the V-coordinates of that segment, and inserts into the given
MinMaxSet. U and V can be arbitrary Dimensions. */
inline void interpolate_v_from_u(
    Dimension dim_u,
    Dimension dim_v,
    Point p0,
    Point p1,
    double u_lower,
    double u_upper,
    MinMaxSet *v_out
) {
    if (p0.at(dim_u) > p1.at(dim_u)) {
        std::swap(p0, p1);
    }
    if (u_upper < p0.at(dim_u) || u_lower > p1.at(dim_u)) {
        return;
    }
    if (u_lower <= p0.at(dim_u)) {
        v_out->insert(p0.at(dim_v));
    } else {
        v_out->insert(p0.at(dim_v)
            + (u_lower - p0.at(dim_u))
                / (p1.at(dim_u) - p0.at(dim_u))
                * (p1.at(dim_v) - p0.at(dim_v)));
    }
    if (u_upper >= p1.at(dim_u)) {
        v_out->insert(p1.at(dim_v));
    } else {
        v_out->insert(p0.at(dim_v)
            + (u_upper - p0.at(dim_u))
                / (p1.at(dim_u) - p0.at(dim_u))
                * (p1.at(dim_v) - p0.at(dim_v)));
    }
}

/* Generic function for taking a list of triangles that all lie within the same
W-plane, and projecting them onto a U/V-grid, determining which grid cells each
triangle overlaps with and calling the callback for each overlap. Note that the
callback may be called multiple times for the same grid cell, with different
triangles. */
void apply_triangles(
    const Plc3 &plc,
    Dimension dim_u,
    Dimension dim_v,
    Dimension dim_w,
    const GridAxis &u_grid,
    const GridAxis &v_grid,
    const std::vector<TriangleRef> &triangles,
    const std::function<void(
        /* The index of the U-interval that the triangle overlaps with */
        int u_index,
        /* The index of the V-interval that the triangle overlaps with */
        int v_index,
        /* The surface ID of the surface holding the triangle */
        Plc3::SurfaceId surface,
        /* The volume ID on the negative-W side of the surface */
        Plc3::VolumeId volume_before,
        /* The volume ID on the positive-W side of the surface */
        Plc3::VolumeId volume_after
    )> &callback
) {
    for (const TriangleRef &tri_ref : triangles) {
        const Plc3::Surface &surface =
            plc.surfaces[tri_ref.surface_id];
        const Plc3::Surface::Triangle &triangle =
            surface.triangles[tri_ref.triangle_ix];
        Point p0 = plc.vertices[triangle.vertices[0]].point;
        Point p1 = plc.vertices[triangle.vertices[1]].point;
        Point p2 = plc.vertices[triangle.vertices[2]].point;

        Plc3::VolumeId volume_before, volume_after;
        if ((p1 - p0).cross(p2 - p0).at(dim_w) < 0) {
            volume_before = surface.volumes[0];
            volume_after = surface.volumes[1];
        } else {
            volume_before = surface.volumes[1];
            volume_after = surface.volumes[0];
        }

        NAIVE_BRICKS_DEBUG(std::cerr
            << "considering triangle "
            << "p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
            << ", vib = " << volume_before << ", via = " << volume_after
            << std::endl);

        MinMaxSet u_minmax;
        u_minmax.insert(p0.at(dim_u));
        u_minmax.insert(p1.at(dim_u));
        u_minmax.insert(p2.at(dim_u));

        /* Shrink the area by epsilon to prevent rounding errors wherein a
        triangle overlaps the wrong area by a tiny amount */
        double u_epsilon = (u_minmax.max() - u_minmax.min()) * 1e-10;
        GridAxis::const_iterator u_min =
            u_grid.find_point_lte(u_minmax.min() + u_epsilon);
        GridAxis::const_iterator u_max =
            u_grid.find_point_gte(u_minmax.max() - u_epsilon);

        for (auto u_lower = u_min; u_lower != u_max; ++u_lower) {
            auto u_upper = u_lower;
            ++u_upper;

            MinMaxSet v_minmax;
            interpolate_v_from_u(dim_u, dim_v,
                p0, p1, u_lower->first, u_upper->first, &v_minmax);
            interpolate_v_from_u(dim_u, dim_v,
                p1, p2, u_lower->first, u_upper->first, &v_minmax);
            interpolate_v_from_u(dim_u, dim_v,
                p2, p0, u_lower->first, u_upper->first, &v_minmax);

            NAIVE_BRICKS_DEBUG(std::cerr << "considering strip "
                << "u = (" << u_lower->first << ", " << u_upper->first << "), "
                << "v = (" << v_minmax.min() << ", " << v_minmax.max() << ")"
                << std::endl);

            double v_epsilon = (v_minmax.max() - v_minmax.min()) * 1e-10;
            GridAxis::const_iterator v_min =
                v_grid.find_point_lte(v_minmax.min() + v_epsilon);
            GridAxis::const_iterator v_max =
                v_grid.find_point_gte(v_minmax.max() - v_epsilon);

            for (auto v_lower = v_min; v_lower != v_max; ++v_lower) {
                auto v_upper = v_lower;
                ++v_upper;
                NAIVE_BRICKS_DEBUG(std::cerr << "applying at cell "
                    << "u = (" << u_lower->first
                    << ", " << u_upper->first << "), "
                    << "v = (" << v_lower->first
                    << ", " << v_upper->first << ")"
                    << std::endl);

                int u_index = u_lower->second;
                int v_index = v_lower->second;
                callback(
                    u_index,
                    v_index,
                    tri_ref.surface_id,
                    volume_before,
                    volume_after);
            }
        }
    }
}

/* Given a set of triangles that all lie in the same Z-plane, computes the
volume IDs for the grid cells immediately in the positive-Z direction of that
Z-plane. */
void apply_triangles_to_update_volume_ids(
    const Plc3 &plc,
    const GridAxis &x_grid,
    const GridAxis &y_grid,
    const std::vector<TriangleRef> &triangles,
    /* Volume IDs for the grid cells in the negative-Z direction of the Z-plane,
    used for sanity checking. */
    const Array2D<Plc3::VolumeId> &volume_ids_before,
    Array2D<Plc3::VolumeId> *volume_ids_after
) {
    apply_triangles(
        plc,
        Dimension::X, Dimension::Y, Dimension::Z,
        x_grid, y_grid,
        triangles,
        [&](int x_index, int y_index,
            Plc3::SurfaceId,
            Plc3::VolumeId volume_before,
            Plc3::VolumeId volume_after
        ) {
            assert(volume_ids_before(x_index, y_index) == volume_before);
            if ((*volume_ids_after)(x_index, y_index) == VOLUME_ID_UNSET) {
                (*volume_ids_after)(x_index, y_index) = volume_after;
            } else {
                assert(
                    (*volume_ids_after)(x_index, y_index) == volume_after);
            }
        }
    );
    for (int x = 0; x < x_grid.num_intervals(); ++x) {
        for (int y = 0; y < y_grid.num_intervals(); ++y) {
            if ((*volume_ids_after)(x, y) == VOLUME_ID_UNSET) {
                (*volume_ids_after)(x, y) = volume_ids_before(x, y);
            }
        }
    }
}

/* Helper function to create a single node */
NodeId create_node(
    const std::pair<double, int> &xp,
    const std::pair<double, int> &yp,
    const std::pair<double, Array2D<NodeId> *> &zp,
    Mesh3 *mesh
) {
    NodeId node_id = (*zp.second)(xp.second, yp.second);
    if (node_id != NodeId::invalid()) {
        return node_id;
    }
    node_id = mesh->nodes.key_end();
    Node3 node;
    node.point = Point(xp.first, yp.first, zp.first);
    mesh->nodes.push_back(node);
    (*zp.second)(xp.second, yp.second) = node_id;
    return node_id;
}

/* Helper function to create a single brick */
void create_brick(
    ElementType element_type,
    int order,
    const GridAxis::const_iterator &x_lower,
    const GridAxis::const_iterator &x_upper,
    const GridAxis::const_iterator &y_lower,
    const GridAxis::const_iterator &y_upper,
    double z_lower,
    double z_upper,
    Array2D<NodeId> *z_lower_node_ids,
    Array2D<NodeId> *z_middle_node_ids,
    Array2D<NodeId> *z_upper_node_ids,
    AttrBitset attrs,
    Mesh3 *mesh
) {
    Element3 element;
    element.type = element_type;

    std::pair<double, int> xp_lower(x_lower->first, x_lower->second * 2);
    std::pair<double, int> xp_upper(x_upper->first, x_upper->second * 2);
    std::pair<double, int> yp_lower(y_lower->first, y_lower->second * 2);
    std::pair<double, int> yp_upper(y_upper->first, y_upper->second * 2);
    std::pair<double, Array2D<NodeId> *> zp_lower(z_lower, z_lower_node_ids);
    std::pair<double, Array2D<NodeId> *> zp_upper(z_upper, z_upper_node_ids);

    element.nodes[0] = create_node(xp_lower, yp_lower, zp_lower, mesh);
    element.nodes[1] = create_node(xp_upper, yp_lower, zp_lower, mesh);
    element.nodes[2] = create_node(xp_upper, yp_upper, zp_lower, mesh);
    element.nodes[3] = create_node(xp_lower, yp_upper, zp_lower, mesh);
    element.nodes[4] = create_node(xp_lower, yp_lower, zp_upper, mesh);
    element.nodes[5] = create_node(xp_upper, yp_lower, zp_upper, mesh);
    element.nodes[6] = create_node(xp_upper, yp_upper, zp_upper, mesh);
    element.nodes[7] = create_node(xp_lower, yp_upper, zp_upper, mesh);

    if (order == 2) {
        std::pair<double, int> xp_middle(
            (x_lower->first + x_upper->first) / 2, x_lower->second * 2 + 1);
        std::pair<double, int> yp_middle(
            (y_lower->first + y_upper->first) / 2, y_lower->second * 2 + 1);
        std::pair<double, Array2D<NodeId> *> zp_middle(
            (z_lower + z_upper) / 2, z_middle_node_ids);

        element.nodes[ 8] = create_node(xp_middle, yp_lower,  zp_lower,  mesh);
        element.nodes[ 9] = create_node(xp_upper,  yp_middle, zp_lower,  mesh);
        element.nodes[10] = create_node(xp_middle, yp_upper,  zp_lower,  mesh);
        element.nodes[11] = create_node(xp_lower,  yp_middle, zp_lower,  mesh);
        element.nodes[12] = create_node(xp_middle, yp_lower,  zp_upper,  mesh);
        element.nodes[13] = create_node(xp_upper,  yp_middle, zp_upper,  mesh);
        element.nodes[14] = create_node(xp_middle, yp_upper,  zp_upper,  mesh);
        element.nodes[15] = create_node(xp_lower,  yp_middle, zp_upper,  mesh);
        element.nodes[16] = create_node(xp_lower,  yp_lower,  zp_middle, mesh);
        element.nodes[17] = create_node(xp_upper,  yp_lower,  zp_middle, mesh);
        element.nodes[18] = create_node(xp_upper,  yp_upper,  zp_middle, mesh);
        element.nodes[19] = create_node(xp_lower,  yp_upper,  zp_middle, mesh);
    }

    element.attrs = attrs;
    for (int face = 0; face < 6; ++face) {
        /* Initialize face attrs the same as volume attrs. This is sometimes
        inaccurate; we'll fix those cases in update_face_attrs. */
        element.face_attrs[face] = attrs;
    }

    mesh->elements.push_back(element);

    NAIVE_BRICKS_DEBUG(std::cerr << "created brick" << std::endl);
}

/* Creates all the bricks in a single Z-interval of the grid. */
void create_bricks(
    const Plc3 &plc,
    ElementType element_type,
    const GridAxis &x_grid,
    const GridAxis &y_grid,
    double z_lower,
    double z_upper,
    const Array2D<Plc3::VolumeId> &volume_ids,
    Array2D<NodeId> *z_lower_node_ids,
    Array2D<NodeId> *z_upper_node_ids,
    Mesh3 *mesh
) {
    assert(element_type_shape(element_type).category ==
        ElementTypeShape::Category::Brick);
    int order = element_type_shape(element_type).order;

    Array2D<NodeId> z_middle_node_ids;
    if (order == 2) {
        z_middle_node_ids = Array2D<NodeId>(
            x_grid.num_points() + x_grid.num_intervals(),
            y_grid.num_points() + y_grid.num_intervals(),
            NodeId::invalid());
    }

    auto x_lower = x_grid.begin_points();
    while (true) {
        auto x_upper = x_lower;
        ++x_upper;
        if (x_upper == x_grid.end_points()) break;

        auto y_lower = y_grid.begin_points();
        while (true) {
            auto y_upper = y_lower;
            ++y_upper;
            if (y_upper == y_grid.end_points()) break;

            Plc3::VolumeId vid = volume_ids(x_lower->second, y_lower->second);

            NAIVE_BRICKS_DEBUG(std::cerr
                << "x = (" << x_lower->first << ", " << x_upper->first << "), "
                << "y = (" << y_lower->first << ", " << y_upper->first << "), "
                << "z = (" << z_lower << ", " << z_upper << "), "
                << "vid = " << vid
                << std::endl);

            if (vid != plc.volume_outside) {
                create_brick(
                    element_type, order,
                    x_lower, x_upper, y_lower, y_upper, z_lower, z_upper,
                    z_lower_node_ids, &z_middle_node_ids, z_upper_node_ids,
                    plc.volumes[vid].attrs,
                    mesh);
            }

            y_lower = y_upper;
        }

        x_lower = x_upper;
    }
}

/* Finds the element ID of the brick whose 0-th node is at 'point'. This is
efficient by taking advantage of the fact that the bricks are always created in
lexicographical order on [z, x, y]. */
ElementId find_brick(const Mesh3 &mesh, Point point) {
    Element3 dummy_element;
    dummy_element.nodes[0] = NodeId::invalid();
    auto iterator_pair = std::equal_range(
        mesh.elements.begin(),
        mesh.elements.end(),
        dummy_element,
        [&](const Element3 &element_a, const Element3 &element_b) {
            Point point_a = (element_a.nodes[0] == NodeId::invalid())
                ? point
                : mesh.nodes[element_a.nodes[0]].point;
            Point point_b = (element_b.nodes[0] == NodeId::invalid())
                ? point
                : mesh.nodes[element_b.nodes[0]].point;
            if (point_a.z < point_b.z) return true;
            if (point_a.z > point_b.z) return false;
            if (point_a.x < point_b.x) return true;
            if (point_a.x > point_b.x) return false;
            return (point_a.y < point_b.y);
        }
    );

    if (iterator_pair.first == iterator_pair.second) {
        return ElementId::invalid();

    } else {
        int offset = iterator_pair.first - mesh.elements.begin();
        ElementId eid = ElementId::from_int(
            mesh.elements.key_begin().to_int() + offset);

        /* Sanity check that we actually found the right element */
        const Element3 &element = mesh.elements[eid];
        assert(mesh.nodes[element.nodes[0]].point == point);

        return eid;
    }
}

/* Sweeps through the entire grid in order of increasing W-dimension. For each
face of a brick in 'mesh' for which that face lies in the W-plane, and is part
of a surface, sets the attrs for that face to the surface's attrs. (Note that
for faces that are not part of surfaces, create_brick() should have already
initialized the attrs.) */
void update_face_attrs(
    const Plc3 &plc,
    Dimension dim_u, Dimension dim_v, Dimension dim_w,
    /* The face index of the face on the negative-W side of the brick */
    int face_before,
    /* The face index of the face on the positive-W side of the brick */
    int face_after,
    GridAxis u_grid, GridAxis v_grid,
    std::map<double, std::vector<TriangleRef> > w_triangles,
    Mesh3 *mesh
) {
    NAIVE_BRICKS_DEBUG(std::cerr
        << "update_face_attrs in dimension w=" << dim_w << std::endl);

    /* Store the element IDs for the previous W-interval of the grid.
    ElementId::invalid() indicates that cell was empty. */
    Array2D<ElementId> element_ids(
        u_grid.num_intervals(),
        v_grid.num_intervals(),
        ElementId::invalid());

    for (const auto &w_triangles_pair : w_triangles) {
        Point point;
        point.set_at(dim_w, w_triangles_pair.first);

        NAIVE_BRICKS_DEBUG(std::cerr
            << "update_face_attrs w=" << w_triangles_pair.first << std::endl);

        /* Compute surface ID for each grid cell in this W-plane */
        Array2D<Plc3::SurfaceId> surface_ids(
            u_grid.num_intervals(),
            v_grid.num_intervals(),
            SURFACE_ID_UNSET);
        apply_triangles(
            plc,
            dim_u, dim_v, dim_w,
            u_grid, v_grid, w_triangles_pair.second,
            [&](int u_index, int v_index,
                Plc3::SurfaceId surface_id, Plc3::VolumeId, Plc3::VolumeId
            ) {
                NAIVE_BRICKS_DEBUG(std::cerr
                    << "update_face_attrs applying triangles"
                    << " u_index=" << u_index
                    << " v_index=" << v_index
                    << " surface_id=" << surface_id << std::endl);

                /* Sanity-check surface IDs. */
                if (surface_ids(u_index, v_index) != SURFACE_ID_UNSET) {
                    assert(surface_ids(u_index, v_index) == surface_id);
                    return;
                } else {
                    surface_ids(u_index, v_index) = surface_id;
                }
            }
        );

        /* For each grid cell, update element_ids and apply attrs to the faces
        of the elements */
        for (int u_index = 0; u_index < u_grid.num_intervals(); ++u_index) {
            for (int v_index = 0; v_index < v_grid.num_intervals(); ++v_index) {
                /* Update element_ids */
                ElementId prev_eid = element_ids(u_index, v_index);
                point.set_at(dim_u, u_grid.point_at_index(u_index));
                point.set_at(dim_v, v_grid.point_at_index(v_index));
                ElementId next_eid = find_brick(*mesh, point);
                element_ids(u_index, v_index) = next_eid;

                Plc3::SurfaceId surface_id = surface_ids(u_index, v_index);
                NAIVE_BRICKS_DEBUG(std::cerr
                    << "update_face_attrs updating elements"
                    << " u_index=" << u_index
                    << " v_index=" << v_index
                    << " prev_eid=" << prev_eid.to_int()
                    << " point=" << point
                    << " next_eid=" << next_eid.to_int()
                    << " surface_id=" << surface_id << std::endl);
                if (surface_id != SURFACE_ID_UNSET) {
                    AttrBitset attrs = plc.surfaces[surface_id].attrs;
                    if (prev_eid != ElementId::invalid()) {
                        mesh->elements[prev_eid].face_attrs[face_after] =
                            attrs;
                    }
                    if (next_eid != ElementId::invalid()) {
                        mesh->elements[next_eid].face_attrs[face_before] =
                            attrs;
                    }
                }
            }
        }
    }
}

void update_node_attrs(const Plc3 &plc, Mesh3 *mesh) {
    struct LessPoint {
        bool operator()(Point p1, Point p2) const {
            return
                (p1.x < p2.x) ||
                (p1.x == p2.x && p1.y < p2.y) ||
                (p1.x == p2.x && p1.y == p2.y && p1.z < p2.z);
        }
    };
    std::map<Point, Plc3::VertexId, LessPoint> vertices_by_point;
    for (Plc3::VertexId vid = 0; vid < plc.vertices.size(); ++vid) {
        auto res = vertices_by_point.insert(
            std::make_pair(plc.vertices[vid].point, vid));
        assert(res.second);
    }
    for (Node3 &node : mesh->nodes) {
        auto it = vertices_by_point.find(node.point);
        if (it != vertices_by_point.end()) {
            node.attrs = plc.vertices[it->second].attrs;
        }
    }
}

Mesh3 mesher_naive_bricks(
    const Plc3 &plc,
    double max_element_size,
    int min_subdivision,
    ElementType element_type
) {
    std::map<double, std::vector<TriangleRef> >
        x_triangles, y_triangles, z_triangles;
    setup_grid(
        plc, max_element_size, min_subdivision,
        &x_triangles, &y_triangles, &z_triangles);

    Mesh3 mesh;
    if (z_triangles.empty()) {
        return mesh;
    }

    GridAxis x_grid(x_triangles), y_grid(y_triangles), z_grid(z_triangles);

    NAIVE_BRICKS_DEBUG(std::cerr
        << "volume_outside = " << plc.volume_outside << std::endl);

    Array2D<Plc3::VolumeId> volume_ids(
        x_grid.num_intervals(),
        y_grid.num_intervals(),
        plc.volume_outside);
    Array2D<NodeId> node_ids(
        /* num_points + num_intervals for second order bricks */
        x_grid.num_points() + x_grid.num_intervals(),
        y_grid.num_points() + y_grid.num_intervals(),
        NodeId::invalid());

    auto z_lower = z_triangles.begin();
    while (true) {
        auto z_upper = z_lower;
        ++z_upper;
        if (z_upper == z_triangles.end()) break;

        NAIVE_BRICKS_DEBUG(std::cerr
            << "apply_triangles at z = " << z_lower->first << std::endl);
        Array2D<Plc3::VolumeId> volume_ids_2(
            x_grid.num_intervals(), y_grid.num_intervals(), VOLUME_ID_UNSET);
        apply_triangles_to_update_volume_ids(
            plc,
            x_grid, y_grid, z_lower->second,
            volume_ids, &volume_ids_2
        );
        volume_ids = std::move(volume_ids_2);

        NAIVE_BRICKS_DEBUG(std::cerr
            << "create bricks at z = (" << z_lower->first
            << ", " << z_upper->first << ")" << std::endl);
        Array2D<NodeId> node_ids_2(
            x_grid.num_points() + x_grid.num_intervals(),
            y_grid.num_points() + y_grid.num_intervals(),
            NodeId::invalid());
        create_bricks(
            plc,
            element_type,
            x_grid, y_grid,
            z_lower->first, z_upper->first,
            volume_ids,
            &node_ids, &node_ids_2,
            &mesh);
        node_ids = std::move(node_ids_2);

        z_lower = z_upper;
    }

    update_face_attrs(
        plc,
        Dimension::X, Dimension::Y, Dimension::Z,
        0, 1,
        x_triangles, y_grid, z_triangles,
        &mesh);
    update_face_attrs(
        plc,
        Dimension::Y, Dimension::Z, Dimension::X,
        5, 3,
        y_grid, z_grid, x_triangles,
        &mesh);
    update_face_attrs(
        plc,
        Dimension::Z, Dimension::X, Dimension::Y,
        2, 4,
        z_grid, x_grid, y_triangles,
        &mesh);

    update_node_attrs(plc, &mesh);

    return mesh;
}

} /* namespace os2cx */
