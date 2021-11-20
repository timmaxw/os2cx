/* When the openscad2calculix executable is running OpenSCAD, it will redefine
this to a different value. "preview" is only used in the interactive OpenSCAD
editor. */
__openscad2calculix_mode = ["preview"];

function __os2cx_is_string(arg) = (str(arg) == arg);
function __os2cx_is_number(arg) = (arg != undef) && (arg + 0 == arg);
function __os2cx_is_vector_3(arg) =
    len(arg) == 3
    && __os2cx_is_number(arg[0])
    && __os2cx_is_number(arg[1])
    && __os2cx_is_number(arg[2]);
function __os2cx_is_array_strings(arg) =
    [for (item = arg) if (!__os2cx_is_string(item)) false] == [];
function __os2cx_is_number_with_unit(arg) =
    len(arg) == 2
    && __os2cx_is_number(arg[0])
    && __os2cx_is_string(arg[1]);
function __os2cx_is_vector_3_with_unit(arg) =
    len(arg) == 2
    && __os2cx_is_vector_3(arg[0])
    && __os2cx_is_string(arg[1]);

module __os2cx_beacon() {
    origin_coords   = [0,   0,   0  ];
    concave_coords  = [0.1, 0.2, 0.3];
    corner_x_coords = [1,   0,   0  ];
    corner_y_coords = [0,   1,   0  ];
    corner_z_coords = [0,   0,   1  ];
    origin_id   = 0;
    concave_id  = 1;
    corner_x_id = 2;
    corner_y_id = 3;
    corner_z_id = 4;
    polyhedron(
        points = [
            origin_coords,
            concave_coords,
            corner_x_coords,
            corner_y_coords,
            corner_z_coords,
        ],
        faces = [
            [origin_id, corner_x_id, corner_y_id],
            [origin_id, corner_y_id, corner_z_id],
            [origin_id, corner_z_id, corner_x_id],
            [concave_id, corner_y_id, corner_x_id],
            [concave_id, corner_z_id, corner_y_id],
            [concave_id, corner_x_id, corner_z_id],
        ],
        convexity = 2
    );
}

module __os2cx_check_existing(object_type, referrer, name) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: ", referrer, " refers to a ", object_type,
                " called ", name, " but that's not a valid name. ",
                "(It needs to be a string.)"));
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "check_existing_directive",
            object_type, referrer, name);
    }
}

module os2cx_analysis_custom(lines, unit_system=undef) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_array_strings(lines)) {
            echo(str("ERROR: os2cx_analysis_custom() parameter must be an ",
                "array of strings"));
        }
        if (!__os2cx_is_array_strings(unit_system) || len(unit_system) != 3) {
            echo(str("ERROR: os2cx_analysis_custom() 'unit_system' parameter ",
                "must be an array of three strings."));
        }
        echo(str("NOTE: To run the CalculiX simulation, open this .scad file ",
            "using the OpenSCAD2CalculiX application."));
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "analysis_directive", lines, unit_system);
    }
}

/* os2cx_analysis_static_simple() simulates a static load applied to a structure
attached to a fixed base. For example, the weight of a person standing on a
footbridge. */

module os2cx_analysis_static_simple(
    mesh=undef,
    material=undef,
    fixed=undef,
    load=undef,
    length_unit=undef
) {
    __os2cx_check_existing(
        "mesh",
        "os2cx_analysis_static_simple() 'mesh' parameter",
        mesh);
    __os2cx_check_existing(
        "material",
        "os2cx_analysis_static_simple() 'material' parameter",
        material);
    __os2cx_check_existing(
        "surface",
        "os2cx_analysis_static_simple() 'fixed' parameter",
        fixed);
    __os2cx_check_existing(
        "load",
        "os2cx_analysis_static_simple() 'load' parameter",
        load);
    if (!__os2cx_is_string(length_unit)) {
        echo(str("ERROR: os2cx_analysis_static_simple() 'length_unit' ",
            "parameter must be a string"));
    }
    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        str("*SOLID SECTION, Elset=E", mesh, ", Material=", material),
        "*STEP",
        "*STATIC",
        "*BOUNDARY",
        str("N", fixed, ",1,3"),
        "*CLOAD",
        str("*INCLUDE, INPUT=", load, ".clo"),
        str("*NODE FILE, Nset=N", mesh),
        "U",
        "*EL FILE",
        "S",
        "*END STEP"
    ], unit_system=[length_unit, "kg", "s"]);
}

