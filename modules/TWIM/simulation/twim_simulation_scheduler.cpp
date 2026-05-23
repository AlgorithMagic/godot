#include "twim_simulation_scheduler.h"

#include "core/object/worker_thread_pool.h"

namespace twim {

void TwimSimulationScheduler::_simulate_chunk_group(void *p_userdata, uint32_t p_index) {
	GroupTaskData *data = static_cast<GroupTaskData *>(p_userdata);
	if (data == nullptr || data->chunks == nullptr || data->coords == nullptr || data->physics == nullptr || data->tile_definitions == nullptr) {
		return;
	}

	const chunk_coord coord = (*data->coords)[p_index];
	chunk *target = data->chunks->get_chunk(coord);
	if (target == nullptr) {
		MutexLock<Mutex> lock(data->error_mutex);
		if (data->first_error == OK) {
			data->first_error = ERR_BUG;
		}
		return;
	}

	const Error simulation_error = data->physics->simulate_chunk(*target, *data->tile_definitions, data->tick_count);
	if (simulation_error != OK) {
		MutexLock<Mutex> lock(data->error_mutex);
		if (data->first_error == OK) {
			data->first_error = simulation_error;
		}
	}
}

Error TwimSimulationScheduler::run_step(chunk_store &p_chunks, const TileDefinitionRegistry &p_tile_definitions, uint32_t p_tick_count) const {
	if (p_tick_count == 0) {
		return OK;
	}

	const Vector<chunk_coord> loaded_coords = p_chunks.get_loaded_chunk_coords();
	if (loaded_coords.is_empty()) {
		return OK;
	}

	WorkerThreadPool *pool = WorkerThreadPool::get_singleton();
	if (pool == nullptr || loaded_coords.size() == 1) {
		for (const chunk_coord &coord : loaded_coords) {
			chunk *target = p_chunks.get_chunk(coord);
			if (target != nullptr) {
				const Error simulation_error = physics_engine.simulate_chunk(*target, p_tile_definitions, p_tick_count);
				if (simulation_error != OK) {
					return simulation_error;
				}
			} else {
				return ERR_BUG;
			}
		}
		return OK;
	}

	GroupTaskData task_data;
	task_data.chunks = &p_chunks;
	task_data.coords = &loaded_coords;
	task_data.physics = &physics_engine;
	task_data.tile_definitions = &p_tile_definitions;
	task_data.tick_count = p_tick_count;

	int32_t tasks = pool->get_thread_count() * 2;
	if (tasks < 1) {
		tasks = 1;
	}
	if (tasks > loaded_coords.size()) {
		tasks = loaded_coords.size();
	}

	const WorkerThreadPool::GroupID group = pool->add_native_group_task(
			&TwimSimulationScheduler::_simulate_chunk_group,
			&task_data,
			loaded_coords.size(),
			tasks,
			true,
			"TWIM simulation step");
	if (group == WorkerThreadPool::INVALID_TASK_ID) {
		return ERR_CANT_CREATE;
	}

	pool->wait_for_group_task_completion(group);
	return task_data.first_error;
}

} // namespace twim

