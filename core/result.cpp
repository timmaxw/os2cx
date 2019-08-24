#include "result.hpp"

namespace os2cx {

void result_var_from_frd_analysis(
    const FrdAnalysis &fa,
    Results::Dataset *var
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
            fa.entities[0].ind1 == 1 && fa.entities[0].name == "MAG1" &&
            fa.entities[1].ind1 == 2 && fa.entities[1].name == "MAG2" &&
            fa.entities[2].ind1 == 3 && fa.entities[2].name == "MAG3" &&
            fa.entities[3].ind1 == 4 && fa.entities[3].name == "PHA1" &&
            fa.entities[4].ind1 == 5 && fa.entities[4].name == "PHA2" &&
            fa.entities[5].ind1 == 6 && fa.entities[5].name == "PHA3") {
        const ContiguousMap<NodeId, double> dxm = fa.entities[0].data;
        const ContiguousMap<NodeId, double> dym = fa.entities[1].data;
        const ContiguousMap<NodeId, double> dzm = fa.entities[2].data;
        const ContiguousMap<NodeId, double> dxp = fa.entities[3].data;
        const ContiguousMap<NodeId, double> dyp = fa.entities[4].data;
        const ContiguousMap<NodeId, double> dzp = fa.entities[5].data;
        ComplexVector nan_vector(
            std::complex<double>(NAN, NAN),
            std::complex<double>(NAN, NAN),
            std::complex<double>(NAN, NAN));
        ContiguousMap<NodeId, ComplexVector> dxyz(
            dxm.key_begin(), dxm.key_end(), nan_vector);
        for (NodeId node = dxm.key_begin(); node != dxm.key_end(); ++node) {
            dxyz[node] = ComplexVector(
                std::polar(dxm[node], dxp[node] / 360.0 * (2*M_PI)),
                std::polar(dym[node], dyp[node] / 360.0 * (2*M_PI)),
                std::polar(dzm[node], dzp[node] / 360.0 * (2*M_PI)));
        }
        var->node_complex_vector.reset(new ContiguousMap<NodeId, ComplexVector>(
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

bool same_datasets(
    const Results::Result::Step &a,
    const Results::Result::Step &b
) {
    auto a_it = a.datasets.begin();
    auto b_it = b.datasets.begin();
    while (a_it != a.datasets.end() || b_it != b.datasets.end()) {
        if (a_it == a.datasets.end()) return false;
        if (b_it == b.datasets.end()) return false;
        if (a_it->first != b_it->first) return false;
        ++a_it;
        ++b_it;
    }
    return true;
}

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out
) {
    /* Collect related FrdAnalysis records into a single Result::Step */
    std::vector<std::pair<const FrdAnalysis *, Results::Result::Step> > steps;
    for (const FrdAnalysis &fa : frd_analyses) {
        bool combine;
        if (steps.empty()) {
            combine = false;
        } else {
            const FrdAnalysis &prev = *steps.back().first;
            combine = (fa.analys == prev.analys)
                && (fa.ctype == prev.ctype)
                && (fa.numstp == prev.numstp)
                && (fa.rtype == prev.rtype)
                && (fa.text == prev.text)
                && (fa.value == prev.value);
        }
        if (!combine) {
            Results::Result::Step step;
            step.frequency = fa.value;
            steps.push_back(std::make_pair(&fa, std::move(step)));
        }
        Results::Result::Step *step = &steps.back().second;
        result_var_from_frd_analysis(fa, &step->datasets[fa.name]);
    }

    /* Collect related Result::Step records into a single Result */
    for (std::pair<const FrdAnalysis *, Results::Result::Step> &pair : steps) {
        if (pair.first->ctype == FrdAnalysis::CType::Static) {
            Results::Result result;
            result.type = Results::Result::Type::Static;
            result.steps.push_back(std::move(pair.second));
            results_out->results.push_back(std::move(result));
        } else if (pair.first->ctype == FrdAnalysis::CType::Frequency) {
            bool combine;
            if (results_out->results.empty()) {
                combine = false;
            } else {
                const Results::Result &prev = results_out->results.back();
                combine = (prev.type == Results::Result::Type::Eigenmode)
                    && same_datasets(prev.steps.back(), pair.second);
            }
            if (!combine) {
                Results::Result result;
                result.type = Results::Result::Type::Eigenmode;
                results_out->results.push_back(std::move(result));
            }
            results_out->results.back().steps.push_back(std::move(pair.second));
        } else if (pair.first->ctype == FrdAnalysis::CType::TimeStep) {
            bool combine;
            if (results_out->results.empty()) {
                combine = false;
            } else {
                const Results::Result &prev = results_out->results.back();
                combine = (prev.type == Results::Result::Type::ModalDynamic)
                    && same_datasets(prev.steps.back(), pair.second);
            }
            if (!combine) {
                Results::Result result;
                result.type = Results::Result::Type::ModalDynamic;
                results_out->results.push_back(std::move(result));
            }
            results_out->results.back().steps.push_back(std::move(pair.second));
        }
    }
}

} /* namespace os2cx */
