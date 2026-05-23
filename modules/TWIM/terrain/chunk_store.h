//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/error/error_list.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"

#include "modules/TWIM/terrain/chunk.h"
#include "modules/TWIM/terrain/tile_id.h"

#include <cstdint>

namespace twim {

class chunk_store {
public:
	explicit chunk_store(chunk_metrics p_metrics = chunk_metrics()) :
			metrics_(p_metrics) {}
	~chunk_store();

	chunk_store(const chunk_store &) = delete;
	chunk_store &operator=(const chunk_store &) = delete;

	void clear();

	bool has_chunk(chunk_coord p_coord) const;

	Error create_chunk(chunk_coord p_coord);
	Error ensure_chunk(chunk_coord p_coord);

	chunk *get_chunk(chunk_coord p_coord);
	const chunk *get_chunk(chunk_coord p_coord) const;

	void set_chunk_metrics(chunk_metrics p_metrics);
	const chunk_metrics &get_chunk_metrics() const;

	Error set_tile_type_id(tile_coord p_coord, tile_type_id p_tile_type_id);
	tile_type_id get_tile_type_id(tile_coord p_coord) const;

	uint32_t get_loaded_chunk_count() const;
	Vector<chunk_coord> get_loaded_chunk_coords() const;

	Dictionary get_chunk_debug_info(chunk_coord p_coord) const;

private:
	HashMap<int64_t, chunk *> chunks_;
	chunk_metrics metrics_;

	static Error validate_chunk_coord(chunk_coord p_coord);
};

} // namespace twim
