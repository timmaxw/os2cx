/* When the openscad2calculix executable is running OpenSCAD, it will redefine
this to a different value. "preview" is only used in the interactive OpenSCAD
editor. */
__openscad2calculix_mode = ["preview"];

function __os2cx_is_string(arg) = (str(arg) == arg);
function __os2cx_is_number(arg) = (arg != undef) && (arg + 0 == arg);
function __os2cx_is_array_strings(arg) =
    all([for (item = arg) __os2cx_is_string(item)]);

module os2cx_analysis_custom(lines) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_array_strings(lines)) {
            echo("ERROR: os2cx_analysis_custom() parameter must be an " +
                "array of strings");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "analysis_directive", lines);
    }
}

module os2cx_mesh(name) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_mesh() parameter must be a string");
        }
        children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "mesh_directive", name);
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

module os2cx_load_volume(name, volume, magnitude) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_load_volume() first parameter must be a string");
        }
        if (!__os2cx_is_string(volume)) {
            echo("ERROR: os2cx_load_volume() second parameter must be a string");
        }
        if (!__os2cx_is_number(magnitude)) {
            echo("ERROR: os2cx_load_volume() third parameter must be a " +
                "number");
        }
        if (num_children() != 0) {
            echo("ERROR: os2cx_load_volume() shouldn't have any children");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "load_volume_directive",
            name, volume, magnitude);
    }
}
