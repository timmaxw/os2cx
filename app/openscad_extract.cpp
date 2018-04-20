#include "openscad_extract.hpp"

#include <iostream>

#include "openscad_run.hpp"

namespace os2cx {

std::unique_ptr<OpenscadRun> call_openscad(
    Project *project,
    const std::string &geometry_file_name,
    std::vector<OpenscadValue> &&mode
) {
    std::map<std::string, OpenscadValue> defines;
    defines.insert(std::make_pair(
        "__openscad2calculix_mode",
        OpenscadValue(std::move(mode))
    ));
    std::unique_ptr<OpenscadRun> run(new OpenscadRun(
        project->scad_path,
        project->temp_dir + "/" + geometry_file_name + ".off",
        defines
    ));
    run->wait();
    return std::move(run);
}

void do_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
            echo[2].type != OpenscadValue::Type::String) {
        throw BadEchoError("malformed directive command");
    }
    project->directives.push_back(echo[2].string_value);
}

void do_element_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
        echo[2].type != OpenscadValue::Type::String) {
        throw BadEchoError("malformed element_directive command");
    }

    // TODO: Check character set
    std::string name = echo[2].string_value;

    if (project->element_directives.find(name) !=
            project->element_directives.end()) {
        /* The first time we encountered an element directive with this
        name, we already captured all of the geometry associated with all of
        the element directives with this name, so we're already done. */
        return;
    }

    if (project->nset_directives.find(name) !=
            project->nset_directives.end()) {
        throw UsageError("Can't have both an element set and a node set "
            "named " + name);
    }

    project->element_directives.insert(std::make_pair(
        name,
        Project::ElementDirective()
    ));
}

void do_nset_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
        echo[2].type != OpenscadValue::Type::String) {
        throw BadEchoError("malformed nset_directive command");
    }

    // TODO: Check character set
    std::string name = echo[2].string_value;

    if (project->nset_directives.find(name) !=
            project->nset_directives.end()) {
        /* The first time we encountered an nset directive with this name,
        we already captured all of the geometry associated with all of the
        nset directives with this name, so we're already done. */
        return;
    }

    if (project->element_directives.find(name) !=
            project->element_directives.end()) {
        throw UsageError("Can't have both an element set and a node set "
            "named " + name);
    }

    project->nset_directives.insert(std::make_pair(
        name,
        Project::NSetDirective()
    ));
}

void do_volume_load_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 4 ||
        echo[2].type != OpenscadValue::Type::String ||
        echo[3].type != OpenscadValue::Type::Number) {
        throw BadEchoError("malformed volume_load_directive command");
    }

    // TODO: Check character set
    std::string name = echo[2].string_value;

    if (project->volume_load_directives.find(name) !=
            project->volume_load_directives.end()) {
        throw BadEchoError("can't have multiple loads with same name");
    }

    project->volume_load_directives[name].force_density =
        ForceDensityVector::raw(0, 0, echo[3].number_value);
}

void openscad_extract_inventory(Project *project) {
    std::unique_ptr<OpenscadRun> run = call_openscad(
        project,
        "inventory",
        { OpenscadValue("inventory") });

    for (const std::string &warning : run->warnings) {
        std::cerr << "openscad: WARNING: " << warning << std::endl;
    }

    if (run->geometry != nullptr) {
        throw UsageError("All toplevel geometry must be wrapped in a command "
            "such as 'calculix_solid()'; for example, 'calculix_solid() "
            "cube()' instead of 'cube(...)'. Your file has some toplevel "
            "geometry that isn't properly wrapped.");
    }

    for (const std::vector<OpenscadValue> &echo : run->echos) {
        if (echo.size() == 0 ||
            echo[0] != OpenscadValue("__openscad2calculix")) {
            continue;
        }
        if (echo.size() == 1 ||
                echo[1].type != OpenscadValue::Type::String) {
            throw BadEchoError("missing or malformed subcommand");
        }
        if (echo[1].string_value == "directive") {
            do_directive(project, echo);
        } else if (echo[1].string_value == "element_directive") {
            do_element_directive(project, echo);
        } else if (echo[1].string_value == "nset_directive") {
            do_nset_directive(project, echo);
        } else if (echo[1].string_value == "volume_load_directive") {
            do_volume_load_directive(project, echo);
        } else {
            throw BadEchoError(
                "unknown command: " + echo[1].string_value);
        }
    }
}

std::unique_ptr<Region3> openscad_extract_region(
    Project *project,
    const std::string &category,
    const std::string &name
) {
    assert(category == "element" || category == "nset" ||
        category == "volume_load");
    std::unique_ptr<OpenscadRun> run = call_openscad(
        project,
        name,
        { OpenscadValue(category), OpenscadValue(name) });
    if (!run->geometry) {
        throw UsageError(category + " " + name + " is empty.");
    }
    return std::move(run->geometry);
}

} /* namespace os2cx */
