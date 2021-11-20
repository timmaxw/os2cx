length = 1;
width = 0.1;
height = 0.1;
thickness = 0.01;

module i_beam() {
    cube([length, thickness, height], center=true);
    translate([0, 0, height/2-thickness/2])
        cube([length, width, thickness], center=true);
    translate([0, 0, -height/2+thickness/2])
        cube([length, width, thickness], center=true);
}

include <../openscad2calculix.scad>

/* Declares a new mesh object in OS2CX. Its name will be "i_beam", and its shape
comes from the the OpenSCAD i_beam() module we defined above. */
os2cx_mesh("i_beam") {
  i_beam();
}

/* Declares two surface selection objects in OS2CX. A surface selection object
refers to part of the surface of a mesh object. It's defined by taking the
intersection of the declared mesh, with another OpenSCAD shape. */
os2cx_select_surface("anchored_end", [-1, 0, 0], 45) {
    translate([-length/2, 0, 0])
        cube([0.1, width+0.1, height+0.1], center=true);
}
os2cx_select_volume("loaded_end") {
    translate([length/2, 0, 0])
        cube([0.1, width+0.1, height+0.1], center=true);
}

os2cx_select_node("some_point", [length/2-0.05, 0, 0]);

/* Declares a load object in OS2CX, called "car_weight". It's defined as a
force of 9,800 newtons in the -Z direction, applied uniformly over the
"loaded_end" surface we defined above. */
weight = 1000;
gravity = 9.8;
os2cx_load_volume(
    "car_weight",
    "loaded_end",
    force_total=[[0, 0, -gravity*weight], "N"]);

/* Declares a material object in OS2CX, called "steel". It has a Young's modulus
of 209 gigapascals, and a Poisson's ratio of 0.3. */
os2cx_material_elastic_simple(
    "steel",
    youngs_modulus=[209, "GPa"],
    poissons_ratio=0.3,
    density=[7600, "kg/m^3"]);

/* Tell OS2CX to do a simple static deflection analysis using the objects we
just defined */
os2cx_analysis_static_simple(
    mesh="i_beam",
    material="steel",
    fixed="anchored_end",
    load="car_weight",
    length_unit="m"
);

/* Tell OS2CX to report the maximum deflection of the loaded end, and the
maximum von Mises stress anywhere in the beam. */
os2cx_measure(
    "loaded_end_deflection",
    "loaded_end",
    "DISP");
os2cx_measure(
    "anywhere_von_mises_stress",
    "i_beam",
    "STRESS");
