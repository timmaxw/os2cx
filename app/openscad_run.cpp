#include "openscad_run.hpp"

#include <assert.h>
#include <string.h>

#include <fstream>
#include <sstream>

#include <process.hpp>

namespace os2cx {

OpenscadRun::OpenscadRun(
    const FilePath &input_path,
    const FilePath &geometry_path,
    const std::map<std::string, OpenscadValue> &defines)
    : geometry_path(geometry_path), has_geometry(true)
{
    std::vector<std::string> args;
    args.push_back(input_path);
    args.push_back("-o");
    assert(geometry_path.substr(geometry_path.size() - 4) == ".off");
    args.push_back(geometry_path);
    for (const std::pair<std::string, OpenscadValue> &pair : defines) {
        std::stringstream stream;
        stream << "-D" << pair.first << "=" << pair.second;
        args.push_back(stream.str());
    }

    process.reset(new TinyProcessLib::Process(
        build_command_line("openscad", args),
        "",
        nullptr,
        [this](const char *bytes, size_t n) { handle_output_chunk(bytes, n); }
    ));
}

OpenscadRun::~OpenscadRun() { }

void OpenscadRun::wait() {
    int status = process->get_exit_status();

    if (!output_leftover.empty()) {
        output_leftover.push_back('\n');
        handle_output_line(
            output_leftover.data(),
            output_leftover.data() + output_leftover.size() - 1);
        output_leftover.clear();
    }

    if (!errors.empty() || (status != 0 && has_geometry)) {
        OpenscadRunError error;
        error.errors = errors;
        throw error;
    }
    assert(has_geometry || status == 1);

    if (has_geometry) {
        std::ifstream stream(geometry_path);
        geometry.reset(new Region3(read_region_off(stream)));
    }
}

void OpenscadRun::handle_output_chunk(const char *bytes, size_t n) {
    const char *mark = bytes;
    for (const char *ptr = bytes; ptr != bytes + n; ++ptr) {
        if (*ptr == '\n') {
            if (!output_leftover.empty()) {
                output_leftover.insert(output_leftover.end(), mark, ptr + 1);
                handle_output_line(
                    output_leftover.data(),
                    output_leftover.data() + output_leftover.size() - 1);
                output_leftover.clear();
            } else {
                handle_output_line(mark, ptr);
            }
            mark = ptr + 1;
        }
    }
    if (mark != bytes + n) {
        output_leftover.insert(output_leftover.end(), mark, bytes + n);
    }
}

void OpenscadRun::handle_output_line(const char *begin, const char *end) {
    std::string line(begin, end);
    if (line.substr(0, 6) == "ECHO: ") {
        echos.push_back(OpenscadValue::parse_many(line.data() + 6));
    } else if (line == "Current top level object is empty.") {
        has_geometry = false;
    } else if (line.substr(0, 7) == "ERROR: ") {
        errors.push_back(line.substr(7));
    } else if (line.substr(0, 9) == "WARNING: ") {
        warnings.push_back(line.substr(9));
    }
}

} /* namespace os2cx */

