#include <fstream>

#include <gtest/gtest.h>

#include "openscad_run.hpp"
#include "util.hpp"

namespace os2cx {

TEST(OpenscadRunTest, OpenscadRun) {
    TempDir temp_dir(
        "./test_openscad_runXXXXXX",
        TempDir::AutoCleanup::Yes);

    FilePath scad_path = temp_dir.path() + "/test.scad";
    std::ofstream stream(scad_path);
    stream << "echo(123, 456);" << std::endl;
    stream << "echo(foo_variable, \"bar\");" << std::endl;
    stream.close();

    FilePath geometry_path = temp_dir.path() + "/test.off";
    std::map<std::string, OpenscadValue> defines;
    defines.insert(std::make_pair("foo_variable", OpenscadValue("foo")));
    OpenscadRun run(scad_path, geometry_path, defines);
    run.run();

    ASSERT_EQ(2, run.echos.size());
    ASSERT_EQ(2, run.echos[0].size());
    EXPECT_EQ(OpenscadValue(123), run.echos[0][0]);
    EXPECT_EQ(OpenscadValue(456), run.echos[0][1]);
    ASSERT_EQ(2, run.echos[1].size());
    EXPECT_EQ(OpenscadValue("foo"), run.echos[1][0]);
    EXPECT_EQ(OpenscadValue("bar"), run.echos[1][1]);
    EXPECT_EQ(nullptr, run.geometry);
}

} /* namespace os2cx */

