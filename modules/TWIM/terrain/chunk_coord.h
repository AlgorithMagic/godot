//
// Created by busin on 2026-05-22.
//

#pragma once

#include "core/math/vector3i.h"

#include <cstdint>

namespace twim {

struct chunk_metrics {
	static constexpr int32_t DEFAULT_SIZE_X = 32;
	static constexpr int32_t DEFAULT_SIZE_Y = 32;
	static constexpr int32_t DEFAULT_SIZE_Z = 16;

	int32_t size_x = DEFAULT_SIZE_X;
	int32_t size_y = DEFAULT_SIZE_Y;
	int32_t size_z = DEFAULT_SIZE_Z;

	bool is_valid() const {
		return size_x > 0 && size_y > 0 && size_z > 0;
	}

	int32_t get_tile_count() const {
		return size_x * size_y * size_z;
	}
};

static constexpr int32_t chunk_coord_pack_bits = 21;
static constexpr int32_t chunk_coord_pack_bias = 1 << (chunk_coord_pack_bits - 1);
static constexpr int32_t chunk_coord_pack_min = -chunk_coord_pack_bias;
static constexpr int32_t chunk_coord_pack_max = chunk_coord_pack_bias - 1;
static constexpr uint64_t chunk_coord_pack_mask = (static_cast<uint64_t>(1) << chunk_coord_pack_bits) - 1;

struct tile_coord {
	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;

	static tile_coord from_vector3_i(Vector3i const p_coord) {
		return tile_coord{ p_coord.x, p_coord.y, p_coord.z };
	}
};

struct chunk_coord {
	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;

	Vector3i to_vector3_i() const {
		return Vector3i(x, y, z);
	}
};

struct local_tile_coord {
	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;
};

inline int32_t floor_divide(int32_t const p_value, int32_t const p_divisor) {
	const int32_t quotient = p_value / p_divisor;

	if (const int32_t remainder = p_value % p_divisor; remainder != 0 && ((remainder < 0) != (p_divisor < 0))) {
		return quotient - 1;
	}

	return quotient;
}

inline int32_t positive_modulo(int32_t const p_value, int32_t const p_divisor) {
	const int32_t result = p_value % p_divisor;
	return result < 0 ? result + p_divisor : result;
}

inline chunk_coord tile_to_chunk_coord(tile_coord const p_coord) {
	return chunk_coord{
		floor_divide(p_coord.x, chunk_metrics::DEFAULT_SIZE_X),
		floor_divide(p_coord.y, chunk_metrics::DEFAULT_SIZE_Y),
		floor_divide(p_coord.z, chunk_metrics::DEFAULT_SIZE_Z),
	};
}

inline chunk_coord tile_to_chunk_coord(tile_coord const p_coord, chunk_metrics const &p_metrics) {
	return chunk_coord{
		floor_divide(p_coord.x, p_metrics.size_x),
		floor_divide(p_coord.y, p_metrics.size_y),
		floor_divide(p_coord.z, p_metrics.size_z),
	};
}

inline local_tile_coord tile_to_local_coord(tile_coord const p_coord) {
	return local_tile_coord{
		positive_modulo(p_coord.x, chunk_metrics::DEFAULT_SIZE_X),
		positive_modulo(p_coord.y, chunk_metrics::DEFAULT_SIZE_Y),
		positive_modulo(p_coord.z, chunk_metrics::DEFAULT_SIZE_Z),
	};
}

inline local_tile_coord tile_to_local_coord(tile_coord const p_coord, chunk_metrics const &p_metrics) {
	return local_tile_coord{
		positive_modulo(p_coord.x, p_metrics.size_x),
		positive_modulo(p_coord.y, p_metrics.size_y),
		positive_modulo(p_coord.z, p_metrics.size_z),
	};
}

inline bool is_valid_local_tile_coord(local_tile_coord const p_coord) {
	return p_coord.x >= 0 && p_coord.x < chunk_metrics::DEFAULT_SIZE_X &&
			p_coord.y >= 0 && p_coord.y < chunk_metrics::DEFAULT_SIZE_Y &&
			p_coord.z >= 0 && p_coord.z < chunk_metrics::DEFAULT_SIZE_Z;
}

inline bool is_valid_local_tile_coord(local_tile_coord const p_coord, chunk_metrics const &p_metrics) {
	return p_coord.x >= 0 && p_coord.x < p_metrics.size_x &&
			p_coord.y >= 0 && p_coord.y < p_metrics.size_y &&
			p_coord.z >= 0 && p_coord.z < p_metrics.size_z;
}

inline bool is_packable_chunk_coord(chunk_coord const p_coord) {
	return p_coord.x >= chunk_coord_pack_min && p_coord.x <= chunk_coord_pack_max &&
			p_coord.y >= chunk_coord_pack_min && p_coord.y <= chunk_coord_pack_max &&
			p_coord.z >= chunk_coord_pack_min && p_coord.z <= chunk_coord_pack_max;
}

inline int64_t pack_chunk_coord_key(chunk_coord const p_coord) {
	const uint64_t packed_x = static_cast<uint64_t>(p_coord.x + chunk_coord_pack_bias) & chunk_coord_pack_mask;
	const uint64_t packed_y = static_cast<uint64_t>(p_coord.y + chunk_coord_pack_bias) & chunk_coord_pack_mask;
	const uint64_t packed_z = static_cast<uint64_t>(p_coord.z + chunk_coord_pack_bias) & chunk_coord_pack_mask;

	return static_cast<int64_t>((packed_x << 42) | (packed_y << 21) | packed_z);
}

inline uint32_t get_local_tile_index(local_tile_coord const p_coord, chunk_metrics const &p_metrics) {
	return static_cast<uint32_t>(
			p_coord.x +
			(p_coord.y * p_metrics.size_x) +
			(p_coord.z * p_metrics.size_x * p_metrics.size_y));
}

} // namespace twim
