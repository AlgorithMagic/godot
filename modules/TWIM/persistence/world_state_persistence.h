#pragma once

#include "core/error/error_list.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

#include "modules/TWIM/entities/entity_instance.h"
#include "modules/TWIM/terrain/chunk_store.h"
#include "modules/TWIM/world/world_config.h"

namespace twim {

class WorldStatePersistence {
public:
	static Error save_world_state(
			const String &p_path,
			const WorldConfig &p_config,
			uint64_t p_tick_index,
			const chunk_store &p_chunks,
			const Vector<EntityInstance> &p_entities);

	static Error load_world_state(
			const String &p_path,
			WorldConfig &r_config,
			uint64_t &r_tick_index,
			chunk_store &r_chunks,
			Vector<EntityInstance> &r_entities);
};

} // namespace twim

