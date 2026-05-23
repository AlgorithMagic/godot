//
// Created by busin on 2026-05-22.
//

#include "world_runtime.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include "modules/TWIM/entities/entity_catalog_resource.h"
#include "modules/TWIM/entities/entity_type_resource.h"
#include "modules/TWIM/items/item_catalog_resource.h"
#include "modules/TWIM/items/item_type_resource.h"
#include "modules/TWIM/persistence/world_state_persistence.h"
#include "modules/TWIM/terrain/tile_set_resource.h"
#include "modules/TWIM/terrain/tile_type_resource.h"
#include "modules/TWIM/world/world_profile_resource.h"

namespace twim {

namespace {
bool is_valid_unicode_scalar(int32_t p_codepoint) {
	if (p_codepoint < 0 || p_codepoint > 0x10FFFF) {
		return false;
	}

	return !(p_codepoint >= 0xD800 && p_codepoint <= 0xDFFF);
}
} // namespace

Error WorldRuntime::create_world(const String &p_world_name, uint64_t p_seed) {
	const WorldConfig new_config = WorldConfig::from_project_settings(p_world_name, p_seed);
	return create_world(new_config);
}

Error WorldRuntime::create_world(const WorldConfig &p_config) {
	const Error validation_error = p_config.validate();
	ERR_FAIL_COND_V_MSG(validation_error != OK, validation_error, "Invalid TWIM world configuration.");
	async_generation_guard.cancel_pending_work();

	unload_world();

	config = p_config;
	ERR_FAIL_COND_V_MSG(!config.metrics.is_valid(), ERR_INVALID_PARAMETER, "TWIM chunk metrics must be positive.");

	if (!config.world_profile_resource_path.is_empty()) {
		const Error profile_error = apply_world_profile(config.world_profile_resource_path);
		ERR_FAIL_COND_V_MSG(profile_error != OK, profile_error, "Failed to load TWIM world profile resource.");
	}

	const Error clock_error = clock.reset(config.tick_rate_hz);
	ERR_FAIL_COND_V_MSG(clock_error != OK, clock_error, "Failed to initialize TWIM world clock.");

	chunks.set_chunk_metrics(config.metrics);

	Error definitions_error = tile_definitions.register_builtin_definitions();
	ERR_FAIL_COND_V_MSG(definitions_error != OK, definitions_error, "Failed to register TWIM built-in tile definitions.");

	if (!config.tile_set_resource_path.is_empty()) {
		definitions_error = load_tiles_from_resource(config.tile_set_resource_path);
		ERR_FAIL_COND_V_MSG(definitions_error != OK, definitions_error, "Failed to load TWIM tile set resource.");
	}

	entity_types.clear();
	item_types.clear();
	entities.clear();
	if (!config.entity_catalog_resource_path.is_empty()) {
		const Error entities_error = load_entities_from_resource(config.entity_catalog_resource_path);
		ERR_FAIL_COND_V_MSG(entities_error != OK, entities_error, "Failed to load TWIM entity catalog resource.");
	}
	if (!config.item_catalog_resource_path.is_empty()) {
		const Error items_error = load_items_from_resource(config.item_catalog_resource_path);
		ERR_FAIL_COND_V_MSG(items_error != OK, items_error, "Failed to load TWIM item catalog resource.");
	}

	const Error origin_chunk_error = chunks.create_chunk(chunk_coord{ 0, 0, 0 });
	ERR_FAIL_COND_V_MSG(origin_chunk_error != OK, origin_chunk_error, "Failed to create TWIM origin chunk.");

	world_created = true;

	return OK;
}

void WorldRuntime::unload_world() {
	async_generation_guard.cancel_pending_work();
	world_created = false;
	config = WorldConfig();
	entity_types.clear();
	item_types.clear();
	entities.clear();

	const Error clock_error = clock.reset(WorldConfig::DEFAULT_TICK_RATE_HZ);
	ERR_FAIL_COND_MSG(clock_error != OK, "Failed to reset TWIM world clock.");

	tile_definitions.clear();
	chunks.clear();
}

bool WorldRuntime::has_world() const {
	return world_created;
}

Error WorldRuntime::step_ticks(uint32_t p_tick_count) {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");

	if (p_tick_count == 0) {
		return OK;
	}

	ERR_FAIL_COND_V_MSG(
			p_tick_count > WorldConfig::MAX_TICKS_PER_STEP,
			ERR_INVALID_PARAMETER,
			"TWIM tick step count exceeds the configured safety limit.");

	const Error simulation_error = simulation_scheduler.run_step(chunks, tile_definitions, p_tick_count);
	ERR_FAIL_COND_V_MSG(simulation_error != OK, simulation_error, "TWIM simulation scheduler failed to complete.");

	return clock.step_ticks(p_tick_count);
}

uint64_t WorldRuntime::get_tick_index() const {
	return clock.get_tick_index();
}

const WorldConfig &WorldRuntime::get_config() const {
	return config;
}

Error WorldRuntime::debug_set_tile_type_id(Vector3i p_tile_coord, uint16_t p_tile_type_id) {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");

	const tile_type_id tile_type_id(p_tile_type_id);

	ERR_FAIL_COND_V_MSG(
			!tile_definitions.has_definition(tile_type_id),
			ERR_INVALID_PARAMETER,
			"Invalid TWIM tile type ID.");

	return chunks.set_tile_type_id(tile_coord::from_vector3_i(p_tile_coord), tile_type_id);
}

int32_t WorldRuntime::get_tile_type_id(Vector3i p_tile_coord) const {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, -1, "No TWIM world is loaded.");

