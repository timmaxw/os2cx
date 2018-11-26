include <./openscad2calculix.scad>;

/* Define the solid mesh that will be analyzed: a simple cantilevered beam */
os2cx_mesh("cantilever") cube([10, 2, 2], center=true);

/* Define a material that we'll apply to the mesh */
os2cx_material_elastic_simple(
    "steel", youngs_modulus=[209, "GPa"], poissons_ratio=0.3);

/* Select one end of the beam to be fixed as the boundary condition */
os2cx_select_surface("fixed_end", [-1, 0, 0], 45) {
    translate([-5, 0, 0]) cube([4, 3, 3], center=true);
}

/* Select the other end of the beam to be loaded, and define the load to be
applied to it */
os2cx_select_surface("free_end", [1, 0, 0], 45) {
    translate([5, 0, 0]) cube([4, 3, 3], center=true);
}
os2cx_load_surface("free_end_load", "free_end", [[0, 0, -1000], "N/m^2"]);

/* Put it all together */
os2cx_analysis_static_simple(
    mesh="cantilever",
    material="steel",
    fixed="fixed_end",
    load="free_end_load"
);
