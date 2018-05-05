#include "mesher_external.hpp"

#include <process.hpp>

#include <fstream>

#include "calculix_inp_read.hpp"
#include "util.hpp"

namespace os2cx {

Mesh3 mesher_netgen(
    const Poly3 &poly3,
    const FilePath &temp_dir,
    const std::string &name
) {
    std::string stl_path = temp_dir + "/" + name + "_netgen.stl";

    {
        std::ofstream stl_stream(stl_path);
        write_poly3_stl_text(stl_stream, poly3);
    }

    std::string mesh_path = temp_dir + "/" + name + "_netgen.msh";

    std::vector<std::string> args;
    args.push_back("-batchmode");
    args.push_back("-geofile=" + stl_path);
    args.push_back("-meshfile=" + mesh_path);
    args.push_back("-meshfiletype=Abaqus Format");
    std::string command_line = build_command_line("netgen", args);

    std::vector<char> out_buffer;
    TinyProcessLib::Process process(
        command_line,
        "",
        [&out_buffer](const char *bytes, size_t n) {
            out_buffer.insert(out_buffer.end(), bytes, bytes+n);
        },
        [&out_buffer](const char *bytes, size_t n) {
            out_buffer.insert(out_buffer.end(), bytes, bytes+n);
        }
    );
    int status = process.get_exit_status();
    if (status != 0) {
        throw ExternalMesherError(
            "netgen exited with status code " + std::to_string(status)
            + "\n\nCommand:\n" + command_line
            + "\n\nOutput messages:\n"
            + std::string(out_buffer.begin(), out_buffer.end())
        );
    }

    Mesh3 mesh;
    mesh.nodes = ContiguousMap<NodeId, Node3>(
        NodeId::from_int(1));
    mesh.elements = ContiguousMap<ElementId, Element3>(
        ElementId::from_int(1));
    std::ifstream mesh_stream(mesh_path);
    try {
        read_calculix_nodes_and_elements(mesh_stream, &mesh);
    } catch (const CalculixFileReadError &error) {
        throw ExternalMesherError(
            std::string("netgen produced file we can't read: ") + error.what());
    }

    return mesh;
}

} /* namespace os2cx */

