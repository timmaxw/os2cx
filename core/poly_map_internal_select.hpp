#ifndef OS2CX_POLY_MAP_INTERNAL_SELECT_HPP_
#define OS2CX_POLY_MAP_INTERNAL_SELECT_HPP_

#include "poly_map.internal.hpp"

namespace os2cx {

void poly3_map_internal_compute_cut_for_select_surface(
    const os2cx::CgalNef3 &solid_nef,
    const Poly3MapSelectSurface &select_surface,
    const os2cx::CgalNef3 &select_surface_nef,
    CGAL::Nef_nary_union_3<os2cx::CgalNef3> *cut_union_out);

void poly3_map_internal_compute_selected_volumes(
    const Poly3Map &poly3_map,
    const std::vector<os2cx::CgalNef3> &select_volume_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &selected_volumes_out);

void poly3_map_internal_compute_selected_surfaces(
    const Poly3Map &poly3_map,
    const std::vector<Poly3MapSelectSurface> &select_surfaces,
    const std::vector<os2cx::CgalNef3> &select_surface_nefs,
    const std::vector<std::set<Poly3Map::SurfaceId> *>
        &selected_surfaces_out);

} /* namespace os2cx */

#endif /* OS2CX_POLY_MAP_INTERNAL_SELECT_HPP_ */