	return static_cast<int32_t>(chunks.get_tile_type_id(tile_coord::from_vector3_i(p_tile_coord)).value);
}

int32_t WorldRuntime::get_tile_type_id_by_name(const StringName &p_tile_type_name) const {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, -1, "No TWIM world is loaded.");
	if (!tile_definitions.has_definition_name(p_tile_type_name)) {
		return -1;
	}

	return static_cast<int32_t>(tile_definitions.find_definition_id(p_tile_type_name).value);
}

PackedStringArray WorldRuntime::get_tile_type_names() const {
	PackedStringArray names;
	if (validate_world_loaded() != OK) {
		return names;
	}

	for (uint32_t i = 0; i < tile_definitions.get_definition_count(); ++i) {
		const TileDefinition *definition = tile_definitions.get_definition(tile_type_id(static_cast<uint16_t>(i)));
		if (definition == nullptr) {
			continue;
		}
		names.push_back(String(definition->name));
	}

	return names;
}

PackedStringArray WorldRuntime::get_entity_type_names() const {
	PackedStringArray names;
	if (validate_world_loaded() != OK) {
		return names;
	}

	for (const KeyValue<StringName, EntityTypeDefinition> &entry : entity_types) {
		names.push_back(String(entry.key));
	}
	names.sort();

	return names;
}

PackedStringArray WorldRuntime::get_item_type_names() const {
	PackedStringArray names;
	if (validate_world_loaded() != OK) {
		return names;
	}

	for (const KeyValue<StringName, ItemTypeDefinition> &entry : item_types) {
		names.push_back(String(entry.key));
	}
	names.sort();

	return names;
}

Error WorldRuntime::spawn_entity(const StringName &p_entity_type_name, Vector3i p_tile_coord, int64_t &r_entity_id) {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");

	const EntityTypeDefinition *type_def = entity_types.getptr(p_entity_type_name);
	ERR_FAIL_COND_V_MSG(type_def == nullptr, ERR_INVALID_PARAMETER, "Unknown TWIM entity type name.");

	return entities.spawn_entity(p_entity_type_name, type_def->type_id, p_tile_coord, r_entity_id);
}

Error WorldRuntime::move_entity(int64_t p_entity_id, Vector3i p_tile_coord) {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return entities.move_entity(p_entity_id, p_tile_coord);
}

Error WorldRuntime::remove_entity(int64_t p_entity_id) {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return entities.remove_entity(p_entity_id);
}

Dictionary WorldRuntime::get_entity_debug_info(int64_t p_entity_id) const {
	Dictionary info;
	if (validate_world_loaded() != OK) {
		info["available"] = false;
		info["reason"] = "No TWIM world is loaded.";
		return info;
	}

	const EntityInstance *entity = entities.get_entity(p_entity_id);
	if (entity == nullptr) {
		info["available"] = false;
		info["reason"] = "Entity ID does not exist.";
		info["entity_id"] = p_entity_id;
		return info;
	}

	info["available"] = true;
	info["entity_id"] = entity->id;
	info["entity_type_name"] = entity->type_name;
	info["entity_type_id"] = entity->type_id;
	info["tile_coord"] = entity->tile_coord;
	if (const EntityTypeDefinition *type_def = entity_types.getptr(entity->type_name); type_def != nullptr) {
		info["glyph_id"] = type_def->glyph_id;
		info["glyph"] = String::chr(static_cast<char32_t>(type_def->glyph_id));
		info["blocking"] = type_def->blocking;
	}
	return info;
}

