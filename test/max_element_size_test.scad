include <../openscad2calculix.scad>

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7.87, "g/cm^3"]);

os2cx_mesh("test_a", material="steel") {
    translate([-2, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_b", max_element_size=0.1, material="steel") {
    translate([0, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_c", material="steel") {
    translate([2, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}
os2cx_select_volume("test_c_subset") {
    translate([2, 2, 0]) cube([1.1, 2.0, 1.1], center=true);
}
os2cx_override_max_element_size("test_c_subset", 0.1);

os2cx_select_surface("fixed", [0, -1, 0], 45) {
    translate([0, -2, 0]) cube([7, 1, 2], center=true);
}

os2cx_select_surface("loaded", [0, 1, 0], 45) {
    translate([0, 2, 0]) cube([7, 1, 2], center=true);
}
os2cx_load_surface("load", "loaded", force_per_area=[[0, 0, -1], "N/mm^2"]);

os2cx_analysis_static_simple(
    fixed="fixed",
    load="load",
    length_unit="mm");
