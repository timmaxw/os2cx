#ifndef OS2CX_REGION_HPP_
#define OS2CX_REGION_HPP_

#include <iostream>
#include <memory>

namespace os2cx {

class Region3Internal;

class Region3 {
public:
    static Region3 box(
        double x1, double y1, double z1,
        double x2, double y2, double z2);

    Region3();
    Region3(Region3 &&other);
    ~Region3();
    Region3 &operator=(Region3 &&other);

    std::unique_ptr<Region3Internal> i;
};

class Region_IOError : public std::runtime_error {
public:
    Region_IOError(const std::string &msg) :
        std::runtime_error(msg) { }
};

Region3 read_region_off(
    std::istream &stream);

void write_region_off(
    std::ostream &stream, const Region3 &region);

void write_region_stl_text(
    std::ostream &stream, const Region3 &region);

} /* namespace os2cx */

#endif

