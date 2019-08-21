#include "plc.hpp"

namespace os2cx {

Length Plc3::compute_approx_scale() const {
    double max_dist = 0;
    for (const Plc3::Vertex &vertex : vertices) {
        max_dist = std::max(max_dist, fabs(vertex.point.x));
        max_dist = std::max(max_dist, fabs(vertex.point.y));
        max_dist = std::max(max_dist, fabs(vertex.point.z));
    }
    return Length(max_dist);
}

void Plc3::debug(std::ostream &stream) const {
    stream << "VERTICES " << vertices.size() << std::endl;
    for (Plc3::VertexId vid = 0;
            vid < static_cast<int>(vertices.size()); ++vid) {
        stream << "V" << vid << ' ' << vertices[vid].point << std::endl;
    }
    stream << "VOLUMES " << volumes.size() << std::endl;
    for (Plc3::VolumeId cid = 0;
            cid < static_cast<int>(volumes.size()); ++cid) {
        stream << "C" << cid;
        if (cid == volume_outside) {
            stream << " outside";
        }
        stream << std::endl;
    }
    stream << "SURFACES " << surfaces.size() << std::endl;
    for (Plc3::SurfaceId sid = 0;
            sid < static_cast<int>(surfaces.size()); ++sid) {
        stream << "S" << sid << " C" << surfaces[sid].volumes[0]
            << " C" << surfaces[sid].volumes[1];
        for (const Plc3::Surface::Triangle &triangle
                : surfaces[sid].triangles) {
            stream << " V" << triangle.vertices[0]
                << "-V" << triangle.vertices[1]
                << "-V" << triangle.vertices[2];
        }
        stream << std::endl;
    }
    stream << "BORDERS " << borders.size() << std::endl;
    for (Plc3::BorderId bid = 0;
            bid < static_cast<int>(borders.size()); ++bid) {
        stream << "B" << bid;
        for (Plc3::VertexId vid : borders[bid].vertices) {
            stream << ' ' << vertices[vid].point;
        }
        for (Plc3::SurfaceId sid : borders[bid].surfaces) {
            stream << " S" << sid;
        }
        stream << std::endl;
    }
}

} /* namespace os2cx */