int64_t WorldRuntime::get_entity_count() const {
	return entities.get_entity_count();
}

Error WorldRuntime::save_world_state(const String &p_path) const {
	ERR_FAIL_COND_V_MSG(validate_world_loaded() != OK, ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return WorldStatePersistence::save_world_state(p_path, config, clock.get_tick_index(), chunks, entities.export_entities());
}

Error WorldRuntime::load_world_state(const String &p_path) {
	async_generation_guard.cancel_pending_work();
	WorldConfig loaded_config;
	uint64_t loaded_tick_index = 0;
	Vector<EntityInstance> loaded_entities;

	const Error load_error = WorldStatePersistence::load_world_state(p_path, loaded_config, loaded_tick_index, chunks, loaded_entities);
	ERR_FAIL_COND_V_MSG(load_error != OK, load_error, "Failed to load TWIM world state.");

	config = loaded_config;
	const Error clock_error = clock.reset(config.tick_rate_hz);
	ERR_FAIL_COND_V_MSG(clock_error != OK, clock_error, "Failed to reset TWIM world clock after load.");
	uint64_t remaining_ticks = loaded_tick_index;
	while (remaining_ticks > 0) {
		const uint32_t step = remaining_ticks > WorldConfig::MAX_TICKS_PER_STEP ? WorldConfig::MAX_TICKS_PER_STEP : static_cast<uint32_t>(remaining_ticks);
		ERR_FAIL_COND_V_MSG(clock.step_ticks(step) != OK, ERR_INVALID_DATA, "Loaded tick index exceeds supported range.");
		remaining_ticks -= step;
	}

	entity_types.clear();
	item_types.clear();
	if (!config.entity_catalog_resource_path.is_empty()) {
		const Error entities_error = load_entities_from_resource(config.entity_catalog_resource_path);
		ERR_FAIL_COND_V_MSG(entities_error != OK, entities_error, "Failed to load TWIM entity catalog resource while loading world state.");
	}

	const Error import_entities_error = entities.import_entities(loaded_entities);
	ERR_FAIL_COND_V_MSG(import_entities_error != OK, import_entities_error, "Failed to import TWIM entities while loading world state.");
	if (config.entity_catalog_resource_path.is_empty()) {
		for (const EntityInstance &entity : loaded_entities) {
			if (!entity_types.has(entity.type_name)) {
				EntityTypeDefinition definition;
				definition.type_id = entity.type_id;
				entity_types.insert(entity.type_name, definition);
			}
		}
	}
	if (!config.item_catalog_resource_path.is_empty()) {
		const Error items_error = load_items_from_resource(config.item_catalog_resource_path);
		ERR_FAIL_COND_V_MSG(items_error != OK, items_error, "Failed to load TWIM item catalog resource while loading world state.");
	}

	tile_definitions.clear();
	Error definitions_error = tile_definitions.register_builtin_definitions();
	ERR_FAIL_COND_V_MSG(definitions_error != OK, definitions_error, "Failed to register TWIM built-in tile definitions while loading world state.");
	if (!config.tile_set_resource_path.is_empty()) {
		definitions_error = load_tiles_from_resource(config.tile_set_resource_path);
		ERR_FAIL_COND_V_MSG(definitions_error != OK, definitions_error, "Failed to load TWIM tile set resource while loading world state.");
	}

	world_created = true;
	return OK;
}

Dictionary WorldRuntime::build_debug_render_snapshot(Vector3i p_origin, Vector2i p_size, int32_t p_z_layer) const {
	if (validate_world_loaded() != OK) {
		RenderSnapshot snapshot;
		snapshot.reason = "No TWIM world is loaded.";
		return snapshot.to_dictionary();
	}

	const RenderSnapshot snapshot = render_snapshot_builder.build(
			chunks,
			tile_definitions,
			p_origin,
			p_size,
			p_z_layer,
			clock.get_tick_index(),
			config.glyph_font_resource_path,
			config.glyph_font_size);
	return snapshot.to_dictionary();
}

Dictionary WorldRuntime::get_world_debug_info() const {
	Dictionary info;

	info["world_loaded"] = world_created;
	info["tick_index"] = static_cast<int64_t>(clock.get_tick_index());
	info["tick_rate_hz"] = static_cast<int64_t>(clock.get_tick_rate_hz());
	info["chunk_size_x"] = config.metrics.size_x;
	info["chunk_size_y"] = config.metrics.size_y;
	info["chunk_size_z"] = config.metrics.size_z;
	info["glyph_font_resource_path"] = config.glyph_font_resource_path;
	info["glyph_font_size"] = config.glyph_font_size;
	info["async_generation"] = static_cast<int64_t>(async_generation_guard.get_current_generation());

	if (!world_created) {
		info["world_name"] = String();
		info["seed"] = 0;
		info["deterministic"] = true;
		info["terrain_available"] = false;
		info["tile_definition_count"] = 0;
		info["loaded_chunk_count"] = 0;
		info["entity_definition_count"] = 0;
		info["item_definition_count"] = 0;
		info["loaded_entity_count"] = 0;
		return info;
	}

	info["world_name"] = config.world_name;
	info["seed"] = static_cast<int64_t>(config.seed);
	info["deterministic"] = config.deterministic;
	info["terrain_available"] = true;
	info["tile_definition_count"] = static_cast<int64_t>(tile_definitions.get_definition_count());
	info["loaded_chunk_count"] = static_cast<int64_t>(chunks.get_loaded_chunk_count());
	info["entity_definition_count"] = static_cast<int64_t>(entity_types.size());
	info["item_definition_count"] = static_cast<int64_t>(item_types.size());
	info["loaded_entity_count"] = entities.get_entity_count();

	return info;
}

Dictionary WorldRuntime::get_chunk_debug_info(Vector3i p_chunk_coord) const {
	if (validate_world_loaded() != OK) {
		Dictionary info;
		info["available"] = false;
		info["reason"] = "No TWIM world is loaded.";
		info["chunk_coord"] = p_chunk_coord;
		return info;
	}

	return chunks.get_chunk_debug_info(chunk_coord{ p_chunk_coord.x, p_chunk_coord.y, p_chunk_coord.z });
}

Error WorldRuntime::validate_world_loaded() const {
	return world_created ? OK : ERR_UNCONFIGURED;
}

Error WorldRuntime::load_tiles_from_resource(const String &p_resource_path) {
	Ref<Resource> base_resource = ResourceLoader::load(p_resource_path);
	ERR_FAIL_COND_V_MSG(base_resource.is_null(), ERR_FILE_NOT_FOUND, "TWIM tile set resource path could not be loaded.");

	Ref<TwimTileSetResource> tile_set = base_resource;
	ERR_FAIL_COND_V_MSG(tile_set.is_null(), ERR_INVALID_DATA, "TWIM tile set resource path must point to TwimTileSetResource.");

	TileDefinitionRegistry loaded_definitions;
	const Array tile_types = tile_set->get_tile_types();

	if (tile_types.is_empty()) {
		const Error error = loaded_definitions.register_builtin_definitions();
		if (error == OK) {
			tile_definitions = loaded_definitions;
		}
		return error;
	}

	for (int32_t index = 0; index < tile_types.size(); ++index) {
		Ref<TwimTileTypeResource> tile_type = tile_types[index];
		if (tile_type.is_null()) {
			return ERR_INVALID_DATA;
		}

		const StringName tile_name = tile_type->get_tile_name();
		if (tile_name == StringName()) {
			return ERR_INVALID_DATA;
		}

		const Error add_error = loaded_definitions.register_definition_with_next_id(
				tile_name,
				tile_type->is_solid(),
				tile_type->is_opaque(),
				tile_type->is_liquid(),
				is_valid_unicode_scalar(tile_type->get_glyph_id()) ? static_cast<uint32_t>(tile_type->get_glyph_id()) : static_cast<uint32_t>('?'),
				static_cast<uint32_t>(tile_type->get_foreground_rgba()),
				static_cast<uint32_t>(tile_type->get_background_rgba()));
		if (add_error != OK) {
			return add_error;
		}
	}

	tile_definitions = loaded_definitions;
	return OK;
}

Error WorldRuntime::load_entities_from_resource(const String &p_resource_path) {
	Ref<Resource> base_resource = ResourceLoader::load(p_resource_path);
	ERR_FAIL_COND_V_MSG(base_resource.is_null(), ERR_FILE_NOT_FOUND, "TWIM entity catalog resource path could not be loaded.");

	Ref<TwimEntityCatalogResource> catalog = base_resource;
	ERR_FAIL_COND_V_MSG(catalog.is_null(), ERR_INVALID_DATA, "TWIM entity catalog resource path must point to TwimEntityCatalogResource.");

	entity_types.clear();
	const Array entity_entries = catalog->get_entity_types();
	for (int32_t index = 0; index < entity_entries.size(); ++index) {
		Ref<TwimEntityTypeResource> entity_type_resource = entity_entries[index];
		if (entity_type_resource.is_null()) {
			return ERR_INVALID_DATA;
		}

		const StringName entity_name = entity_type_resource->get_entity_name();
		if (entity_name == StringName()) {
			return ERR_INVALID_DATA;
		}

		if (entity_types.has(entity_name)) {
			return ERR_ALREADY_EXISTS;
		}

		if (!is_valid_unicode_scalar(entity_type_resource->get_glyph_id())) {
			return ERR_INVALID_DATA;
		}

		EntityTypeDefinition definition;
		definition.type_id = index;
		definition.glyph_id = entity_type_resource->get_glyph_id();
		definition.blocking = entity_type_resource->is_blocking();
		entity_types.insert(entity_name, definition);
	}

	return OK;
}

Error WorldRuntime::load_items_from_resource(const String &p_resource_path) {
	Ref<Resource> base_resource = ResourceLoader::load(p_resource_path);
	ERR_FAIL_COND_V_MSG(base_resource.is_null(), ERR_FILE_NOT_FOUND, "TWIM item catalog resource path could not be loaded.");

	Ref<TwimItemCatalogResource> catalog = base_resource;
	ERR_FAIL_COND_V_MSG(catalog.is_null(), ERR_INVALID_DATA, "TWIM item catalog resource path must point to TwimItemCatalogResource.");

	item_types.clear();
	const Array item_entries = catalog->get_item_types();
	for (int32_t index = 0; index < item_entries.size(); ++index) {
		Ref<TwimItemTypeResource> item_type_resource = item_entries[index];
		if (item_type_resource.is_null()) {
			return ERR_INVALID_DATA;
		}

		const StringName item_name = item_type_resource->get_item_name();
		if (item_name == StringName()) {
			return ERR_INVALID_DATA;
		}

		if (item_types.has(item_name)) {
			return ERR_ALREADY_EXISTS;
		}

		if (!is_valid_unicode_scalar(item_type_resource->get_glyph_id())) {
			return ERR_INVALID_DATA;
		}

		if (item_type_resource->get_unit_weight() < 0.0) {
			return ERR_INVALID_DATA;
		}

		ItemTypeDefinition definition;
		definition.type_id = index;
		definition.glyph_id = item_type_resource->get_glyph_id();
		definition.stackable = item_type_resource->is_stackable();
		definition.unit_weight = item_type_resource->get_unit_weight();
		item_types.insert(item_name, definition);
	}

	return OK;
}

Error WorldRuntime::apply_world_profile(const String &p_resource_path) {
	Ref<Resource> base_resource = ResourceLoader::load(p_resource_path);
	ERR_FAIL_COND_V_MSG(base_resource.is_null(), ERR_FILE_NOT_FOUND, "TWIM world profile resource path could not be loaded.");

	Ref<TwimWorldProfileResource> profile = base_resource;
	ERR_FAIL_COND_V_MSG(profile.is_null(), ERR_INVALID_DATA, "TWIM world profile resource path must point to TwimWorldProfileResource.");

	config.tick_rate_hz = static_cast<uint32_t>(profile->get_tick_rate_hz());
	config.metrics.size_x = profile->get_chunk_size_x();
	config.metrics.size_y = profile->get_chunk_size_y();
	config.metrics.size_z = profile->get_chunk_size_z();
	config.glyph_font_resource_path = profile->get_glyph_font_resource_path();
	config.glyph_font_size = profile->get_glyph_font_size();

	return config.validate();
}

} // namespace twim