/* os2cx_analysis_steady_state_dynamics() simulates oscillating loads applied to
a structure attached to a fixed base. The load is simulated at a range of
frequencies. */

module os2cx_analysis_steady_state_dynamics(
    mesh=undef,
    material=undef,
    fixed=undef,
    load=undef,
    length_unit=undef,
    num_eigenfrequencies=10,
    min_frequency=undef,
    max_frequency=undef,
    damping_ratio=undef
) {
    __os2cx_check_existing(
        "mesh",
        "os2cx_analysis_steady_state_dynamics() 'mesh' parameter",
        mesh);
    __os2cx_check_existing(
        "material",
        "os2cx_analysis_steady_state_dynamics() 'material' parameter",
        material);
    __os2cx_check_existing(
        "surface",
        "os2cx_analysis_steady_state_dynamics() 'fixed' parameter",
        fixed);
    __os2cx_check_existing(
        "load",
        "os2cx_analysis_steady_state_dynamics() 'load' parameter",
        load);
    if (!__os2cx_is_string(length_unit)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics() 'length_unit' ",
            "parameter must be a string"));
    }
    if (!__os2cx_is_number(num_eigenfrequencies)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics() ",
            "'num_eigenfrequencies' parameter must be a number"));
    }
    if (!__os2cx_is_number_with_unit(min_frequency) ||
            min_frequency[1] != "Hz") {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics() ",
            "'min_frequency' parameter must be [a number, \"Hz\"]"));
    }
    if (!__os2cx_is_number_with_unit(max_frequency) ||
            max_frequency[1] != "Hz") {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics() ",
            "'max_frequency' parameter must be [a number, \"Hz\"]"));
    }
    if (!__os2cx_is_number(damping_ratio)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics() ",
            "'damping_ratio' parameter must be a number"));
    }
    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        str("*SOLID SECTION, Elset=E", mesh, ", Material=", material),
        "*BOUNDARY",
        str("N", fixed, ",1,3"),
        "*STEP",
        "*FREQUENCY,STORAGE=yes",
        str(num_eigenfrequencies),
        str("*NODE FILE, Nset=N", mesh),
        "U",
        "*END STEP",
        "*STEP",
        "*MODAL DAMPING",
        str("1", ",", num_eigenfrequencies, ",", damping_ratio),
        "*STEADY STATE DYNAMICS",
        str(min_frequency[0], ",", max_frequency[0], ",", 10),
        "*CLOAD",
        str("*INCLUDE, INPUT=", load, ".clo"),
        str("*NODE FILE, Nset=N", mesh),
        "PU,U",
        "*EL FILE",
        "S",
        "*END STEP"
    ], unit_system=[length_unit, "kg", "s"]);
}

/* os2cx_analysis_steady_state_dynamics_osc_boundary() simulates a structure
attached to an oscillating base. For example, a tower during an earthquake. */

