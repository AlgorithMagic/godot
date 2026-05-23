//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/string/string_name.h"

#include "modules/TWIM/terrain/tile_id.h"

#include <cstdint>

namespace twim {

struct TileDefinition {
	tile_type_id id;
	StringName name;

	bool solid = false;
	bool opaque = false;
	bool liquid = false;

	uint32_t glyph_id = 0;
	uint32_t foreground_rgba = 0xffffffff;
	uint32_t background_rgba = 0x000000ff;

	TileDefinition() = default;

	TileDefinition(
			tile_type_id p_id,
			const StringName &p_name,
			bool p_solid,
			bool p_opaque,
			bool p_liquid,
			uint32_t p_glyph_id,
			uint32_t p_foreground_rgba,
			uint32_t p_background_rgba) :
			id(p_id),
			name(p_name),
			solid(p_solid),
			opaque(p_opaque),
			liquid(p_liquid),
			glyph_id(p_glyph_id),
			foreground_rgba(p_foreground_rgba),
			background_rgba(p_background_rgba) {
	}
};

} // namespace twim
