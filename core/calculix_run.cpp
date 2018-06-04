#include "calculix_run.hpp"

#include <process.hpp>

#include "util.hpp"

namespace os2cx {

void run_calculix(
    const std::string &temp_dir,
    const std::string &filename
) {
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back(filename);
    TinyProcessLib::Process process(
        build_command_line("ccx", args),
        temp_dir,
        /* TODO: trap stdout/stderr and do something useful */
        nullptr,
        nullptr
    );
    int status = process.get_exit_status();
    if (status != 0) {
        throw CalculixRunError("ccx failed");
    }
}

} /* namespace os2cx */
