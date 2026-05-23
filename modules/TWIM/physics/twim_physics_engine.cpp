#include "twim_physics_engine.h"

#include "core/error/error_macros.h"

namespace twim {

Error TwimPhysicsEngine::simulate_chunk(chunk &p_chunk, const TileDefinitionRegistry &p_tile_definitions, uint32_t p_tick_count) const {
	if (p_tick_count == 0) {
		return OK;
	}

	const chunk_metrics &metrics = p_chunk.get_metrics();
	const tile_type_id air_id = p_tile_definitions.find_definition_id("air");
	ERR_FAIL_COND_V_MSG(!p_tile_definitions.has_definition(air_id), ERR_UNAVAILABLE, "TWIM physics requires an 'air' tile definition.");

	for (uint32_t tick = 0; tick < p_tick_count; ++tick) {
		for (int32_t z = 1; z < metrics.size_z; ++z) {
			for (int32_t y = 0; y < metrics.size_y; ++y) {
				for (int32_t x = 0; x < metrics.size_x; ++x) {
					const local_tile_coord from_local{ x, y, z };
					const local_tile_coord to_local{ x, y, z - 1 };

					const tile_cell from_cell = p_chunk.get_cell_local(from_local);
					const tile_cell to_cell = p_chunk.get_cell_local(to_local);

					const TileDefinition *from_definition = p_tile_definitions.get_definition(from_cell.type_id);
					if (from_definition == nullptr || !from_definition->liquid) {
						continue;
					}

					const TileDefinition *to_definition = p_tile_definitions.get_definition(to_cell.type_id);
					const bool can_flow_down = to_cell.type_id == air_id || (to_definition != nullptr && !to_definition->solid && !to_definition->liquid);
					if (!can_flow_down) {
						continue;
					}

					const Error set_down_error = p_chunk.set_cell_local(to_local, from_cell);
					ERR_FAIL_COND_V_MSG(set_down_error != OK, set_down_error, "TWIM physics failed to write destination liquid cell.");

					const Error set_up_error = p_chunk.set_cell_local(from_local, tile_cell(air_id));
					ERR_FAIL_COND_V_MSG(set_up_error != OK, set_up_error, "TWIM physics failed to clear source liquid cell.");
				}
			}
		}
	}

	p_chunk.clear_dirty_flags();
	return OK;
}

} // namespace twim