module os2cx_analysis_steady_state_dynamics_osc_boundary(
    mesh=undef,
    material=undef,
    boundary=undef,
    oscillation=undef,
    length_unit=undef,
    num_eigenfrequencies=10,
    min_frequency=undef,
    max_frequency=undef,
    rayleigh_damping_alpha=undef,
    rayleigh_damping_beta=undef,
) {
    __os2cx_check_existing(
        "mesh",
        "os2cx_analysis_steady_state_dynamics_osc_boundary() 'mesh' parameter",
        mesh);
    __os2cx_check_existing(
        "material",
        "os2cx_analysis_steady_state_dynamics_osc_boundary() 'material' parameter",
        material);
    __os2cx_check_existing(
        "surface",
        "os2cx_analysis_steady_state_dynamics_osc_boundary() 'boundary' parameter",
        boundary);
    if (!__os2cx_is_vector_3_with_unit(oscillation)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
          "'oscillation' must be a [vector, unit] pair"));
    }
    if (!__os2cx_is_string(length_unit)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'length_unit' parameter must be a string"));
    }
    if (oscillation[1] != length_unit) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "oscillation units must be same as length_unit"));
    }
    if (!__os2cx_is_number(num_eigenfrequencies)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'num_eigenfrequencies' parameter must be a number"));
    }
    if (!__os2cx_is_number_with_unit(min_frequency) ||
            min_frequency[1] != "Hz") {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'min_frequency' parameter must be [a number, \"Hz\"]"));
    }
    if (!__os2cx_is_number_with_unit(max_frequency) ||
            max_frequency[1] != "Hz") {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'max_frequency' parameter must be [a number, \"Hz\"]"));
    }
    if (!__os2cx_is_number(rayleigh_damping_alpha)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'rayleigh_damping_alpha' parameter must be a number"));
    }
    if (!__os2cx_is_number(rayleigh_damping_beta)) {
        echo(str("ERROR: os2cx_analysis_steady_state_dynamics_osc_boundary() ",
            "'rayleigh_damping_beta' parameter must be a number"));
    }
    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        str("*SOLID SECTION, Elset=E", mesh, ", Material=", material),
        "*BOUNDARY",
        str("N", boundary, ",1,3"),
        "*STEP",
        "*FREQUENCY,STORAGE=yes",
        str(num_eigenfrequencies),
        str("*NODE FILE, Nset=N", mesh),
        "U",
        "*END STEP",
        "*STEP",
        "*MODAL DAMPING,RAYLEIGH",
        str(",,", rayleigh_damping_alpha, ",", rayleigh_damping_beta),
        "*STEADY STATE DYNAMICS",
        str(min_frequency[0], ",", max_frequency[0], ",", 10),
        "*BOUNDARY",
        str("N", boundary, ",1,1,", oscillation[0][0]),
        str("N", boundary, ",2,2,", oscillation[0][1]),
        str("N", boundary, ",3,3,", oscillation[0][2]),
        str("*NODE FILE, Nset=N", mesh),
        "PU,U",
        "*EL FILE",
        "S",
        "*END STEP"
    ], unit_system=[length_unit, "kg", "s"]);
}

module os2cx_mesh(name, mesher="tetgen", max_element_size=undef) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_mesh() name must be a string");
        }
        if (!__os2cx_is_string(mesher)) {
            echo("ERROR: os2cx_mesh() mesher must be a string");
        }
        if (max_element_size != undef && !__os2cx_is_number(max_element_size)) {
            echo("ERROR: os2cx_mesh() max_element_size must be a number");
        }
        children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "mesh_directive",
            name, mesher, max_element_size);
    } else if (__openscad2calculix_mode == ["mesh", name]) {
        children();
    }
}

module os2cx_slice(name, direction_vector, direction_angle_tolerance) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_slice() first parameter must be a string");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "slice_directive",
            name,
            direction_vector,
            direction_angle_tolerance);
    } else if (__openscad2calculix_mode == ["slice", name]) {
        children();
    }
}

module os2cx_select_volume(name) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_select_volume() parameter must be a string");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_volume_directive", name);
    } else if (__openscad2calculix_mode == ["select_volume", name]) {
        children();
    }
}

module os2cx_select_surface(name, direction_vector, direction_angle_tolerance) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_select_surface() parameter must be a string");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_surface_directive",
            name,
            "external",
            direction_vector,
            direction_angle_tolerance);
    } else if (__openscad2calculix_mode == ["select_surface", name]) {
        children();
    }
}

module os2cx_select_surface_internal(
    name, direction_vector, direction_angle_tolerance
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_select_surface() parameter must be a string");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_surface_directive",
            name,
            "internal",
            direction_vector,
            direction_angle_tolerance);
    } else if (__openscad2calculix_mode == ["select_surface", name]) {
        children();
    }
}

module os2cx_select_node(name, point) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: os2cx_select_node() first parameter must be a ",
                "string"));
        }
        if (!__os2cx_is_vector_3(point)) {
            echo(str("ERROR: os2cx_select_node() second parameter must be a ",
                "vector of 3 numbers"));
        }
        if ($children != 0) {
            echo(str("ERROR: os2cx_select_node() shouldn't have any children"));
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_node_directive",
            name, point);
    }
}

module os2cx_create_node(name, point) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: os2cx_create_node() first parameter must be a ",
                "string"));
        }
        if (!__os2cx_is_vector_3(point)) {
            echo(str("ERROR: os2cx_create_node() second parameter must be a ",
                "vector of 3 numbers"));
        }
        if ($children != 0) {
            echo(str("ERROR: os2cx_create_node() shouldn't have any children"));
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "create_node_directive",
            name, point);
    }
}

