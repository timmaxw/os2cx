#include "util.hpp"

#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace os2cx {

/* Instead of going through and escaping every single special character, we just
wrap the entire argument in single quotes. This takes care of everything except
for the single quotes themselves, which we replace with the four-character
sequence '\'' to close the single-quoted block; add a single quote; then open
the next single-quoted block. */
void escape_command_line(std::stringstream &stream, const std::string &str) {
    stream << '\'';
    for (char c : str) {
        if (c == '\'') {
            stream << "'\\''";
        } else {
            stream << c;
        }
    }
    stream << '\'';
}

std::string build_command_line(
        const std::string &command,
        const std::vector<std::string> &args) {
    std::stringstream stream;
    escape_command_line(stream, command);
    for (const std::string &arg : args) {
        stream << ' ';
        escape_command_line(stream, arg);
    }
    return stream.str();
}

void maybe_create_directory(const std::string &directory) {
    int res = mkdir(directory.c_str(), 0755);
    if (res != 0 && errno != EEXIST) {
        throw std::runtime_error(
            "mkdir() failed: " + std::string(strerror(errno)));
    }
}

TempDir::TempDir(const std::string &tmplate, AutoCleanup ac) :
        auto_cleanup(AutoCleanup::No) {
    std::vector<char> scratch(tmplate.begin(), tmplate.end());
    scratch.push_back('\0');
    char *ptr = mkdtemp(scratch.data());
    if (ptr == nullptr) {
        throw std::runtime_error(
            "mkdtemp() failed: " + std::string(strerror(errno)));
    }
    _path = FilePath(ptr);
    auto_cleanup = ac;
}

TempDir::~TempDir() {
    if (auto_cleanup == AutoCleanup::Yes) {
        try {
            cleanup();
        } catch (const std::runtime_error &error) {
            std::cerr << "Error when trying to clean up temporary directory:\n"
                << error.what() << std::endl;
            /* Just leave it */
        }
    }
}

class DirWalker {
public:
    DirWalker(const char *path) {
        dir = opendir(path);
        if (dir == nullptr) {
            throw std::runtime_error(
                "opendir() failed: " + std::string(strerror(errno)));
        }
    }
    ~DirWalker() {
        if (dir != nullptr) {
            int res = closedir(dir);
            assert(res == 0);
        }
    }
    struct dirent *next() {
        struct dirent *result;
        int res = readdir_r(dir, &buffer, &result);
        if (res != 0) {
            throw std::runtime_error(
                "readdir_r() failed: " + std::string(strerror(errno)));
        }
        return result;
    }
private:
    DIR *dir;
    struct dirent buffer;
};

void TempDir::cleanup() {
    DirWalker dir_walker(_path.c_str());
    struct dirent *dirent;
    while ((dirent = dir_walker.next()) != nullptr) {
        if (strcmp(dirent->d_name, ".") == 0
            || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }
        std::string full_path = (_path + "/") + dirent->d_name;
        int res = remove(full_path.c_str());
        if (res != 0) {
            throw std::runtime_error(
                "remove(" + full_path + ") failed: " +
                std::string(strerror(errno)));
        }
    }
    int res = rmdir(_path.c_str());
    if (res != 0) {
        throw std::runtime_error(
            "rmdir(" + _path + ") failed: " + std::string(strerror(errno)));
    }
}

} /* namespace os2cx */
