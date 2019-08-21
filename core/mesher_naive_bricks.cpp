#include "mesher_naive_bricks.hpp"

#include <limits>
#include <map>

#define NAIVE_BRICKS_DEBUG(x) (void)0

namespace os2cx {

class TriangleRef {
public:
    TriangleRef() { }
    TriangleRef(Plc3::SurfaceId sid, int tix) :
        surface_id(sid), triangle_ix(tix) { }
    Plc3::SurfaceId surface_id;
    int triangle_ix;
};

static const Plc3::VolumeId VOLUME_ID_UNSET = -1;

class GridAxis {
public:
    typedef std::map<double, int>::const_iterator const_iterator;
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
    void add_point(double p) {
        points[p];
    }
    void precompute_indexes() {
        int i = 0;
        for (auto &pair : points) {
            pair.second = i;
            ++i;
        }
    }
    std::map<double, int> points;
};

template<class Value>
void subdivide_grid(
    double max_element_size,
    std::map<double, Value> *grid
) {
    auto lower = grid->begin();
    while (true) {
        auto upper = lower;
        ++upper;
        if (upper == grid->end()) break;

        int num_parts = std::max(
            static_cast<int>(ceil(
                (upper->first - lower->first) / max_element_size)),
            2); /* always split each grid cell into at least two parts */
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

void setup_grid(
    const Plc3 &plc,
    double max_element_size,
    GridAxis *x_grid_out,
    GridAxis *y_grid_out,
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
                x_grid_out->add_point(p0.x);
            } else if (p0.y == p1.y && p0.y == p2.y) {
                y_grid_out->add_point(p0.y);
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
                x_grid_out->add_point(p0.x);
                y_grid_out->add_point(p0.y);
            } else if (p0.x == p1.x && p0.z == p1.z) {
                x_grid_out->add_point(p0.x);
                (*z_triangles_out)[p0.z]; /* make sure entry in map exists */
            } else if (p0.y == p1.y && p0.z == p1.z) {
                y_grid_out->add_point(p0.y);
                (*z_triangles_out)[p0.z]; /* make sure entry in map exists */
            } else {
                throw NaiveBricksAlignmentError(p0, p1);
            }
        }
    }

    subdivide_grid(max_element_size, &x_grid_out->points);
    subdivide_grid(max_element_size, &y_grid_out->points);
    subdivide_grid(max_element_size, z_triangles_out);

    x_grid_out->precompute_indexes();
    y_grid_out->precompute_indexes();
}

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

inline void interpolate_y_from_x(
    Point p0,
    Point p1,
    double x_lower,
    double x_upper,
    MinMaxSet *y_out
) {
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }
    if (x_upper < p0.x || x_lower > p1.x) {
        return;
    }
    if (x_lower <= p0.x) {
        y_out->insert(p0.y);
    } else {
        y_out->insert(p0.y + (x_lower - p0.x) / (p1.x - p0.x) * (p1.y - p0.y));
    }
    if (x_upper >= p1.x) {
        y_out->insert(p1.y);
    } else {
        y_out->insert(p0.y + (x_upper - p0.x) / (p1.x - p0.x) * (p1.y - p0.y));
    }
}

