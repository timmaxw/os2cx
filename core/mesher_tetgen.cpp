#include "mesher_tetgen.hpp"

#include <limits>

#define TETLIBRARY
#include <tetgen.h>

#include "plc_index.hpp"

namespace os2cx {

void convert_input(const Plc3 &plc, tetgenio *tetgen) {
    tetgen->numberofpoints = plc.vertices.size();
    tetgen->pointlist = new REAL[tetgen->numberofpoints * 3];
    for (Plc3::VertexId vid = 0;
            vid < static_cast<int>(plc.vertices.size()); ++vid) {
        tetgen->pointlist[3 * vid + 0] = plc.vertices[vid].point.x;
        tetgen->pointlist[3 * vid + 1] = plc.vertices[vid].point.y;
        tetgen->pointlist[3 * vid + 2] = plc.vertices[vid].point.z;
    }

    tetgen->numberoffacets = 0;
    for (const Plc3::Surface &surface : plc.surfaces) {
        tetgen->numberoffacets += surface.triangles.size();
    }
    tetgen->facetlist = new tetgenio::facet[tetgen->numberoffacets];
    tetgen->facetmarkerlist = new int[tetgen->numberoffacets];
    int facet_counter = 0;
    for (Plc3::SurfaceId sid = 0;
            sid < static_cast<int>(plc.surfaces.size()); ++sid) {
        const Plc3::Surface &surface = plc.surfaces[sid];
        int facetmarker;
        if (surface.volumes[0] == plc.volume_outside ||
                surface.volumes[1] == plc.volume_outside) {
            facetmarker = sid + 1;
        } else {
            /* TODO: How to denote features on internal faces? */
            facetmarker = 0;
        }
        for (const Plc3::Surface::Triangle &tri : surface.triangles) {
            tetgenio::facet *facet = &tetgen->facetlist[facet_counter];
            facet->polygonlist = new tetgenio::polygon[1];
            facet->numberofpolygons = 1;
            facet->holelist = nullptr;
            facet->numberofholes = 0;
            facet->polygonlist[0].numberofvertices = 3;
            facet->polygonlist[0].vertexlist = new int[3];
            for (int i = 0; i < 3; ++i) {
                facet->polygonlist[0].vertexlist[i] = tri.vertices[i];
            }
            tetgen->facetmarkerlist[facet_counter] = facetmarker;
            ++facet_counter;
        }
    }
}

Mesh3 convert_output(tetgenio *tetgen) {
    Mesh3 mesh;
    mesh.nodes = ContiguousMap<NodeId, Node3>(
        NodeId::from_int(0));
    mesh.elements = ContiguousMap<ElementId, Element3>(
        ElementId::from_int(0));

    mesh.nodes.reserve(tetgen->numberofpoints);
    for (int nid = 0; nid < tetgen->numberofpoints; ++nid) {
        Node3 node;
        node.point = Point(
            tetgen->pointlist[3 * nid + 0],
            tetgen->pointlist[3 * nid + 1],
            tetgen->pointlist[3 * nid + 2]);
        mesh.nodes.push_back(node);
    }

    mesh.elements.reserve(tetgen->numberoftetrahedra);
    for (int eid = 0; eid < tetgen->numberoftetrahedra; ++eid) {
        Element3 element;
        if (tetgen->numberofcorners == 4) {
            element.type = ElementType::C3D4;
        } else if (tetgen->numberofcorners == 10) {
            element.type = ElementType::C3D10;
        } else {
            assert(false);
        }
        const int *tetgen_corners =
            tetgen->tetrahedronlist + eid * tetgen->numberofcorners;
        for (int i = 0; i < 4; ++i) {
            element.nodes[i] = NodeId::from_int(tetgen_corners[i]);
        }
        if (tetgen->numberofcorners == 10) {
            element.nodes[5 - 1] = NodeId::from_int(tetgen_corners[7 - 1]);
            element.nodes[6 - 1] = NodeId::from_int(tetgen_corners[8 - 1]);
            element.nodes[7 - 1] = NodeId::from_int(tetgen_corners[10 - 1]);
            element.nodes[8 - 1] = NodeId::from_int(tetgen_corners[6 - 1]);
            element.nodes[9 - 1] = NodeId::from_int(tetgen_corners[9 - 1]);
            element.nodes[10 - 1] = NodeId::from_int(tetgen_corners[5 - 1]);
        }
        mesh.elements.push_back(element);
    }

    return mesh;
}

void transfer_attrs(const Plc3 &plc, Mesh3 *mesh) {
    Plc3Index plc_index(&plc);

    for (NodeId nid = mesh->nodes.key_begin();
            nid < mesh->nodes.key_end(); ++nid) {
        Node3 *node = &mesh->nodes[nid];
        if (nid.to_int() < plc.vertices.size()) {
            /* This node corresponds directly to the Plc3 vertex with the same
            numerical ID. */
            Plc3::VertexId vid = nid.to_int();
            const Plc3::Vertex &vertex = plc.vertices[vid];
            assert(vertex.point == node->point);
            node->attrs = vertex.attrs;
        } else {
            /* This node was created artificially by tetgen. Set the attrs to
            empty. This isn't technically correct, but node attrs only matter
            for purposes of os2cx_select_node(), so this is fine. */
            node->attrs = AttrBitset();
        }
    }

    for (ElementId eid = mesh->elements.key_begin();
            eid != mesh->elements.key_end(); ++eid) {
        Element3 *element = &mesh->elements[eid];

        LengthVector sum = LengthVector::zero();
        int num_nodes = element->num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            sum += mesh->nodes[element->nodes[i]].point - Point::origin();
        }
        Point center = Point::origin() + sum / num_nodes;

        Plc3::VolumeId volume_id = plc_index.volume_containing_point(center);
        element->attrs = plc.volumes[volume_id].attrs;

        FaceId fid;
        fid.element_id = eid;
        const ElementTypeShape *shape = &element_type_shape(element->type);
        for (fid.face = 0; fid.face < static_cast<int>(shape->faces.size());
                ++fid.face) {
            LengthVector sum = LengthVector::zero();
            for (int vertex_index : shape->faces[fid.face].vertices) {
                sum += mesh->nodes[element->nodes[vertex_index]].point
                    - Point::origin();
            }
            Point center = Point::origin()
                + sum / shape->faces[fid.face].vertices.size();

            Plc3::SurfaceId surface_id =
                plc_index.surface_containing_point(center);
            if (surface_id == -1) {
                /* internal face, not on any surface */
                element->face_attrs[fid.face] = plc.volumes[volume_id].attrs;
            } else {
                /* copy attrs of the surface */
                element->face_attrs[fid.face] = plc.surfaces[surface_id].attrs;
            }
        }
    }
}

