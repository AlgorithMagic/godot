//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/math/vector3i.h"
#include "core/string/string_name.h"

#include <cstdint>

namespace twim {

struct VisualCell {
	int32_t x = 0;
	int32_t y = 0;
	Vector3i world_coord;
	uint16_t tile_type_id = 0;
	StringName tile_name;
	uint32_t glyph_id = 0;
	uint32_t foreground_rgba = 0;
	uint32_t background_rgba = 0;
	bool solid = false;
	bool opaque = false;
	bool liquid = false;
};

} // namespace twim
