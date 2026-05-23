//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/math/vector2i.h"
#include "core/math/vector3i.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

namespace twim {

struct RenderSnapshot {
	bool available = false;
	String reason;
	Vector3i origin;
	Vector2i size;
	int32_t z_layer = 0;
	uint64_t tick_index = 0;
	String glyph_font_resource_path;
	int32_t glyph_font_size = 16;
	Array cells;

	Dictionary to_dictionary() const {
		Dictionary result;
		result["available"] = available;
		if (!reason.is_empty()) {
			result["reason"] = reason;
		}
		result["origin"] = origin;
		result["size"] = size;
		result["z_layer"] = z_layer;
		result["tick_index"] = static_cast<int64_t>(tick_index);
		result["glyph_font_resource_path"] = glyph_font_resource_path;
		result["glyph_font_size"] = glyph_font_size;
		result["cells"] = cells;
		return result;
	}
};

} // namespace twim
