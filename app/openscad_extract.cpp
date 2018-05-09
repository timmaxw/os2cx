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

void check_name(
    Project *project,
    const std::string &new_object_type,
    const std::string &new_name
) {
    // TODO: Check character set

    std::string existing_object_type;
    if (project->mesh_objects.count(new_name)) {
        existing_object_type = "mesh";
    } else if (project->select_volume_objects.count(new_name)) {
        existing_object_type = "volume";
    } else if (project->load_objects.count(new_name)) {
        existing_object_type = "load";
    }
    if (!existing_object_type.empty()) {
        throw UsageError("Can't declare a new " + new_object_type + " named '" +
            new_name + "' because there already exists a " +
            existing_object_type + " named '" + new_name + "'.");
    }
}

void do_analysis_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
            echo[2].type != OpenscadValue::Type::Vector) {
        throw BadEchoError("malformed analysis_directive");
    }
    if (!project->calculix_deck.empty()) {
        throw UsageError("Can't have multiple os2cx_analysis_...() directives "
            "in the same file.");
    }
    for (const auto &subvalue : echo[2].vector_value) {
        if (subvalue.type != OpenscadValue::Type::String) {
            throw BadEchoError("malformed analysis_directive");
        }
        project->calculix_deck.push_back(subvalue.string_value);
    }
    if (project->calculix_deck.empty()) {
        throw BadEchoError("empty analysis_directive");
    }
}

void do_mesh_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
        echo[2].type != OpenscadValue::Type::String) {
        throw BadEchoError("malformed mesh_directive");
    }

    std::string name = echo[2].string_value;
    check_name(project, "mesh", name);

    project->mesh_objects.insert(std::make_pair(
        name,
        Project::MeshObject()
    ));
    project->volume_objects.insert(std::make_pair(
        name,
        Project::VolumeObject()
    ));
}

void do_select_volume_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 3 ||
        echo[2].type != OpenscadValue::Type::String) {
        throw BadEchoError("malformed select_volume_directive");
    }

    std::string name = echo[2].string_value;
    check_name(project, "volume", name);

    project->select_volume_objects.insert(std::make_pair(
        name,
        Project::SelectVolumeObject()
    ));
    project->volume_objects.insert(std::make_pair(
        name,
        Project::VolumeObject()
    ));
}

void do_load_volume_directive(
    Project *project,
    const std::vector<OpenscadValue> &echo
) {
    if (echo.size() != 5 ||
        echo[2].type != OpenscadValue::Type::String ||
        echo[3].type != OpenscadValue::Type::String ||
        echo[4].type != OpenscadValue::Type::Number) {
        throw BadEchoError("malformed load_directive");
    }

    std::string name = echo[2].string_value;
    check_name(project, "load", name);

    std::string volume = echo[3].string_value;
    if (!project->volume_objects.count(volume)) {
        throw UsageError("Load '" + name + "' refers to volume '" + volume +
            "', which does not exist (yet).");
    }
    project->load_objects[name].volume = volume;

    auto force_density = ForceDensityVector::raw(0, 0, echo[4].number_value);
    project->load_objects[name].force_density = force_density;
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
        throw UsageError("All toplevel geometry must be wrapped in a directive "
            "such as 'os2cx_mesh()'; for example, 'os2cx_mesh() cube(...)' "
            "instead of 'cube(...)'. Your file has some toplevel geometry that "
            "isn't properly wrapped.");
    }

    for (const std::vector<OpenscadValue> &echo : run->echos) {
        if (echo.size() == 0 ||
            echo[0] != OpenscadValue("__openscad2calculix")) {
            continue;
        }
        if (echo.size() == 1 ||
                echo[1].type != OpenscadValue::Type::String) {
            throw BadEchoError("missing or malformed subdirective");
        }
        if (echo[1].string_value == "analysis_directive") {
            do_analysis_directive(project, echo);
        } else if (echo[1].string_value == "mesh_directive") {
            do_mesh_directive(project, echo);
        } else if (echo[1].string_value == "select_volume_directive") {
            do_select_volume_directive(project, echo);
        } else if (echo[1].string_value == "load_volume_directive") {
            do_load_volume_directive(project, echo);
        } else {
            throw BadEchoError(
                "unknown directive: " + echo[1].string_value);
        }
    }

    if (project->calculix_deck.empty()) {
        throw UsageError("Please specify an os2cx_analysis_...() directive.");
    }
}

std::unique_ptr<Poly3> openscad_extract_poly3(
    Project *project,
    const std::string &object_type,
    const std::string &name
) {
    std::unique_ptr<OpenscadRun> run = call_openscad(
        project,
        name,
        { OpenscadValue(object_type), OpenscadValue(name) });
    if (!run->geometry) {
        throw UsageError("Empty " + object_type + " '" + name + "'.");
    }
    return std::move(run->geometry);
}

} /* namespace os2cx */
