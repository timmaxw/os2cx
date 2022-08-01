include <../openscad2calculix.scad>

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7.87, "g/cm^3"]);

os2cx_material_elastic_simple(
    "rubber",
    youngs_modulus=[0.1, "GPa"],
    poissons_ratio=0.49,
    density=[1.10, "g/cm^3"]);

os2cx_mesh("beam", material="steel") {
    cube([1, 0.1, 0.1], center=true);
}

os2cx_select_volume("rubber_part") {
    cube([0.1, 0.2, 0.2], center=true);
}
os2cx_override_material("rubber_part", material="rubber");

os2cx_select_surface("anchored_end", [-1, 0, 0], 45) {
    translate([-0.5, 0, 0]) cube([0.1, 0.2, 0.2], center=true);
}
os2cx_select_surface("loaded_end", [1, 0, 0], 45) {
    translate([0.5, 0, 0]) cube([0.1, 0.2, 0.2], center=true);
}

os2cx_load_surface(
    "load",
    "loaded_end",
    force_total=[[0, 0, -1], "kN"]);

os2cx_analysis_static_simple(
    fixed="anchored_end",
    load="load",
    length_unit="m"
);
