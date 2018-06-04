#ifndef OS2CX_POLY_MAP_INTERNAL_BORDER_TRAVERSAL_HPP_
#define OS2CX_POLY_MAP_INTERNAL_BORDER_TRAVERSAL_HPP_

#include "poly_map.internal.hpp"

namespace os2cx {

/* The poly3_map_internal_traverse_*() functions are helpers for
poly3_map_create(). They consume 'poly3_map->i->nef' and produce the publicly
visible fields of Poly3Map. They also consume each others' output, so they must
be called in the following order: vertices, volumes, borders, surfaces. */

/* Populate 'vertices', 'vertex_from_nef', and 'vertex_to_nef' */
void poly3_map_internal_traverse_vertices(Poly3Map *poly3_map);

/* Populate 'volumes', 'volume_outside', 'volume_from_nef' and 'volume_to_nef'
*/
void poly3_map_internal_traverse_volumes(Poly3Map *poly3_map);

/* Populate 'borders' and 'borders_by_vertex', but don't fill
'Border::surfaces' yet. */
void poly3_map_internal_traverse_borders(Poly3Map *poly3_map);

/* Populate 'surfaces' and fill 'Border::surfaces' */
void poly3_map_internal_traverse_surfaces(Poly3Map *poly3_map);

} /* namespace os2cx */

#endif /* OS2CX_POLY_MAP_INTERNAL_BORDER_TRAVERSAL_HPP_ */
