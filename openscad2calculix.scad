/* When the openscad2calculix executable is running OpenSCAD, it will redefine
this to a different value. "preview" is only used in the interactive OpenSCAD
editor. */
__openscad2calculix_mode = ["preview"];

function __os2cx_is_vector_3(arg) =
    is_list(arg)
    && len(arg) == 3
    && is_num(arg[0])
    && is_num(arg[1])
    && is_num(arg[2]);
function __os2cx_is_num_with_unit(arg) =
    is_list(arg)
    && len(arg) == 2
    && is_num(arg[0])
    && is_string(arg[1]);
function __os2cx_is_vector_3_with_unit(arg) =
    is_list(arg)
    && len(arg) == 2
    && __os2cx_is_vector_3(arg[0])
    && is_string(arg[1]);
function __os2cx_is_list_of(arg, pred) =
    is_list(arg)
    && [] == [for (a = arg) if (!pred(a)) a];

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

module os2cx_analysis_custom(lines, unit_system=undef) {
    assert(
        __os2cx_is_list_of(lines, function(l) (
            is_string(l)
            || __os2cx_is_list_of(l, function (l2) (
                is_string(l2)
                || is_list(l2)
            ))
        )),
        "malformed input lines");
    assert(
        is_list(unit_system)
        && len(unit_system) == 3
        && is_string(unit_system[0])
        && is_string(unit_system[1])
        && is_string(unit_system[2]),
        "unit_system must be a list of three strings");
    assert($children == 0);

    if (__openscad2calculix_mode == ["preview"]) {
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
    fixed=undef,
    load=undef,
    length_unit=undef
) {
    assert(is_string(fixed));
    assert(is_string(load));
    assert(is_string(length_unit));
    assert($children == 0);

    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        "*STEP",
        "*STATIC",
        "*BOUNDARY",
        [["nset", fixed], ",1,3"],
        "*CLOAD",
        ["*INCLUDE, INPUT=", ["cload_file", load]],
        "*NODE FILE",
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
    fixed=undef,
    load=undef,
    length_unit=undef,
    num_eigenfrequencies=10,
    min_frequency=undef,
    max_frequency=undef,
    damping_ratio=undef
) {
    assert(is_string(fixed));
    assert(is_string(load));
    assert(is_string(length_unit));
    assert(is_num(num_eigenfrequencies));
    assert(__os2cx_is_num_with_unit(min_frequency) && min_frequency[1] == "Hz",
        "min_frequency must be [a number, \"Hz\"]");
    assert(__os2cx_is_num_with_unit(max_frequency) && max_frequency[1] == "Hz",
        "max_frequency must be [a number, \"Hz\"]");
    assert(min_frequency[0] < max_frequency[0]);
    assert(is_num(damping_ratio));
    assert($children == 0);

    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        "*BOUNDARY",
        [["nset", fixed], ",1,3"],
        "*STEP",
        "*FREQUENCY,STORAGE=yes",
        str(num_eigenfrequencies),
        "*NODE FILE",
        "U",
        "*END STEP",
        "*STEP",
        "*MODAL DAMPING",
        str("1", ",", num_eigenfrequencies, ",", damping_ratio),
        "*STEADY STATE DYNAMICS",
        str(min_frequency[0], ",", max_frequency[0], ",", 10),
        "*CLOAD",
        ["*INCLUDE, INPUT=", ["cload_file", load]],
        "*NODE FILE",
        "PU,U",
        "*EL FILE",
        "S",
        "*END STEP"
    ], unit_system=[length_unit, "kg", "s"]);
}

/* os2cx_analysis_steady_state_dynamics_osc_boundary() simulates a structure
attached to an oscillating base. For example, a tower during an earthquake. */

module os2cx_analysis_steady_state_dynamics_osc_boundary(
    boundary=undef,
    oscillation=undef,
    length_unit=undef,
    num_eigenfrequencies=10,
    min_frequency=undef,
    max_frequency=undef,
    rayleigh_damping_alpha=undef,
    rayleigh_damping_beta=undef,
) {
    assert(is_string(boundary));
    assert(__os2cx_is_vector_3_with_unit(oscillation));
    assert(is_string(length_unit));
    assert(oscillation[1] == length_unit,
        "oscillation units must be same as length_unit");
    assert(is_num(num_eigenfrequencies));
    assert(__os2cx_is_num_with_unit(min_frequency) && min_frequency[1] == "Hz",
        "min_frequency must be [a number, \"Hz\"]");
    assert(__os2cx_is_num_with_unit(max_frequency) && max_frequency[1] == "Hz",
        "max_frequency must be [a number, \"Hz\"]");
    assert(is_num(rayleigh_damping_alpha));
    assert(is_num(rayleigh_damping_beta));
    assert($children == 0);

    os2cx_analysis_custom([
        "*INCLUDE, INPUT=objects.inp",
        "*BOUNDARY",
        [["nset", boundary], ",1,3"],
        "*STEP",
        "*FREQUENCY,STORAGE=yes",
        str(num_eigenfrequencies),
        "*NODE FILE",
        "U",
        "*END STEP",
        "*STEP",
        "*MODAL DAMPING,RAYLEIGH",
        str(",,", rayleigh_damping_alpha, ",", rayleigh_damping_beta),
        "*STEADY STATE DYNAMICS",
        str(min_frequency[0], ",", max_frequency[0], ",", 10),
        "*BOUNDARY",
        [["nset", boundary], ",1,1,", str(oscillation[0][0])],
        [["nset", boundary], ",2,2,", str(oscillation[0][1])],
        [["nset", boundary], ",3,3,", str(oscillation[0][2])],
        "*NODE FILE",
        "PU,U",
        "*EL FILE",
        "S",
        "*END STEP"
    ], unit_system=[length_unit, "kg", "s"]);
}

