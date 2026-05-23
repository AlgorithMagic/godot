//
// Created by busin on 2026-05-23.
//

#pragma once

#include "modules/TWIM/terrain/tile_id.h"

#include <cstdint>

namespace twim {

struct tile_cell {
	tile_type_id type_id;
	uint16_t flags = 0;

	constexpr tile_cell() = default;

	constexpr explicit tile_cell(const tile_type_id p_type_id) :
			type_id(p_type_id) {
	}

	constexpr tile_cell(const tile_type_id p_type_id, const uint16_t p_flags) :
			type_id(p_type_id),
			flags(p_flags) {
	}
};

} // namespace twim
