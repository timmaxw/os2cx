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
    stream << "os2cx_analysis_custom([\"a\",\"b\"], unit_system=[\"m\",\"kg\",\"s\"]);" << std::endl;
    stream.close();

    Project project(scad_path);
    openscad_extract_inventory(&project);

    EXPECT_EQ(2, project.calculix_deck.size());
    EXPECT_EQ("a", project.calculix_deck[0]);
    EXPECT_EQ("b", project.calculix_deck[1]);
}

} /* namespace os2cx */

