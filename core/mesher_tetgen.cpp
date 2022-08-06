#include "mesher_tetgen.hpp"

#include <limits>

#define TETLIBRARY
#include <tetgen.h>

#include "plc_index.hpp"

namespace os2cx {

void convert_input(
    const Plc3 &plc,
    MaxElementSize max_element_size_default,
    const AttrOverrides<MaxElementSize> &max_element_size_overrides,
    tetgenio *tetgen
) {
    tetgen->numberofpoints = plc.vertices.size();
    tetgen->pointlist = new REAL[tetgen->numberofpoints * 3];
    for (Plc3::VertexId vid = 0; vid < (int)plc.vertices.size(); ++vid) {
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

    /* We'll have facet constraints on external surfaces, but not internal ones.
    So plc.surfaces.size() is an upper bound on the number of facet constraints.
    We'll calculate the true number as we go. */
    tetgen->numberoffacetconstraints = 0;
    tetgen->facetconstraintlist = new REAL[2 * plc.surfaces.size()];

    int facet_counter = 0;
    for (Plc3::SurfaceId sid = 0;
            sid < static_cast<int>(plc.surfaces.size()); ++sid) {
        const Plc3::Surface &surface = plc.surfaces[sid];

        int facetmarker;
        MaxElementSize max_element_size;
        if (surface.volumes[0] == plc.volume_outside) {
            facetmarker = sid + 1;
            max_element_size = max_element_size_overrides.lookup(
                plc.volumes[surface.volumes[1]].attrs,
                max_element_size_default);

        } else if (surface.volumes[1] == plc.volume_outside) {
            facetmarker = sid + 1;
            max_element_size = max_element_size_overrides.lookup(
                plc.volumes[surface.volumes[0]].attrs,
                max_element_size_default);

        } else {
            /* TODO: This may behave strangely if the Plc3 has complex internal
            surfaces. E.g. imagine using os2cx_select_surface_internal() to
            select _half_ of the cross-section of a solid object, and then again
            to select the other half. We would correctly generate facets for the
            cross-section, so tetgen wouldn't generate any tetrahedra straddling
            the cross-section. However, the facets for the two halves of the
            cross-section would all have facetmarker 0, so tetgen might generate
            tetrahedra that straddled the two halves of the cross-section. */
            facetmarker = 0;
        }

        if (facetmarker != 0) {
            /* Note, we have no way of applying max_element_size constraints
            to internal surfaces. So e.g. a max_element_size_override on a
            purely-internal volume would have no effect :( */
            int fcid = tetgen->numberoffacetconstraints++;
            tetgen->facetconstraintlist[2 * fcid] = facetmarker;
            tetgen->facetconstraintlist[2 * fcid + 1] =
                (max_element_size * max_element_size) / 2;
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
                int vid = tri.vertices[i];
                facet->polygonlist[0].vertexlist[i] = vid;
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
        if (nid.to_int() < (int)plc.vertices.size()) {
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
    MaxElementSize max_element_size_default,
    const AttrOverrides<MaxElementSize> &max_element_size_overrides,
    ElementType element_type
) {
    tetgenio tetgen_input;
    convert_input(
        plc,
        max_element_size_default,
        max_element_size_overrides,
        &tetgen_input);

    /* Tetgen always respects the PLC exactly. If the PLC is malformed such that
    it e.g. has two edges that are very close together, then Tetgen may try to
    generate an enormous number of tiny tetrahedra to model this accurately. Cap
    the number of Steiner points that Tetgen is allowed to insert, in order to
    force Tetgen to abort if this happens. */
    Volume approx_volume = pow(2 * plc.compute_approx_scale(), 3);
    Volume approx_tet_volume = pow(max_element_size_default, 3) / 6.0;
    int approx_num_tets = approx_volume / approx_tet_volume;
    int max_steiner_points = std::max(3 * approx_num_tets, 100);

    std::string flags;
    flags += "p";
    flags += "q1.414";
    flags += "S" + std::to_string(max_steiner_points);
    flags += "Q";

    if (element_type == ElementType::C3D4) {
        (void)0;
    } else if (element_type == ElementType::C3D10) {
        flags += "o2";
    } else {
        throw TetgenError(
            "tetgen mesher only supports element_type C3D4 or C3D10");
    }

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

} /* namespace os2cx */
