//
// Created by busin on 2026-05-22.
//

#pragma once

#include <cstdint>

namespace twim {

struct tile_type_id {
	uint16_t value = 0;

	constexpr tile_type_id() = default;

	constexpr explicit tile_type_id(uint16_t p_value) :
			value(p_value) {
	}

	constexpr bool operator==(const tile_type_id &p_other) const {
		return value == p_other.value;
	}

	constexpr bool operator!=(const tile_type_id &p_other) const {
		return value != p_other.value;
	}
};

} // namespace twim
