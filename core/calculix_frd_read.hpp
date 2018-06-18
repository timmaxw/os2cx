#ifndef OS2CX_CALCULIX_FRD_READ_HPP_
#define OS2CX_CALCULIX_FRD_READ_HPP_

#include <iostream>

#include "calc.hpp"
#include "mesh.hpp"
#include "util.hpp"

namespace os2cx {

class FrdEntity {
public:
    std::string name;

    int menu; /* always 1 */

    enum class Type {
        Scalar = 1,
        Vector_3 = 2,
        Matrix = 4,
        VectorAmplitudePhase_3 = 12
    };
    Type type;

    int ind1, ind2;

    enum class Exist {
        Provided = 0,
        ShouldCalculate = 1,
        ProvidedEarmarked = 2
    };
    Exist exist;

    std::string calculation_name;

    ContiguousMap<NodeId, double> data;
};

class FrdAnalysis {
public:
    std::string setname; /* not used */

    double value;

    std::string text;

    enum class CType {
        Static = 0,
        TimeStep = 1,
        Frequency = 2,
        LoadStep = 3,
        UserNamed = 4
    };
    CType ctype;

    int numstp;

    std::string analys;

    std::string name;

    enum class RType {
        NodalMaterialIndependent = 1,
        NodalMaterialDependent = 2,
        Element = 3
    };
    RType rtype;

    std::vector<FrdEntity> entities;
};

class CalculixFrdFileReadError : public std::runtime_error {
public:
    CalculixFrdFileReadError(const std::string &msg) :
        std::runtime_error(msg) { }
};

void read_calculix_frd(
    std::istream &stream,
    NodeId node_id_begin,
    NodeId node_id_end,
    std::vector<FrdAnalysis> *analyses_out);

} /* namespace os2cx */

#endif

