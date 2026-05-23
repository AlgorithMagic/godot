#pragma once

#include "modules/TWIM/rendering/render_snapshot.h"
#include "modules/TWIM/terrain/chunk_store.h"
#include "modules/TWIM/terrain/tile_registry.h"

namespace twim {

class RenderSnapshotBuilder {
public:
	RenderSnapshot build(
			const chunk_store &p_chunks,
			const TileDefinitionRegistry &p_tile_definitions,
			Vector3i p_origin,
			Vector2i p_size,
			int32_t p_z_layer,
			uint64_t p_tick_index,
			const String &p_glyph_font_resource_path,
			int32_t p_glyph_font_size) const;
};

} // namespace twim