void apply_triangles(
    const Plc3 &plc,
    const GridAxis &x_grid,
    const GridAxis &y_grid,
    const std::vector<TriangleRef> &triangles,
    const Array2D<Plc3::VolumeId> &volume_ids_before,
    Array2D<Plc3::VolumeId> *volume_ids_after
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
        if ((p1 - p0).cross(p2 - p0).z < 0) {
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

        MinMaxSet x_minmax;
        x_minmax.insert(p0.x);
        x_minmax.insert(p1.x);
        x_minmax.insert(p2.x);

        /* Shrink the area by epsilon to prevent rounding errors wherein a
        triangle overlaps the wrong area by a tiny amount */
        static const double epsilon = 1e-20;

        GridAxis::const_iterator x_min =
            x_grid.find_point_lte(x_minmax.min() + epsilon);
        GridAxis::const_iterator x_max =
            x_grid.find_point_gte(x_minmax.max() - epsilon);
        for (auto x_lower = x_min; x_lower != x_max; ++x_lower) {
            auto x_upper = x_lower;
            ++x_upper;

            MinMaxSet y_minmax;
            interpolate_y_from_x(
                p0, p1, x_lower->first, x_upper->first, &y_minmax);
            interpolate_y_from_x(
                p1, p2, x_lower->first, x_upper->first, &y_minmax);
            interpolate_y_from_x(
                p2, p0, x_lower->first, x_upper->first, &y_minmax);

            NAIVE_BRICKS_DEBUG(std::cerr << "considering strip "
                << "x = (" << x_lower->first << ", " << x_upper->first << "), "
                << "y = (" << y_minmax.min() << ", " << y_minmax.max() << ")"
                << std::endl);

            GridAxis::const_iterator y_min =
                y_grid.find_point_lte(y_minmax.min() + epsilon);
            GridAxis::const_iterator y_max =
                y_grid.find_point_gte(y_minmax.max() - epsilon);

            for (auto y_lower = y_min; y_lower != y_max; ++y_lower) {
                auto y_upper = y_lower;
                ++y_upper;
                NAIVE_BRICKS_DEBUG(std::cerr << "applying at cell "
                    << "x = (" << x_lower->first
                    << ", " << x_upper->first << "), "
                    << "y = (" << y_lower->first
                    << ", " << y_upper->first << ")"
                    << std::endl);

                int x_index = x_lower->second;
                int y_index = y_lower->second;
                assert(volume_ids_before(x_index, y_index) == volume_before);
                if ((*volume_ids_after)(x_index, y_index) == VOLUME_ID_UNSET) {
                    (*volume_ids_after)(x_index, y_index) = volume_after;
                } else {
                    assert(
                        (*volume_ids_after)(x_index, y_index) == volume_after);
                }
            }
        }
    }

    for (int x = 0; x < x_grid.num_intervals(); ++x) {
        for (int y = 0; y < y_grid.num_intervals(); ++y) {
            if ((*volume_ids_after)(x, y) == VOLUME_ID_UNSET) {
                (*volume_ids_after)(x, y) = volume_ids_before(x, y);
            }
        }
    }
}

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

    mesh->elements.push_back(element);

    NAIVE_BRICKS_DEBUG(std::cerr << "created brick" << std::endl);
}

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

            NAIVE_BRICKS_DEBUG(std::cerr
                << "x = (" << x_lower->first << ", " << x_upper->first << "), "
                << "y = (" << y_lower->first << ", " << y_upper->first << "), "
                << "z = (" << z_lower << ", " << z_upper << "), "
                << "vid = " << volume_ids(x_lower->second, y_lower->second)
                << std::endl);

            if (volume_ids(x_lower->second, y_lower->second) !=
                    plc.volume_outside) {
                create_brick(
                    element_type, order,
                    x_lower, x_upper, y_lower, y_upper, z_lower, z_upper,
                    z_lower_node_ids, &z_middle_node_ids, z_upper_node_ids,
                    mesh);
            }

            y_lower = y_upper;
        }

        x_lower = x_upper;
    }
}

Mesh3 mesher_naive_bricks(
    const Plc3 &plc,
    double max_element_size,
    ElementType element_type
) {
    GridAxis x_grid, y_grid;
    std::map<double, std::vector<TriangleRef> > z_triangles;
    setup_grid(plc, max_element_size, &x_grid, &y_grid, &z_triangles);

    Mesh3 mesh;
    if (z_triangles.empty()) {
        return mesh;
    }

    NAIVE_BRICKS_DEBUG(std::cerr
        << "volume_outside = " << plc.volume_outside << std::endl);

    Array2D<Plc3::VolumeId> volume_ids(
        x_grid.num_intervals(), y_grid.num_intervals(), plc.volume_outside);
    Array2D<NodeId> node_ids(
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
        apply_triangles(
            plc,
            x_grid, y_grid,
            z_lower->second,
            volume_ids, &volume_ids_2);
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

    return mesh;
}

} /* namespace os2cx */