Mesh3 mesher_tetgen(
    const Plc3 &plc,
    double max_element_size
) {
    tetgenio tetgen_input;
    convert_input(plc, &tetgen_input);

    double bbox_volume = compute_bbox_volume(plc);

    /* Tetgen doesn't let us limit tetrahedron length, but it does let us limit
    tetrahedron volume. Convert max_element_size into a roughly equivalent
    max_tet_volume. */
    double max_tet_volume = pow(max_element_size, 3) / 6.0;

    /* Tetgen always respects the PLC exactly. If the PLC is malformed such that
    it e.g. has two edges that are very close together, then Tetgen may try to
    generate an enormous number of tiny tetrahedra to model this accurately. Cap
    the number of Steiner points that Tetgen is allowed to insert, in order to
    force Tetgen to abort if this happens. */
    int max_steiner_points = std::max(
        3 * static_cast<int>(bbox_volume / max_tet_volume),
        100);

    std::string flags;
    flags += "p";
    flags += "q1.414";
    flags += "a" + std::to_string(max_tet_volume);
    flags += "S" + std::to_string(max_steiner_points);
    flags += "o2";
    flags += "Q";

    tetgenio tetgen_output;
    try {
        tetrahedralize(
            const_cast<char *>(flags.c_str()),
            &tetgen_input,
            &tetgen_output);
    } catch (int error_code) {
        if (error_code == 1) {
            throw std::bad_alloc();
        } else if (error_code == 2) {
            throw TetgenError("Tetgen has a bug in it");
        } else if (error_code == 3) {
            throw TetgenError("Tetgen found self-intersection");
        } else if (error_code == 4) {
            throw TetgenError("Tetgen found very small input feature");
        } else if (error_code == 5) {
            throw TetgenError("Tetgen found very close input facets");
        } else if (error_code == 10) {
            throw TetgenError("Tetgen input was not valid");
        } else {
            throw TetgenError("Tetgen threw an unknown error");
        }
    }

    Mesh3 mesh = convert_output(&tetgen_output);

    transfer_attrs(plc, &mesh);

    return mesh;
}

double compute_bbox_volume(const Plc3 &plc) {
    double xmin, xmax, ymin, ymax, zmin, zmax;
    xmin = ymin = zmin = std::numeric_limits<double>::max();
    xmax = ymax = zmax = std::numeric_limits<double>::min();
    for (const Plc3::Vertex &v : plc.vertices) {
        xmax = std::max(xmax, v.point.x);
        xmin = std::min(xmin, v.point.x);
        ymax = std::max(ymax, v.point.y);
        ymin = std::min(ymin, v.point.y);
        zmax = std::max(zmax, v.point.z);
        zmin = std::min(zmin, v.point.z);
    }
    return (xmax - xmin) * (ymax - ymin) * (zmax - zmin);
}

double suggest_max_element_size(const Plc3 &plc) {
    double bbox_volume = compute_bbox_volume(plc);

    /* Simulating 10,000 second-order tetrahedra takes a few seconds of CPU
    time and a less than a gigabyte of RAM, which makes it a safe default. */
    static const int default_max_tets = 10000;

    /* In practice, tetgen seems to produce tets with an average volume of about
    half of max_tet_volume, in bulk. So apply a fudge factor. */
    static constexpr double fudge_factor = 0.5;

    /* Choose max_tet_volume such that at most default_max_tets can fit in the
    bounding box. In practice the shape won't completely fill the bounding box,
    so we'll end up with fewer than default_max_tets (often far fewer), but it's
    an OK approximation. */
    double max_tet_volume = bbox_volume / default_max_tets / fudge_factor;

    /* Convert max_tet_volume into a roughly equivalent max_element_size */
    double max_element_size = pow(max_tet_volume * 6.0, 1 / 3.0);

    return max_element_size;
}

} /* namespace os2cx */
