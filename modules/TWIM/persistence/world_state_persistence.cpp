#include "world_state_persistence.h"

#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include "modules/TWIM/world/world_coord.h"

#include <cstdint>

namespace {
constexpr const char *KEY_CONFIG = "config";
constexpr const char *KEY_WORLD_NAME = "world_name";
constexpr const char *KEY_SEED = "seed";
constexpr const char *KEY_TICK_RATE_HZ = "tick_rate_hz";
constexpr const char *KEY_CHUNK_SIZE_X = "chunk_size_x";
constexpr const char *KEY_CHUNK_SIZE_Y = "chunk_size_y";
constexpr const char *KEY_CHUNK_SIZE_Z = "chunk_size_z";
constexpr const char *KEY_TILE_SET_PATH = "tile_set_path";
constexpr const char *KEY_ENTITY_CATALOG_PATH = "entity_catalog_path";
constexpr const char *KEY_ITEM_CATALOG_PATH = "item_catalog_path";
constexpr const char *KEY_WORLD_PROFILE_PATH = "world_profile_path";
constexpr const char *KEY_GLYPH_FONT_PATH = "glyph_font_resource_path";
constexpr const char *KEY_GLYPH_FONT_SIZE = "glyph_font_size";
constexpr const char *KEY_TICK_INDEX = "tick_index";
constexpr const char *KEY_CHUNKS = "chunks";
constexpr const char *KEY_COORD = "coord";
constexpr const char *KEY_CELLS = "cells";
constexpr const char *KEY_LX = "lx";
constexpr const char *KEY_LY = "ly";
constexpr const char *KEY_LZ = "lz";
constexpr const char *KEY_TILE = "tile";
constexpr const char *KEY_FLAGS = "flags";
constexpr const char *KEY_ENTITIES = "entities";
constexpr const char *KEY_ID = "id";
constexpr const char *KEY_TYPE_NAME = "type_name";
constexpr const char *KEY_TYPE_ID = "type_id";
constexpr const char *KEY_TILE_COORD = "tile_coord";
} // namespace

namespace twim {

Error WorldStatePersistence::save_world_state(
		const String &p_path,
		const WorldConfig &p_config,
		uint64_t p_tick_index,
		const chunk_store &p_chunks,
		const Vector<EntityInstance> &p_entities) {
	ERR_FAIL_COND_V_MSG(p_path.is_empty(), ERR_INVALID_PARAMETER, "TWIM save path cannot be empty.");
	const Error make_dir_error = DirAccess::make_dir_recursive_absolute(p_path.get_base_dir());
	ERR_FAIL_COND_V_MSG(make_dir_error != OK, make_dir_error, "Failed to create TWIM save directory.");

	Dictionary root;
	Dictionary config;
	config[KEY_WORLD_NAME] = p_config.world_name;
	config[KEY_SEED] = static_cast<int64_t>(p_config.seed);
	config[KEY_TICK_RATE_HZ] = static_cast<int64_t>(p_config.tick_rate_hz);
	config[KEY_CHUNK_SIZE_X] = p_config.metrics.size_x;
	config[KEY_CHUNK_SIZE_Y] = p_config.metrics.size_y;
	config[KEY_CHUNK_SIZE_Z] = p_config.metrics.size_z;
	config[KEY_TILE_SET_PATH] = p_config.tile_set_resource_path;
	config[KEY_ENTITY_CATALOG_PATH] = p_config.entity_catalog_resource_path;
	config[KEY_ITEM_CATALOG_PATH] = p_config.item_catalog_resource_path;
	config[KEY_WORLD_PROFILE_PATH] = p_config.world_profile_resource_path;
	config[KEY_GLYPH_FONT_PATH] = p_config.glyph_font_resource_path;
	config[KEY_GLYPH_FONT_SIZE] = p_config.glyph_font_size;
	root[KEY_CONFIG] = config;
	root[KEY_TICK_INDEX] = static_cast<int64_t>(p_tick_index);

	Array chunks;
	const Vector<chunk_coord> chunk_coords = p_chunks.get_loaded_chunk_coords();
	for (const chunk_coord &coord : chunk_coords) {
		const chunk *source_chunk = p_chunks.get_chunk(coord);
		if (source_chunk == nullptr) {
			continue;
		}

		Dictionary chunk_dict;
		chunk_dict[KEY_COORD] = coord.to_vector3_i();

		Array chunk_cells;
		const chunk_metrics &metrics = source_chunk->get_metrics();
		for (int32_t z = 0; z < metrics.size_z; ++z) {
			for (int32_t y = 0; y < metrics.size_y; ++y) {
				for (int32_t x = 0; x < metrics.size_x; ++x) {
					const local_tile_coord local_coord{ x, y, z };
					const tile_cell cell = source_chunk->get_cell_local(local_coord);
					if (cell.type_id.value == 0 && cell.flags == 0) {
						continue;
					}

					Dictionary cell_dict;
					cell_dict[KEY_LX] = x;
					cell_dict[KEY_LY] = y;
					cell_dict[KEY_LZ] = z;
					cell_dict[KEY_TILE] = static_cast<int64_t>(cell.type_id.value);
					cell_dict[KEY_FLAGS] = static_cast<int64_t>(cell.flags);
					chunk_cells.push_back(cell_dict);
				}
			}
		}

		chunk_dict[KEY_CELLS] = chunk_cells;
		chunks.push_back(chunk_dict);
	}
	root[KEY_CHUNKS] = chunks;

	Array entities;
	for (const EntityInstance &entity : p_entities) {
		Dictionary entity_dict;
		entity_dict[KEY_ID] = entity.id;
		entity_dict[KEY_TYPE_NAME] = entity.type_name;
		entity_dict[KEY_TYPE_ID] = entity.type_id;
		entity_dict[KEY_TILE_COORD] = entity.tile_coord;
		entities.push_back(entity_dict);
	}
	root[KEY_ENTITIES] = entities;

	Error open_error = OK;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &open_error);
	ERR_FAIL_COND_V_MSG(file.is_null(), open_error, "Failed to open TWIM save file for writing.");
	file->store_var(root, false);
	return file->get_error();
}

