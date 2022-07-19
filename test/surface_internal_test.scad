include <../openscad2calculix.scad>

length = 1;
width = 0.1;
height = 0.1;
epsilon = 0.01;

os2cx_mesh("beam") {
  cube([length, width, height], center=true);
}

os2cx_select_surface("anchored_end", [-1, 0, 0], 45) {
    translate([-length/2, 0, 0])
        cube([epsilon, width+epsilon, height+epsilon], center=true);
}

os2cx_select_surface("loaded_end", [1, 0, 0], 45) {
    translate([length/2, 0, 0])
        cube([epsilon, width+epsilon, height+epsilon], center=true);
}

os2cx_select_surface_internal("halfway_section", [-1, 0, 0], 45) {
    translate([epsilon/2, 0, 0])
        cube([epsilon, width+epsilon, height+epsilon], center=true);
}

os2cx_load_surface("load", "loaded_end", force_total=[[0, 0, -1], "kN"]);

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7600, "kg/m^3"]);

os2cx_analysis_custom([
    "*INCLUDE, INPUT=objects.inp",
    ["*SOLID SECTION, Elset=", ["elset", "beam"], ", Material=", "steel"],
    "*STEP",
    "*STATIC",
    "*BOUNDARY",
    [["nset", "anchored_end"], ",1,3"],
    "*CLOAD",
    ["*INCLUDE, INPUT=", ["cload_file", "load"]],
    "*NODE FILE",
    "U",
    "*EL FILE",
    "S",
    ["*SECTION PRINT,SURFACE=", ["surface", "halfway_section"], ",NAME=SP1"],
    "SOF",
    "*END STEP"
], unit_system=["m", "kg", "s"]);
