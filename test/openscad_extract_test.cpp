#include <fstream>

#include <gtest/gtest.h>

#include "openscad_extract.hpp"
#include "util.hpp"

namespace os2cx {

TEST(OpenscadExtractTest, OpenscadExtractInventory) {
    TempDir temp_dir(
        "./test_run_inventoryXXXXXX",
        TempDir::AutoCleanup::Yes);

    FilePath scad_path = temp_dir.path() + "/test.scad";
    std::ofstream stream(scad_path);
    stream << "use <../openscad2calculix.scad>;" << std::endl;
    stream << "os2cx(\"a\");" << std::endl;
    stream << "os2cx(\"b\");" << std::endl;
    stream.close();

    Project project(scad_path, temp_dir.path());
    openscad_extract_inventory(&project);

    EXPECT_EQ(2, project.directives.size());
    EXPECT_EQ("a", project.directives[0]);
    EXPECT_EQ("b", project.directives[1]);
}

} /* namespace os2cx */

