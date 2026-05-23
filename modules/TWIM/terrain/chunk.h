//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/error/error_list.h"

#include "modules/TWIM/terrain/chunk_coord.h"
#include "modules/TWIM/terrain/tile_cell.h"

#include "core/templates/vector.h"

#include <cstdint>

namespace twim {

class chunk {
public:
	chunk(chunk_coord const p_coord, chunk_metrics const &p_metrics) :
			coord_(p_coord),
			metrics_(p_metrics) {
		if (!metrics_.is_valid()) {
			metrics_ = chunk_metrics();
		}

		cells_.resize(metrics_.get_tile_count());
		clear();
	}

	void clear(tile_type_id const p_fill_type = tile_type_id()) {
		for (tile_cell &cell : cells_) {
			cell = tile_cell(p_fill_type);
		}

		terrain_dirty_ = false;
	}

	chunk_coord get_coord() const {
		return coord_;
	}

	Error set_cell_local(local_tile_coord const p_coord, tile_cell const p_cell) {
		if (!is_valid_local_tile_coord(p_coord, metrics_)) {
			return ERR_INVALID_PARAMETER;
		}

		cells_.write[get_local_tile_index(p_coord, metrics_)] = p_cell;
		terrain_dirty_ = true;
		generation_ += 1;

		return OK;
	}

	tile_cell get_cell_local(local_tile_coord const p_coord) const {
		if (!is_valid_local_tile_coord(p_coord, metrics_)) {
			return tile_cell();
		}

		return cells_[get_local_tile_index(p_coord, metrics_)];
	}

	bool is_terrain_dirty() const {
		return terrain_dirty_;
	}

	void clear_dirty_flags() {
		terrain_dirty_ = false;
	}

	uint32_t get_generation() const {
		return generation_;
	}

	uint32_t get_tile_count() const {
		return static_cast<uint32_t>(cells_.size());
	}

	const chunk_metrics &get_metrics() const {
		return metrics_;
	}

private:
	chunk_coord coord_;
	chunk_metrics metrics_;
	Vector<tile_cell> cells_;

	bool terrain_dirty_ = false;
	uint32_t generation_ = 0;
};

} // namespace twim
