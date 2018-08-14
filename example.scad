include <./openscad2calculix.scad>;

module cantilever() {
    cube([10, 2, 2], center=true);
}

os2cx_mesh("cantilever") cantilever();
os2cx_select_surface("boundary", [-1, 0, 0], 45) {
    translate([-5, 0, 0]) cube([4, 3, 3], center=true);
}
os2cx_select_volume("load_volume") {
    translate([5, 0, 0]) cube([4, 3, 3], center=true);
}
os2cx_load_volume("load", "load_volume", 1000);

os2cx_analysis_custom([
    "*INCLUDE, INPUT=cantilever.msh",
    "*INCLUDE, INPUT=boundary.nam",
    "*MATERIAL, Name=steel",
    "*ELASTIC",
    "209000000000, 0.3",
    "*SOLID SECTION, Elset=Ecantilever, Material=steel",
    "*STEP",
    "*STATIC",
    "*BOUNDARY",
    "Nboundary,1,3",
    "*CLOAD",
    "*INCLUDE, INPUT=load.clo",
    "*NODE FILE, Nset=Ncantilever",
    "U",
    "*EL FILE, Elset=Ecantilever",
    "S",
    "*END STEP"
], units=["m", "kg", "s"]);