module os2cx_load_volume(
    name, volume, force_total=undef, force_per_volume=undef
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_load_volume() first parameter must be a string");
        }
        if (!__os2cx_is_string(volume)) {
            echo(str("ERROR: os2cx_load_volume() second parameter must be a ",
                "string"));
        }
        if (force_total == undef && force_per_volume == undef) {
            echo(str("ERROR: os2cx_load_volume() requires either force_total ",
                "or force_per_volume to be specified"));
        } else if (force_total != undef && force_per_volume != undef) {
            echo(str("ERROR: os2cx_load_volume() doesn't allow both ",
                "force_total and force_per_volume to be specified ",
                "simultaneously."));
        } else if (force_total != undef) {
            if (!__os2cx_is_vector_3_with_unit(force_total)) {
                echo(str("ERROR: os2cx_load_volume() 'force_total' parameter ",
                    "must be a [vector, unit] pair."));
            }
        } else if (force_per_volume != undef) {
            if (!__os2cx_is_vector_3_with_unit(force_per_volume)) {
                echo(str("ERROR: os2cx_load_volume() 'force_per_volume' ",
                    "parameter must be a [vector, unit] pair."));
            }
        }
        if ($children != 0) {
            echo("ERROR: os2cx_load_volume() shouldn't have any children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_volume_directive",
            name, volume, force_total, force_per_volume);
    }
}

module os2cx_load_surface(
    name, surface, force_total=undef, force_per_area=undef
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: os2cx_load_surface() first parameter must be a ",
                "string"));
        }
        if (!__os2cx_is_string(surface)) {
            echo(str("ERROR: os2cx_load_surface() second parameter must be a ",
                "string"));
        }
        if (force_total == undef && force_per_area == undef) {
            echo(str("ERROR: os2cx_load_surface() requires either force_total ",
                "or force_per_area to be specified"));
        } else if (force_total != undef && force_per_area != undef) {
            echo(str("ERROR: os2cx_load_surface() doesn't allow both ",
                "force_total and force_per_area to be specified ",
                "simultaneously."));
        } else if (force_total != undef) {
            if (!__os2cx_is_vector_3_with_unit(force_total)) {
                echo(str("ERROR: os2cx_load_surface() 'force_total' parameter ",
                    "must be a [vector, unit] pair."));
            }
        } else if (force_per_area != undef) {
            if (!__os2cx_is_vector_3_with_unit(force_per_area)) {
                echo(str("ERROR: os2cx_load_surface() 'force_per_area' ",
                    "parameter must be a [vector, unit] pair."));
            }
        }
        if ($children != 0) {
            echo("ERROR: os2cx_load_surface() shouldn't have any children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_surface_directive",
            name, surface, force_total, force_per_area);
    }
}

module os2cx_material_elastic_simple(
    name, youngs_modulus=undef, poissons_ratio=undef, density=undef
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: os2cx_material_elastic_simple() first parameter ",
                "must be a string"));
        }
        if (!__os2cx_is_number_with_unit(youngs_modulus)) {
            echo(str("ERROR: os2cx_material_elastic_simple() 'youngs_modulus' ",
                "parameter must be a [number, unit] pair."));
        }
        if (!__os2cx_is_number(poissons_ratio)) {
            echo(str("ERROR: os2cx_material_elastic_simple() 'poissons_ratio' ",
                "parameter must be a number."));
        }
        if (!__os2cx_is_number_with_unit(density)) {
            echo(str("ERROR: os2cx_material_elastic_simple() 'density' ",
                "parameter must be a [number, unit] pair."));
        }
        if ($children != 0) {
            echo(str("ERROR: os2cx_material_elastic_simple() shouldn't have ",
                "any children"));
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "material_elastic_simple_directive",
            name, youngs_modulus, poissons_ratio, density);
    }
}

module os2cx_measure(
    name, volume, variable
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo(str("ERROR: os2cx_measure() first parameter must be a ",
                "string"));
        }
        if (!__os2cx_is_string(volume)) {
            echo(str("ERROR: os2cx_measure() second parameter must be a ",
                "string"));
        }
        if (!__os2cx_is_string(variable)) {
            echo(str("ERROR: os2cx_measure() third parameter must be a ",
                "string"));
        }
        if ($children != 0) {
            echo(str("ERROR: os2cx_measure() shouldn't have any children"));
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "measure_directive",
            name, volume, variable);
    }
}
