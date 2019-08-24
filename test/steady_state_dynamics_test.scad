include <../openscad2calculix.scad>

os2cx_mesh("bar", mesher="naive_bricks") {
    cube([10.0, 1.0, 1.0], center=true);
}

os2cx_select_surface("anchored_end", [-1, 0, 0], 45) {
    translate([-5.0, 0.0, 0.0])
        cube([0.1, 1.1, 1.1], center=true);
}

os2cx_select_volume("loaded_part") {
    cube([1.0, 1.1, 1.1], center=true);
}

os2cx_load_volume(
    "load",
    "loaded_part",
    force_total=[[0, 0, 100], "kN"]);

os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7600, "kg/m^3"]);

os2cx_analysis_steady_state_dynamics(
    mesh="bar",
    material="steel",
    fixed="anchored_end",
    load="load",
    length_unit="m",
    min_frequency=[1, "Hz"],
    max_frequency=[100, "Hz"]
);
