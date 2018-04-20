#include <assert.h>

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
    run.wait();

    assert(run.echos.size() == 2);
    assert(run.echos[0].size() == 2);
    assert(run.echos[0][0] == OpenscadValue(123));
    assert(run.echos[0][1] == OpenscadValue(456));
    assert(run.echos[1].size() == 2);
    assert(run.echos[1][0] == OpenscadValue("foo"));
    assert(run.echos[1][1] == OpenscadValue("bar"));
    assert(run.geometry == nullptr);
}

} /* namespace os2cx */

