include <./openscad2calculix.scad>;

module cantilever() {
    cube([10, 2, 2], center=true);
}

os2cx_element("cantilever") cantilever();
os2cx_nset("fixed") translate([-5,0,0]) cube([4,3,3],center=true);
os2cx_volume_load("load", 1e9) translate([5,0,0]) cube([4,3,3],center=true);

os2cx("*MATERIAL, Name=steel");
os2cx("*ELASTIC");
os2cx("209000000000, 0.3");
os2cx("*SOLID SECTION, Elset=Ecantilever, Material=steel");
os2cx("*STEP");
os2cx("*STATIC");
os2cx("*BOUNDARY");
os2cx("Nfixed,1,3");
os2cx("*CLOAD");
os2cx("*INCLUDE, INPUT=load.clo");
os2cx("*NODE FILE, Nset=Ncantilever");
os2cx("U");
os2cx("*END STEP");
