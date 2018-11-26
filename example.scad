include <./openscad2calculix.scad>;

module cantilever() {
    cube([10, 2, 2], center=true);
}

os2cx_mesh("cantilever") cantilever();
os2cx_select_surface("fixed_end", [-1, 0, 0], 45) {
    translate([-5, 0, 0]) cube([4, 3, 3], center=true);
}
os2cx_select_surface("free_end", [1, 0, 0], 45) {
    translate([5, 0, 0]) cube([4, 3, 3], center=true);
}
os2cx_load_surface("load", "free_end", [[0, 0, -1000], "N/m^2"]);
os2cx_material_elastic_simple(
    "steel", youngs_modulus=[209, "GPa"], poissons_ratio=0.3);

os2cx_analysis_custom([
    "*INCLUDE, INPUT=objects.inp",
    "*SOLID SECTION, Elset=Ecantilever, Material=steel",
    "*STEP",
    "*STATIC",
    "*BOUNDARY",
    "Nfixed_end,1,3",
    "*CLOAD",
    "*INCLUDE, INPUT=load.clo",
    "*NODE FILE, Nset=Ncantilever",
    "U",
    "*EL FILE, Elset=Ecantilever",
    "S",
    "*END STEP"
], unit_system=["m", "kg", "s"]);
