#include <fstream>

#include <gtest/gtest.h>

#include "mesher_external.hpp"

namespace os2cx {

TEST(MesherExternalTest, MesherNetgen) {
    Region3 region = Region3::box(-1, -1, -1, 1, 1, 1);

    TempDir temp_dir(
        "./test_external_meshersXXXXXX",
        TempDir::AutoCleanup::Yes);
    Mesh3 mesh = mesher_netgen(region, temp_dir.path(), "cube_test");

    assert(mesh.nodes.size() == 9);
    assert(mesh.elements.size() == 12);
}

} /* namespace os2cx */

