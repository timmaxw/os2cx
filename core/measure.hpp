#ifndef OS2CX_MEASURE_HPP_
#define OS2CX_MEASURE_HPP_

#include <string>

#include "result.hpp"

namespace os2cx {

class Project;

class Measure
{
public:
    double measure(
        const Project &project,
        const Results::Result::Step &step
    ) const;

    std::string subject; /* volume, surface, or node object */
    std::string dataset;
};

} /* namespace os2cx */

#endif
