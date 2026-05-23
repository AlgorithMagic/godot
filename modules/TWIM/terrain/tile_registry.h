//
// Created by busin on 2026-05-23.
//

#pragma once

#include "core/error/error_list.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"

#include "modules/TWIM/terrain/tile_definition.h"

namespace twim {

class TileDefinitionRegistry {
public:
	enum BuiltinTile : uint16_t {
		TILE_AIR = 0,
		TILE_WATER = 1,
		TILE_DIRT = 2,
		TILE_SAND = 3,
		TILE_GRAVEL = 4,
		TILE_CLAY = 5,
		TILE_MUD = 6,
		TILE_SNOW = 7,
		TILE_ICE = 8,
		TILE_GRANITE = 9,
		TILE_BASALT = 10,
		TILE_LIMESTONE = 11,
		TILE_SANDSTONE = 12,
		TILE_MARBLE = 13,
		TILE_SLATE = 14,
		TILE_OBSIDIAN = 15,
		TILE_OAK_WOOD = 16,
		TILE_PINE_WOOD = 17,
		TILE_BIRCH_WOOD = 18,
		TILE_MAPLE_WOOD = 19,
		TILE_WILLOW_WOOD = 20,
		TILE_BAMBOO = 21,
		TILE_STONE_BRICK = 22,
		TILE_COBBLESTONE = 23,
	};

	void clear() {
		definitions.clear();
		name_to_id.clear();
	}

	Error register_builtin_definitions() {
		clear();
		return register_default_material_set();
	}

	Error register_definition(const TileDefinition &p_definition) {
		if (p_definition.id.value != definitions.size()) {
			return ERR_INVALID_PARAMETER;
		}

		if (name_to_id.has(p_definition.name)) {
			return ERR_ALREADY_EXISTS;
		}

		definitions.push_back(p_definition);
		name_to_id.insert(p_definition.name, p_definition.id);
		return OK;
	}

	Error register_definition_with_next_id(
			const StringName &p_name,
			bool p_solid,
			bool p_opaque,
			bool p_liquid,
			uint32_t p_glyph_id,
			uint32_t p_foreground_rgba,
			uint32_t p_background_rgba) {
		if (name_to_id.has(p_name)) {
			return ERR_ALREADY_EXISTS;
		}

		if (definitions.size() > UINT16_MAX) {
			return ERR_OUT_OF_MEMORY;
		}

		return register_definition(TileDefinition(
				tile_type_id(static_cast<uint16_t>(definitions.size())),
				p_name,
				p_solid,
				p_opaque,
				p_liquid,
				p_glyph_id,
				p_foreground_rgba,
				p_background_rgba));
	}

	bool has_definition(tile_type_id p_id) const {
		return p_id.value < definitions.size();
	}

	const TileDefinition *get_definition(tile_type_id p_id) const {
		if (!has_definition(p_id)) {
			return nullptr;
		}

		return &definitions[p_id.value];
	}

	uint32_t get_definition_count() const {
		return definitions.size();
	}

	bool has_definition_name(const StringName &p_name) const {
		return name_to_id.has(p_name);
	}

	tile_type_id find_definition_id(const StringName &p_name) const {
		const tile_type_id *id = name_to_id.getptr(p_name);
		return id == nullptr ? tile_type_id(TILE_AIR) : *id;
	}

	Error register_default_material_set() {
		Error error = OK;

		error = register_definition_with_next_id("air", false, false, false, ' ', 0x00000000, 0x00000000);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("water", false, false, true, '~', 0x4080ffff, 0x000020ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("dirt", false, false, false, '.', 0x8a5a2bff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("sand", false, false, false, '.', 0xd8c189ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("gravel", false, false, false, ':', 0x9c9b97ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("clay", false, false, false, '.', 0xa8847aff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("mud", false, false, false, ':', 0x6c4c3dff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("snow", false, false, false, '.', 0xf4f8ffff, 0x202830ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("ice", false, false, false, '*', 0xbde8ffff, 0x0e1f2eff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("granite", true, true, false, '#', 0x8b8f94ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("basalt", true, true, false, '#', 0x444b54ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("limestone", true, true, false, '#', 0xc8c1acff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("sandstone", true, true, false, '#', 0xbda26fff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("marble", true, true, false, '#', 0xe2dfd6ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("slate", true, true, false, '#', 0x5f6770ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("obsidian", true, true, false, '#', 0x2a1d3bff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("oak_wood", true, true, false, '+', 0x8d633aff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("pine_wood", true, true, false, '+', 0xa37d53ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("birch_wood", true, true, false, '+', 0xddd2b1ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("maple_wood", true, true, false, '+', 0xb06c3cff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("willow_wood", true, true, false, '+', 0x7c9b6eff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("bamboo", true, false, false, '|', 0x9cc063ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("stone_brick", true, true, false, '#', 0xa7a7a7ff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		error = register_definition_with_next_id("cobblestone", true, true, false, '#', 0x7d7d7dff, 0x000000ff);
		if (error != OK) {
			return error;
		}

		return OK;
	}

private:
	LocalVector<TileDefinition> definitions;
	HashMap<StringName, tile_type_id> name_to_id;
};

} // namespace twim
