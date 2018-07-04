#include "mesher_tetgen.hpp"

#define TETLIBRARY
#include <tetgen.h>

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

Mesh3 mesher_tetgen(const Plc3 &plc) {
    tetgenio tetgen_input;
    convert_input(plc, &tetgen_input);

    tetgenio tetgen_output;
    tetrahedralize(const_cast<char *>("pq1.414a0.1Qo2"), &tetgen_input, &tetgen_output);

    return convert_output(&tetgen_output);
}

} /* namespace os2cx */
