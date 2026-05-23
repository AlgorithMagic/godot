//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/error/error_list.h"
#include "core/math/vector2i.h"
#include "core/math/vector3i.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include "modules/TWIM/entities/entity_store.h"
#include "modules/TWIM/rendering/render_snapshot_builder.h"
#include "modules/TWIM/simulation/twim_async_generation_guard.h"
#include "modules/TWIM/simulation/twim_simulation_scheduler.h"
#include "modules/TWIM/terrain/chunk_store.h"
#include "modules/TWIM/terrain/tile_registry.h"
#include "modules/TWIM/world/world_clock.h"
#include "modules/TWIM/world/world_config.h"

#include <cstdint>

namespace twim {

class WorldRuntime {
public:
	WorldRuntime() = default;
	~WorldRuntime() = default;

	WorldRuntime(const WorldRuntime &) = delete;
	WorldRuntime &operator=(const WorldRuntime &) = delete;

	Error create_world(const String &p_world_name, uint64_t p_seed);
	Error create_world(const WorldConfig &p_config);

	void unload_world();
	bool has_world() const;

	Error step_ticks(uint32_t p_tick_count);
	uint64_t get_tick_index() const;

	const WorldConfig &get_config() const;

	Error debug_set_tile_type_id(Vector3i p_tile_coord, uint16_t p_tile_type_id);
	int32_t get_tile_type_id(Vector3i p_tile_coord) const;
	int32_t get_tile_type_id_by_name(const StringName &p_tile_type_name) const;
	PackedStringArray get_tile_type_names() const;
	PackedStringArray get_entity_type_names() const;
	PackedStringArray get_item_type_names() const;

	Error spawn_entity(const StringName &p_entity_type_name, Vector3i p_tile_coord, int64_t &r_entity_id);
	Error move_entity(int64_t p_entity_id, Vector3i p_tile_coord);
	Error remove_entity(int64_t p_entity_id);
	Dictionary get_entity_debug_info(int64_t p_entity_id) const;
	int64_t get_entity_count() const;

	Error save_world_state(const String &p_path) const;
	Error load_world_state(const String &p_path);

	Dictionary build_debug_render_snapshot(Vector3i p_origin, Vector2i p_size, int32_t p_z_layer) const;

	Dictionary get_world_debug_info() const;
	Dictionary get_chunk_debug_info(Vector3i p_chunk_coord) const;

private:
	struct EntityTypeDefinition {
		int64_t type_id = -1;
		int32_t glyph_id = '@';
		bool blocking = false;
	};

	struct ItemTypeDefinition {
		int64_t type_id = -1;
		int32_t glyph_id = '!';
		bool stackable = true;
		double unit_weight = 1.0;
	};

	bool world_created = false;

	WorldConfig config;
	WorldClock clock;

	TileDefinitionRegistry tile_definitions;
	chunk_store chunks;
	EntityStore entities;
	TwimAsyncGenerationGuard async_generation_guard;
	TwimSimulationScheduler simulation_scheduler;
	RenderSnapshotBuilder render_snapshot_builder;

	HashMap<StringName, EntityTypeDefinition> entity_types;
	HashMap<StringName, ItemTypeDefinition> item_types;

	Error validate_world_loaded() const;
	Error load_tiles_from_resource(const String &p_resource_path);
	Error load_entities_from_resource(const String &p_resource_path);
	Error load_items_from_resource(const String &p_resource_path);
	Error apply_world_profile(const String &p_resource_path);
};

} // namespace twim
