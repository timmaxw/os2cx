#include <assert.h>

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "region.internal.hpp"

namespace os2cx {

const char *cube_off_text =
    "OFF\n"
    "8 12 0\n"
    "\n"
    "-1 -1 1\n"
    "1 -1 1\n"
    "1 1 1\n"
    "-1 1 -1\n"
    "-1 -1 -1\n"
    "1 -1 -1\n"
    "1 1 -1\n"
    "-1 1 1\n"
    "3  4 5 1\n"
    "3  0 4 1\n"
    "3  0 7 4\n"
    "3  4 7 3\n"
    "3  0 1 2\n"
    "3  7 0 2\n"
    "3  3 6 4\n"
    "3  4 6 5\n"
    "3  5 6 2\n"
    "3  1 5 2\n"
    "3  7 2 3\n"
    "3  3 2 6\n";

TEST(RegionTest, ReadWriteRegionOff) {
    std::istringstream stream1(cube_off_text);
    Region3 r = read_region_off(stream1);
    assert(r.i->p.size_of_vertices() == 8);
    assert(r.i->p.size_of_facets() == 12);

    std::ostringstream stream2;
    write_region_off(stream2, r);
    std::istringstream stream3(stream2.str());
    Region3 r_copy = read_region_off(stream3);
    assert(r_copy.i->p.size_of_vertices() == 8);
    assert(r_copy.i->p.size_of_facets() == 12);
}

} /* namespace os2cx */
