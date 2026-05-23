//
// Created by Kyle Maillet on 2026-05-22.
//

#pragma once

#include "core/error/error_list.h"
#include "core/string/ustring.h"

#include "modules/TWIM/terrain/chunk_coord.h"

#include <cstdint>

namespace twim {

struct WorldConfig {
	static constexpr uint32_t DEFAULT_TICK_RATE_HZ = 120;
	static constexpr uint32_t MAX_TICKS_PER_STEP = 100000;

	String world_name;
	uint64_t seed = 0;
	uint32_t tick_rate_hz = DEFAULT_TICK_RATE_HZ;
	chunk_metrics metrics;

	String tile_set_resource_path;
	String entity_catalog_resource_path;
	String item_catalog_resource_path;
	String world_profile_resource_path;
	String glyph_font_resource_path;
	int32_t glyph_font_size = 16;

	bool deterministic = true;

	WorldConfig() = default;

	WorldConfig(const String &p_world_name, uint64_t p_seed) :
			world_name(p_world_name),
			seed(p_seed) {
	}

	Error validate() const {
		if (world_name.is_empty()) {
			return ERR_INVALID_PARAMETER;
		}

		if (tick_rate_hz == 0) {
			return ERR_INVALID_PARAMETER;
		}

		if (!metrics.is_valid()) {
			return ERR_INVALID_PARAMETER;
		}

		if (glyph_font_size <= 0) {
			return ERR_INVALID_PARAMETER;
		}

		return OK;
	}

	static WorldConfig from_project_settings(const String &p_world_name, uint64_t p_seed);
};

} // namespace twim
