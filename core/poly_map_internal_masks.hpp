#ifndef OS2CX_POLY_MAP_INTERNAL_MASKS_HPP_
#define OS2CX_POLY_MAP_INTERNAL_MASKS_HPP_

#include "poly_map.internal.hpp"

namespace os2cx {

void poly3_map_internal_compute_volume_mask_volumes(
    const Poly3Map &poly3_map,
    const std::vector<os2cx::CgalNef3> &volume_mask_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &volume_mask_volumes_out);

} /* namespace os2cx */

#endif /* OS2CX_POLY_MAP_INTERNAL_MASKS_HPP_ */
