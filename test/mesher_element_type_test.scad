include <../openscad2calculix.scad>

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7.87, "g/cm^3"]);

* os2cx_mesh("test_a", material="steel", max_element_size=0.3,
        mesher="tetgen", element_type="C3D4") {
    translate([-5, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_b", material="steel", max_element_size=0.3,
        mesher="tetgen", element_type="C3D10") {
    translate([-3, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

* os2cx_mesh("test_c", material="steel", max_element_size=0.3,
        mesher="naive_bricks", element_type="C3D8") {
    translate([-1, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

* os2cx_mesh("test_d", material="steel", max_element_size=0.3,
        mesher="naive_bricks", element_type="C3D20") {
    translate([1, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_e", material="steel", max_element_size=0.3,
        mesher="naive_bricks", element_type="C3D20R") {
    translate([3, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

* os2cx_mesh("test_f", material="steel", max_element_size=0.3,
        mesher="naive_bricks", element_type="C3D20RI") {
    translate([5, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_select_surface("fixed", [0, -1, 0], 45) {
    translate([0, -2, 0]) cube([13, 1, 2], center=true);
}

os2cx_select_surface("loaded", [0, 1, 0], 45) {
    translate([0, 2, 0]) cube([13, 1, 2], center=true);
}
os2cx_load_surface("load", "loaded", force_total=[[0, 0, -2], "N"]);

os2cx_analysis_static_simple(
    fixed="fixed",
    load="load",
    length_unit="mm");