module os2cx_mesh(
    name,
    mesher="tetgen",
    max_element_size=undef,
    material=undef
) {
    assert(is_string(name));
    assert(is_string(mesher));
    assert(is_undef(max_element_size) ||
        (is_num(max_element_size) && max_element_size > 0));
    assert(is_string(material));
    assert($children > 0);

    if (__openscad2calculix_mode == ["preview"]) {
        children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "mesh_directive",
            name, mesher, max_element_size, material);
    } else if (__openscad2calculix_mode == ["mesh", name]) {
        children();
    }
}

module os2cx_slice(name, direction_vector, direction_angle_tolerance) {
    assert(is_string(name));
    assert(__os2cx_is_vector_3(direction_vector));
    assert(is_num(direction_angle_tolerance));
    assert($children > 0);

    if (__openscad2calculix_mode == ["preview"] && $preview) {
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
    assert(is_string(name));
    assert($children > 0);

    if (__openscad2calculix_mode == ["preview"] && $preview) {
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_volume_directive", name);
    } else if (__openscad2calculix_mode == ["select_volume", name]) {
        children();
    }
}

module os2cx_select_surface(name, direction_vector, direction_angle_tolerance) {
    assert(is_string(name));
    assert(__os2cx_is_vector_3(direction_vector));
    assert(is_num(direction_angle_tolerance));
    assert($children > 0);

    if (__openscad2calculix_mode == ["preview"] && $preview) {
        if (!is_string(name)) {
            echo("ERROR: os2cx_select_surface() parameter must be a string");
        }
        if ($preview) {
            # children();
        }
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
    assert(is_string(name));
    assert(__os2cx_is_vector_3(direction_vector));
    assert(is_num(direction_angle_tolerance));
    assert(direction_angle_tolerance < 90);
    assert($children > 0);

    if (__openscad2calculix_mode == ["preview"] && $preview) {
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
    assert(is_string(name));
    assert(__os2cx_is_vector_3(point));
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "select_node_directive",
            name, point);
    }
}

module os2cx_create_node(name, point) {
    assert(is_string(name));
    assert(__os2cx_is_vector_3(point));
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "create_node_directive",
            name, point);
    }
}

module os2cx_load_volume(
    name, volume, force_total=undef, force_per_volume=undef
) {
    assert(is_string(name));
    assert(is_string(volume));
    assert(!is_undef(force_total) || !is_undef(force_per_volume),
        "must specify either force_total or force_per_volume");
    assert(is_undef(force_total) || is_undef(force_per_volume),
        "can't specify both force_total and force_per_volume");
    if (force_total != undef) {
        assert(__os2cx_is_vector_3_with_unit(force_total));
    }
    if (force_per_volume != undef) {
        assert(__os2cx_is_vector_3_with_unit(force_per_volume));
    }
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_volume_directive",
            name, volume, force_total, force_per_volume);
    }
}

module os2cx_load_surface(
    name, surface, force_total=undef, force_per_area=undef
) {
    assert(is_string(name));
    assert(is_string(surface));
    assert(!is_undef(force_total) || !is_undef(force_per_area),
        "must specify either force_total or force_per_area");
    assert(is_undef(force_total) || is_undef(force_per_area),
        "can't specify both force_total and force_per_area");
    if (force_total != undef) {
        assert(__os2cx_is_vector_3_with_unit(force_total));
    }
    if (force_per_area != undef) {
        assert(__os2cx_is_vector_3_with_unit(force_per_area));
    }
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_surface_directive",
            name, surface, force_total, force_per_area);
    }
}

module os2cx_material_elastic_simple(
    name, youngs_modulus=undef, poissons_ratio=undef, density=undef
) {
    assert(is_string(name));
    assert(__os2cx_is_num_with_unit(youngs_modulus));
    assert(is_num(poissons_ratio));
    assert(__os2cx_is_num_with_unit(density));
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "material_elastic_simple_directive",
            name, youngs_modulus, poissons_ratio, density);
    }
}

module os2cx_override_max_element_size(
    volume, max_element_size
) {
    assert(is_string(volume));
    assert(is_num(max_element_size));
    assert(max_element_size > 0);
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "override_max_element_size_directive",
            volume, max_element_size);
    }
}

module os2cx_override_material(
    volume, material
) {
    assert(is_string(volume));
    assert(is_string(material));
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "override_material_directive",
            volume, material);
    }
}

module os2cx_measure(
    name, volume, variable
) {
    assert(is_string(name));
    assert(is_string(volume));
    assert(is_string(variable));
    assert($children == 0);

    if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "measure_directive",
            name, volume, variable);
    }
}
