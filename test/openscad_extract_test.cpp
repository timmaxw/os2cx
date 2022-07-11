#include <fstream>

#include <gtest/gtest.h>

#include "openscad_extract.hpp"
#include "util.hpp"

namespace os2cx {

TEST(OpenscadExtractTest, OpenscadExtractInventory) {
    std::ifstream file_checker("openscad2calculix.scad");
    ASSERT_FALSE(file_checker.fail()) <<
      "File openscad2calculix.scad not found. Tests should be run in the root os2cx/ directory.";
    file_checker.close();

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

    ASSERT_EQ(2, project.calculix_deck_raw.size());
    ASSERT_EQ("a", project.calculix_deck_raw[0].string_value);
    ASSERT_EQ("b", project.calculix_deck_raw[1].string_value);
}

} /* namespace os2cx */

