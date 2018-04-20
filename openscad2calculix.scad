/* When the openscad2calculix executable is running OpenSCAD, it will redefine
this to a different value. "preview" is only used in the interactive OpenSCAD
editor. */
__openscad2calculix_mode = ["preview"];

function __os2cx_is_string(arg) = (str(arg) == arg);
function __os2cx_is_number(arg) = (arg != undef) && (arg + 0 == arg);

module os2cx(directive) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(directive)) {
            echo("ERROR: os2cx() parameter must be a string");
        }
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "directive", directive);
    }
}

module os2cx_element(name) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_element() parameter must be a string");
        }
        children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "element_directive", name);
    } else if (__openscad2calculix_mode == ["element", name]) {
        children();
    }
}

module os2cx_nset(name) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_nset() parameter must be a string");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "nset_directive", name);
    } else if (__openscad2calculix_mode == ["nset", name]) {
        children();
    }
}

module os2cx_volume_load(name, magnitude) {
    if (__openscad2calculix_mode == ["preview"]) {
        if (!__os2cx_is_string(name)) {
            echo("ERROR: os2cx_volume_load() first parameter must be a string");
        }
        if (!__os2cx_is_number(magnitude)) {
            echo("ERROR: os2cx_volume_load() second parameter must be a " +
                "number");
        }
        # children();
    } else if (__openscad2calculix_mode == ["inventory"]) {
        echo("__openscad2calculix", "volume_load_directive", name, magnitude);
    } else if (__openscad2calculix_mode == ["volume_load", name]) {
        children();
    }
}
