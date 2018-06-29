#include "result.hpp"

namespace os2cx {

void result_var_from_frd_analysis(
    const FrdAnalysis &fa,
    Results::Variable *var
) {
    if (fa.entities.size() == 4 &&
            fa.entities[0].ind1 == 1 &&
            fa.entities[1].ind1 == 2 &&
            fa.entities[2].ind1 == 3 &&
            fa.entities[3].exist == FrdEntity::Exist::ShouldCalculate) {
        const ContiguousMap<NodeId, double> &dx = fa.entities[0].data;
        const ContiguousMap<NodeId, double> &dy = fa.entities[1].data;
        const ContiguousMap<NodeId, double> &dz = fa.entities[2].data;
        ContiguousMap<NodeId, Vector> dxyz(
            dx.key_begin(), dx.key_end(), Vector(NAN, NAN, NAN));
        for (NodeId node = dx.key_begin(); node != dx.key_end(); ++node) {
            dxyz[node] = Vector(dx[node], dy[node], dz[node]);
        }
        var->node_vector.reset(new ContiguousMap<NodeId, Vector>(
            std::move(dxyz)));

    } else if (fa.entities.size() == 6 &&
            fa.entities[0].ind1 == 1 && fa.entities[0].ind2 == 1 &&
            fa.entities[1].ind1 == 2 && fa.entities[1].ind2 == 2 &&
            fa.entities[2].ind1 == 3 && fa.entities[2].ind2 == 3 &&
            fa.entities[3].ind1 == 1 && fa.entities[3].ind2 == 2 &&
            fa.entities[4].ind1 == 2 && fa.entities[4].ind2 == 3 &&
            fa.entities[5].ind1 == 3 && fa.entities[5].ind2 == 1) {
        const ContiguousMap<NodeId, double> sxx = fa.entities[0].data;
        const ContiguousMap<NodeId, double> syy = fa.entities[1].data;
        const ContiguousMap<NodeId, double> szz = fa.entities[2].data;
        const ContiguousMap<NodeId, double> sxy = fa.entities[3].data;
        const ContiguousMap<NodeId, double> syz = fa.entities[4].data;
        const ContiguousMap<NodeId, double> szx = fa.entities[5].data;
        Matrix nan_matrix;
        nan_matrix.cols[0] = nan_matrix.cols[1] = nan_matrix.cols[2] =
            Vector(NAN, NAN, NAN);
        ContiguousMap<NodeId, Matrix> sxyz(
            sxx.key_begin(), sxx.key_end(), nan_matrix);
        for (NodeId node = sxx.key_begin(); node != sxx.key_end(); ++node) {
            Matrix m;
            m.cols[0].x = sxx[node];
            m.cols[1].y = syy[node];
            m.cols[2].z = szz[node];
            m.cols[0].y = m.cols[1].x = sxy[node];
            m.cols[1].z = m.cols[2].y = syz[node];
            m.cols[2].x = m.cols[0].z = szx[node];
            sxyz[node] = m;
        }
        var->node_matrix.reset(new ContiguousMap<NodeId, Matrix>(
            std::move(sxyz)));

    } else {
        assert(false);
    }
}

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out
) {
    for (const FrdAnalysis &fa : frd_analyses) {
        if (fa.ctype == FrdAnalysis::CType::Static) {
            Results::VariableSet *step =
                &results_out->static_steps[fa.analys];
            Results::Variable *var = &step->vars[fa.name];
            result_var_from_frd_analysis(fa, var);
        }
    }
}

} /* namespace os2cx */
