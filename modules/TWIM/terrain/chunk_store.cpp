//
// Created by busin on 2026-05-23.
//

#include "chunk_store.h"
#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"

namespace twim {
namespace {

const StringName DEBUG_KEY_AVAILABLE("available");
const StringName DEBUG_KEY_CHUNK_COORD("chunk_coord");
const StringName DEBUG_KEY_GENERATION("generation");
const StringName DEBUG_KEY_LOADED("loaded");
const StringName DEBUG_KEY_REASON("reason");
const StringName DEBUG_KEY_TERRAIN_DIRTY("terrain_dirty");
const StringName DEBUG_KEY_TILE_COUNT("tile_count");

const String DEBUG_REASON_COORD_OUT_OF_RANGE = "Chunk coordinate is outside the supported packed coordinate range.";

} // namespace

chunk_store::~chunk_store() {
	clear();
}

void chunk_store::clear() {
	for (KeyValue<int64_t, chunk *> &entry : chunks_) {
		memdelete(entry.value);
	}

	chunks_.clear();
}

bool chunk_store::has_chunk(chunk_coord p_coord) const {
	if (validate_chunk_coord(p_coord) != OK) {
		return false;
	}

	return chunks_.has(pack_chunk_coord_key(p_coord));
}

Error chunk_store::create_chunk(chunk_coord p_coord) {
	const Error validation_error = validate_chunk_coord(p_coord);
	if (validation_error != OK) {
		return validation_error;
	}

	const int64_t key = pack_chunk_coord_key(p_coord);

	if (chunks_.has(key)) {
		return OK;
	}

	chunks_.insert(key, memnew(chunk(p_coord, metrics_)));
	return OK;
}

Error chunk_store::ensure_chunk(chunk_coord p_coord) {
	return create_chunk(p_coord);
}

chunk *chunk_store::get_chunk(chunk_coord p_coord) {
	if (validate_chunk_coord(p_coord) != OK) {
		return nullptr;
	}

	chunk **chunk_ptr = chunks_.getptr(pack_chunk_coord_key(p_coord));
	return chunk_ptr != nullptr ? *chunk_ptr : nullptr;
}

const chunk *chunk_store::get_chunk(chunk_coord p_coord) const {
	if (validate_chunk_coord(p_coord) != OK) {
		return nullptr;
	}

	chunk *const *chunk_ptr = chunks_.getptr(pack_chunk_coord_key(p_coord));
	return chunk_ptr != nullptr ? *chunk_ptr : nullptr;
}

void chunk_store::set_chunk_metrics(chunk_metrics p_metrics) {
	ERR_FAIL_COND_MSG(!p_metrics.is_valid(), "Chunk metrics must be strictly positive.");

	if (chunks_.is_empty()) {
		metrics_ = p_metrics;
		return;
	}

	clear();
	metrics_ = p_metrics;
}

const chunk_metrics &chunk_store::get_chunk_metrics() const {
	return metrics_;
}

Error chunk_store::set_tile_type_id(tile_coord p_coord, tile_type_id p_tile_type_id) {
	const chunk_coord target_chunk_coord = tile_to_chunk_coord(p_coord, metrics_);
	const local_tile_coord local_coord = tile_to_local_coord(p_coord, metrics_);

	const Error ensure_error = ensure_chunk(target_chunk_coord);
	if (ensure_error != OK) {
		return ensure_error;
	}

	chunk *target_chunk = get_chunk(target_chunk_coord);
	if (target_chunk == nullptr) {
		return ERR_BUG;
	}

	return target_chunk->set_cell_local(local_coord, tile_cell(p_tile_type_id));
}

tile_type_id chunk_store::get_tile_type_id(tile_coord p_coord) const {
	const chunk_coord target_chunk_coord = tile_to_chunk_coord(p_coord, metrics_);
	const local_tile_coord local_coord = tile_to_local_coord(p_coord, metrics_);

	const chunk *target_chunk = get_chunk(target_chunk_coord);
	if (target_chunk == nullptr) {
		return tile_type_id();
	}

	return target_chunk->get_cell_local(local_coord).type_id;
}

uint32_t chunk_store::get_loaded_chunk_count() const {
	return static_cast<uint32_t>(chunks_.size());
}

Vector<chunk_coord> chunk_store::get_loaded_chunk_coords() const {
	Vector<chunk_coord> coords;
	coords.resize(chunks_.size());

	int32_t index = 0;
	for (const KeyValue<int64_t, chunk *> &entry : chunks_) {
		const chunk *chunk_ptr = entry.value;
		if (chunk_ptr == nullptr) {
			continue;
		}

		coords.write[index++] = chunk_ptr->get_coord();
	}

	if (index < coords.size()) {
		coords.resize(index);
	}

	return coords;
}

Dictionary chunk_store::get_chunk_debug_info(chunk_coord p_coord) const {
	Dictionary info;
	info[DEBUG_KEY_CHUNK_COORD] = p_coord.to_vector3_i();

	if (validate_chunk_coord(p_coord) != OK) {
		info[DEBUG_KEY_AVAILABLE] = false;
		info[DEBUG_KEY_REASON] = DEBUG_REASON_COORD_OUT_OF_RANGE;
		return info;
	}

	const chunk *target_chunk = get_chunk(p_coord);

	if (target_chunk == nullptr) {
		info[DEBUG_KEY_AVAILABLE] = false;
		info[DEBUG_KEY_LOADED] = false;
		info[DEBUG_KEY_TILE_COUNT] = 0;
		info[DEBUG_KEY_GENERATION] = 0;
		info[DEBUG_KEY_TERRAIN_DIRTY] = false;
		return info;
	}

	info[DEBUG_KEY_AVAILABLE] = true;
	info[DEBUG_KEY_LOADED] = true;
	info[DEBUG_KEY_TILE_COUNT] = static_cast<int64_t>(target_chunk->get_tile_count());
	info["chunk_size_x"] = target_chunk->get_metrics().size_x;
	info["chunk_size_y"] = target_chunk->get_metrics().size_y;
	info["chunk_size_z"] = target_chunk->get_metrics().size_z;
	info[DEBUG_KEY_GENERATION] = static_cast<int64_t>(target_chunk->get_generation());
	info[DEBUG_KEY_TERRAIN_DIRTY] = target_chunk->is_terrain_dirty();

	return info;
}

Error chunk_store::validate_chunk_coord(chunk_coord p_coord) {
	if (!is_packable_chunk_coord(p_coord)) {
		return ERR_INVALID_PARAMETER;
	}

	return OK;
}

} // namespace twim
