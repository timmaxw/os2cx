include <../openscad2calculix.scad>

os2cx_mesh("test_a") {
    translate([-2, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_b", max_element_size = 0.1) {
    translate([0, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}

os2cx_mesh("test_c") {
    translate([2, 0, 0]) cube([1.0, 4.0, 1.0], center=true);
}
os2cx_select_volume("test_c_subset") {
    translate([2, 2, 0]) cube([1.1, 2.0, 1.1], center=true);
}
os2cx_override_max_element_size("test_c_subset", 0.1);

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7.87, "g/cm^3"]);

os2cx_analysis_custom([
    "*INCLUDE, INPUT=objects.inp",
    ["*SOLID SECTION, Elset=", ["elset", "test_a"], ", Material=steel"],
    ["*SOLID SECTION, Elset=", ["elset", "test_b"], ", Material=steel"],
    "*STEP",
    "*NO ANALYSIS",
    "*END STEP",
], unit_system=["mm", "kg", "s"]);
