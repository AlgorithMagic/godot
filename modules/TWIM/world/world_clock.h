//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/error/error_list.h"

#include "world_config.h"

#include <cstdint>
#include <limits>

namespace twim {

class WorldClock {
public:
	WorldClock() = default;

	Error reset(uint32_t p_tick_rate_hz) {
		if (p_tick_rate_hz == 0) {
			return ERR_INVALID_PARAMETER;
		}

		tick_index = 0;
		tick_rate_hz = p_tick_rate_hz;
		return OK;
	}

	Error step_ticks(uint32_t p_tick_count) {
		if (p_tick_count == 0) {
			return OK;
		}

		if (tick_index > std::numeric_limits<uint64_t>::max() - p_tick_count) {
			return ERR_OUT_OF_MEMORY;
		}

		tick_index += p_tick_count;
		return OK;
	}

	uint64_t get_tick_index() const {
		return tick_index;
	}

	uint32_t get_tick_rate_hz() const {
		return tick_rate_hz;
	}

private:
	uint64_t tick_index = 0;
	uint32_t tick_rate_hz = WorldConfig::DEFAULT_TICK_RATE_HZ;
};

} // namespace twim