Error WorldStatePersistence::load_world_state(
		const String &p_path,
		WorldConfig &r_config,
		uint64_t &r_tick_index,
		chunk_store &r_chunks,
		Vector<EntityInstance> &r_entities) {
	ERR_FAIL_COND_V_MSG(p_path.is_empty(), ERR_INVALID_PARAMETER, "TWIM load path cannot be empty.");

	Error open_error = OK;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ, &open_error);
	ERR_FAIL_COND_V_MSG(file.is_null(), open_error, "Failed to open TWIM save file for reading.");

	const Variant value = file->get_var(false);
	ERR_FAIL_COND_V_MSG(file->get_error() != OK, file->get_error(), "Failed while reading TWIM save file data.");
	ERR_FAIL_COND_V_MSG(value.get_type() != Variant::DICTIONARY, ERR_INVALID_DATA, "TWIM save data root must be a dictionary.");

	const Dictionary root = value;
	ERR_FAIL_COND_V_MSG(!root.has(KEY_CONFIG), ERR_INVALID_DATA, "TWIM save data is missing config section.");
	ERR_FAIL_COND_V_MSG(root[KEY_CONFIG].get_type() != Variant::DICTIONARY, ERR_INVALID_DATA, "TWIM config section must be a dictionary.");

	const Dictionary config = root[KEY_CONFIG];
	r_config = WorldConfig();
	r_config.world_name = config.get(KEY_WORLD_NAME, String());
	r_config.seed = static_cast<uint64_t>(int64_t(config.get(KEY_SEED, int64_t(0))));
	r_config.tick_rate_hz = static_cast<uint32_t>(int64_t(config.get(KEY_TICK_RATE_HZ, int64_t(WorldConfig::DEFAULT_TICK_RATE_HZ))));
	r_config.metrics.size_x = int32_t(config.get(KEY_CHUNK_SIZE_X, chunk_metrics::DEFAULT_SIZE_X));
	r_config.metrics.size_y = int32_t(config.get(KEY_CHUNK_SIZE_Y, chunk_metrics::DEFAULT_SIZE_Y));
	r_config.metrics.size_z = int32_t(config.get(KEY_CHUNK_SIZE_Z, chunk_metrics::DEFAULT_SIZE_Z));
	r_config.tile_set_resource_path = config.get(KEY_TILE_SET_PATH, String());
	r_config.entity_catalog_resource_path = config.get(KEY_ENTITY_CATALOG_PATH, String());
	r_config.item_catalog_resource_path = config.get(KEY_ITEM_CATALOG_PATH, String());
	r_config.world_profile_resource_path = config.get(KEY_WORLD_PROFILE_PATH, String());
	r_config.glyph_font_resource_path = config.get(KEY_GLYPH_FONT_PATH, String());
	r_config.glyph_font_size = int32_t(config.get(KEY_GLYPH_FONT_SIZE, 16));

	ERR_FAIL_COND_V_MSG(r_config.validate() != OK, ERR_INVALID_DATA, "TWIM save data contains invalid world configuration.");

	r_tick_index = static_cast<uint64_t>(int64_t(root.get(KEY_TICK_INDEX, int64_t(0))));
	r_chunks.clear();
	r_chunks.set_chunk_metrics(r_config.metrics);

	if (root.has(KEY_CHUNKS)) {
		ERR_FAIL_COND_V_MSG(root[KEY_CHUNKS].get_type() != Variant::ARRAY, ERR_INVALID_DATA, "TWIM chunks section must be an array.");
		const Array chunks = root[KEY_CHUNKS];
		for (int32_t i = 0; i < chunks.size(); ++i) {
			const Variant chunk_var = chunks[i];
			if (chunk_var.get_type() != Variant::DICTIONARY) {
				return ERR_INVALID_DATA;
			}

			const Dictionary chunk_dict = chunk_var;
			const Vector3i coord_vec = chunk_dict.get(KEY_COORD, Vector3i());
			const chunk_coord coord{ coord_vec.x, coord_vec.y, coord_vec.z };
			ERR_FAIL_COND_V_MSG(r_chunks.create_chunk(coord) != OK, ERR_INVALID_DATA, "Failed to create chunk while loading TWIM world state.");

			chunk *target_chunk = r_chunks.get_chunk(coord);
			ERR_FAIL_NULL_V(target_chunk, ERR_BUG);

			const Variant cells_variant = chunk_dict.get(KEY_CELLS, Array());
			ERR_FAIL_COND_V_MSG(cells_variant.get_type() != Variant::ARRAY, ERR_INVALID_DATA, "TWIM chunk cells section must be an array.");
			const Array chunk_cells = cells_variant;
			for (int32_t c = 0; c < chunk_cells.size(); ++c) {
				const Variant cell_var = chunk_cells[c];
				if (cell_var.get_type() != Variant::DICTIONARY) {
					return ERR_INVALID_DATA;
				}
				const Dictionary cell_dict = cell_var;

				const local_tile_coord local_coord{
					int32_t(cell_dict.get(KEY_LX, 0)),
					int32_t(cell_dict.get(KEY_LY, 0)),
					int32_t(cell_dict.get(KEY_LZ, 0)),
				};

				ERR_FAIL_COND_V_MSG(!is_valid_local_tile_coord(local_coord, r_config.metrics), ERR_INVALID_DATA, "TWIM save data includes invalid local cell coordinate.");

				const uint16_t type_id = static_cast<uint16_t>(int64_t(cell_dict.get(KEY_TILE, int64_t(0))));
				const int64_t flags_raw = int64_t(cell_dict.get(KEY_FLAGS, int64_t(0)));
				ERR_FAIL_COND_V_MSG(flags_raw < 0 || flags_raw > UINT16_MAX, ERR_INVALID_DATA, "TWIM save data includes invalid tile flags value.");
				ERR_FAIL_COND_V_MSG(target_chunk->set_cell_local(local_coord, tile_cell(tile_type_id(type_id), static_cast<uint16_t>(flags_raw))) != OK, ERR_INVALID_DATA, "Failed to restore chunk cell while loading TWIM world state.");
			}

			target_chunk->clear_dirty_flags();
		}
	}

	r_entities.clear();
	if (root.has(KEY_ENTITIES)) {
		ERR_FAIL_COND_V_MSG(root[KEY_ENTITIES].get_type() != Variant::ARRAY, ERR_INVALID_DATA, "TWIM entities section must be an array.");
		const Array entities = root[KEY_ENTITIES];
		r_entities.resize(entities.size());
		for (int32_t i = 0; i < entities.size(); ++i) {
			const Variant entity_var = entities[i];
			if (entity_var.get_type() != Variant::DICTIONARY) {
				return ERR_INVALID_DATA;
			}
			const Dictionary entity_dict = entity_var;

			EntityInstance entity;
			entity.id = int64_t(entity_dict.get(KEY_ID, int64_t(0)));
			entity.type_name = entity_dict.get(KEY_TYPE_NAME, StringName());
			entity.type_id = int64_t(entity_dict.get(KEY_TYPE_ID, int64_t(-1)));
			entity.tile_coord = entity_dict.get(KEY_TILE_COORD, Vector3i());

			ERR_FAIL_COND_V_MSG(entity.id <= 0, ERR_INVALID_DATA, "TWIM save data includes an entity with invalid ID.");
			ERR_FAIL_COND_V_MSG(entity.type_name == StringName(), ERR_INVALID_DATA, "TWIM save data includes an entity with empty type name.");
			ERR_FAIL_COND_V_MSG(!is_packable_world_coord(entity.tile_coord), ERR_INVALID_DATA, "TWIM save data includes an entity with unsupported coordinates.");

			r_entities.write[i] = entity;
		}
	}

	return OK;
}

} // namespace twim




