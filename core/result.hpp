#ifndef OS2CX_RESULT_HPP_
#define OS2CX_RESULT_HPP_

#include <map>
#include <memory>
#include <string>

#include "calculix_frd_read.hpp"
#include "mesh.hpp"

namespace os2cx {

class Results {
public:
    class Dataset {
    public:
        NodeId node_begin() const {
            if (node_scalar) return node_scalar->key_begin();
            if (node_vector) return node_vector->key_begin();
            if (node_complex_vector) return node_complex_vector->key_begin();
            if (node_matrix) return node_matrix->key_begin();
            assert(false);
        }
        NodeId node_end() const {
            if (node_scalar) return node_scalar->key_end();
            if (node_vector) return node_vector->key_end();
            if (node_complex_vector) return node_complex_vector->key_end();
            if (node_matrix) return node_matrix->key_end();
            assert(false);
        }

        /* Exactly one of these will be non-null */
        std::unique_ptr<ContiguousMap<NodeId, double> > node_scalar;
        std::unique_ptr<ContiguousMap<NodeId, Vector> > node_vector;
        std::unique_ptr<ContiguousMap<NodeId, ComplexVector> >
            node_complex_vector;
        std::unique_ptr<ContiguousMap<NodeId, Matrix> > node_matrix;
    };

    class Result {
    public:
        enum class Type {
            /* Static: There will only be one step. */
            Static,
            /* Eigenmode: Each eigenmode is represented by a step.
            Step::frequency is the eigenfrequency. */
            Eigenmode,
            /* Modal dynamic: Each driving frequency that was analyzed is
            represented by a step. Step::frequency is the frequency of the
            driving force. */
            ModalDynamic
        };
        class Step {
        public:
            std::map<std::string, Dataset> datasets;
            double frequency;
        };

        Type type;
        std::vector<Step> steps;

        /* Note that all steps are guaranteed to have the same number of
        datasets, with the same names and types. */
    };
    std::vector<Result> results;
};

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out);

} /* namespace os2cx */

#endif
