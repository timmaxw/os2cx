#include <iostream>

#include "project_run.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: os2cx path/to/file.scad" << std::endl;
        return 1;
    }
    std::string scad_path = argv[1];

    os2cx::Project project(scad_path, "os2cx_temp");
    os2cx::ProjectRunCallbacks callbacks;
    os2cx::project_run(&project, &callbacks);

    return 0;
}
