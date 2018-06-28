#include "result.hpp"

namespace os2cx {

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out
) {
    for (const FrdAnalysis &fa : frd_analyses) {
        if (fa.ctype == FrdAnalysis::CType::Static &&
                fa.name == "DISP" &&
                fa.entities.size() == 4 &&
                fa.entities[0].name == "D1" &&
                fa.entities[1].name == "D2" &&
                fa.entities[2].name == "D3") {
            const ContiguousMap<NodeId, double> &dx = fa.entities[0].data;
            const ContiguousMap<NodeId, double> &dy = fa.entities[1].data;
            const ContiguousMap<NodeId, double> &dz = fa.entities[2].data;
            ContiguousMap<NodeId, Vector> dxyz(
                dx.key_begin(), dx.key_end(), Vector(NAN, NAN, NAN));
            for (NodeId node = dx.key_begin(); node != dx.key_end(); ++node) {
                dxyz[node] = Vector(dx[node], dy[node], dz[node]);
            }
            results_out->node_vectors.insert(
                std::make_pair(fa.name, std::move(dxyz)));
        }
    }
}

} /* namespace os2cx */
