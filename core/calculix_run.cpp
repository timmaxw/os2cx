#include "calculix_run.hpp"

#include <QProcess>

#include "util.hpp"

namespace os2cx {

void run_calculix(
    const std::string &temp_dir,
    const std::string &filename
) {
    QStringList args;
    args.push_back("-i");
    args.push_back(filename.c_str());
    QProcess process;
    process.setWorkingDirectory(temp_dir.c_str());
    process.start("ccx", args);
    if (!process.waitForFinished(-1)) {
        throw CalculixRunError("ccx failed");
    }
}

} /* namespace os2cx */
