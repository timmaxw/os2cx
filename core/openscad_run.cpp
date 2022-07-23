#include "openscad_run.hpp"

#include <assert.h>
#include <string.h>

#include <fstream>
#include <sstream>

#include <QProcess>

namespace os2cx {

OpenscadRun::OpenscadRun(
    const FilePath &input_path,
    const FilePath &geometry_path,
    const std::map<std::string, OpenscadValue> &defines)
    : geometry_path(geometry_path), has_geometry(true)
{
    QStringList args;
    args.push_back(input_path.c_str());
    args.push_back("-o");
    assert(geometry_path.substr(geometry_path.size() - 4) == ".off");
    args.push_back(geometry_path.c_str());
    for (const auto &pair : defines) {
        std::stringstream stream;
        stream << "-D" << pair.first << "=" << pair.second;
        args.push_back(stream.str().c_str());
    }

    process.reset(new QProcess);
    process->setProgram("openscad");
    process->setArguments(args);
    process->setProcessChannelMode(QProcess::MergedChannels);
}

OpenscadRun::~OpenscadRun() { }

void OpenscadRun::run() {
    process->start();
    if (!process->waitForFinished(-1)) {
        throw OpenscadRunError();
    }

    QByteArray output = process->readAllStandardOutput();
    const char *start = output.constData(), *end = start + output.length();
    const char *mark = start, *ptr = start;
    while (true) {
        if (*ptr == '\n') {
            handle_output_line(mark, ptr);
            ptr = mark = ptr + 1;
        } else if (ptr == end) {
            handle_output_line(mark, end);
            break;
        } else {
            ++ptr;
        }
    }

    int status = process->exitCode();
    if (!errors.empty() || (status != 0 && has_geometry)) {
        OpenscadRunError error;
        error.errors = errors;
        throw error;
    }
    assert(has_geometry || status == 1);

    if (has_geometry) {
        std::ifstream stream(geometry_path);
        geometry.reset(new Poly3(read_poly3_off(stream)));
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

