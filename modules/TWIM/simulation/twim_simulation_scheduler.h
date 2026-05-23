#pragma once

#include "modules/TWIM/physics/twim_physics_engine.h"
#include "modules/TWIM/terrain/chunk_store.h"

#include "core/os/mutex.h"

#include <cstdint>

namespace twim {

class TwimSimulationScheduler {
public:
	Error run_step(chunk_store &p_chunks, const TileDefinitionRegistry &p_tile_definitions, uint32_t p_tick_count) const;

private:
	TwimPhysicsEngine physics_engine;

	struct GroupTaskData {
		chunk_store *chunks = nullptr;
		const Vector<chunk_coord> *coords = nullptr;
		const TwimPhysicsEngine *physics = nullptr;
		const TileDefinitionRegistry *tile_definitions = nullptr;
		uint32_t tick_count = 0;
		Mutex error_mutex;
		Error first_error = OK;
	};

	static void _simulate_chunk_group(void *p_userdata, uint32_t p_index);
};

} // namespace twim

