#pragma once

#include "core/error/error_list.h"

#include "modules/TWIM/terrain/chunk.h"
#include "modules/TWIM/terrain/tile_registry.h"

#include <cstdint>

namespace twim {

class TwimPhysicsEngine {
public:
	Error simulate_chunk(chunk &p_chunk, const TileDefinitionRegistry &p_tile_definitions, uint32_t p_tick_count) const;
};

} // namespace twim

