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
            echo("ERROR: ", referrer, "refers to a", object_type, "called",
                name, "but that's not a valid name.",
                "(It needs to be a string.)");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "check_existing_directive",
            object_type, referrer, name);
    }
}

module os2cx_analysis_custom(lines, unit_system=undef) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_array_strings(lines)) {
            echo("ERROR: os2cx_analysis_custom() parameter must be an ",
                "array of strings");
        }
        if (!__os2cx_is_array_strings(unit_system) || len(unit_system) != 3) {
            echo("ERROR: os2cx_analysis_custom() 'unit_system' parameter ",
                "must be an array of three strings.");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "analysis_directive", lines, unit_system);
    }
}

module os2cx_analysis_static_simple(
    mesh=undef,
    material=undef,
    fixed=undef,
    load=undef
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
    ], unit_system=["m", "kg", "s"]);
}

module os2cx_mesh(name, max_tet_volume=undef) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_mesh() name must be a string");
        }
        if (max_tet_volume != undef && !__os2cx_is_number(max_tet_volume)) {
            echo("ERROR: os2cx_mesh() max_tet_volume must be a number");
        }
        children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "mesh_directive", name, max_tet_volume);
    } else if (__openscad2calculix_mode == ["mesh", name]) {
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
            direction_vector,
            direction_angle_tolerance);
    } else if (__openscad2calculix_mode == ["select_surface", name]) {
        children();
    }
}

module os2cx_load_volume(name, volume, force_per_volume) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_load_volume() first parameter must be a string");
        }
        if (!__os2cx_is_string(volume)) {
            echo("ERROR: os2cx_load_volume() second parameter must be a string");
        }
        if (!__os2cx_is_vector_3_with_unit(force_per_volume)) {
            echo("ERROR: os2cx_load_volume() third parameter must be a ",
                "[vector, unit] pair.");
        }
        if ($children != 0) {
            echo("ERROR: os2cx_load_volume() shouldn't have any children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_volume_directive",
            name, volume, force_per_volume);
    }
}

module os2cx_load_surface(name, surface, force_per_area) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_load_surface() first parameter must be a ",
                "string");
        }
        if (!__os2cx_is_string(surface)) {
            echo("ERROR: os2cx_load_surface() second parameter must be a ",
                "string");
        }
        if (!__os2cx_is_vector_3_with_unit(force_per_area)) {
            echo("ERROR: os2cx_load_surface() third parameter must be a ",
                "[vector, unit] pair.");
        }
        if ($children != 0) {
            echo("ERROR: os2cx_load_surface() shouldn't have any children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_surface_directive",
            name, surface, force_per_area);
    }
}

module os2cx_material_elastic_simple(
    name, youngs_modulus=undef, poissons_ratio=undef
) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_material_elastic_simple() first parameter ",
                "must be a string");
        }
        if (!__os2cx_is_number_with_unit(youngs_modulus)) {
            echo("ERROR: os2cx_material_elastic_simple() 'youngs_modulus' ",
                "parameter must be a [number, unit] pair.");
        }
        if (!__os2cx_is_number(poissons_ratio)) {
            echo("ERROR: os2cx_material_elastic_simple() 'poissons_ratio' ",
                "parameter must be a number.");
        }
        if ($children != 0) {
            echo("ERROR: os2cx_material_elastic_simple() shouldn't have any ",
                "children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "material_elastic_simple_directive",
            name, youngs_modulus, poissons_ratio);
    }
}
