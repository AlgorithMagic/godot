#pragma once

#include "core/math/vector3i.h"

#include <cstdint>

namespace twim {

static constexpr int32_t world_coord_pack_bits = 21;
static constexpr int32_t world_coord_pack_bias = 1 << (world_coord_pack_bits - 1);
static constexpr int32_t world_coord_pack_min = -world_coord_pack_bias;
static constexpr int32_t world_coord_pack_max = world_coord_pack_bias - 1;
static constexpr uint64_t world_coord_pack_mask = (static_cast<uint64_t>(1) << world_coord_pack_bits) - 1;

inline bool is_packable_world_coord(const Vector3i &p_coord) {
	return p_coord.x >= world_coord_pack_min && p_coord.x <= world_coord_pack_max &&
			p_coord.y >= world_coord_pack_min && p_coord.y <= world_coord_pack_max &&
			p_coord.z >= world_coord_pack_min && p_coord.z <= world_coord_pack_max;
}

inline int64_t pack_world_coord_key(const Vector3i &p_coord) {
	const uint64_t packed_x = static_cast<uint64_t>(p_coord.x + world_coord_pack_bias) & world_coord_pack_mask;
	const uint64_t packed_y = static_cast<uint64_t>(p_coord.y + world_coord_pack_bias) & world_coord_pack_mask;
	const uint64_t packed_z = static_cast<uint64_t>(p_coord.z + world_coord_pack_bias) & world_coord_pack_mask;
	return static_cast<int64_t>((packed_x << 42) | (packed_y << 21) | packed_z);
}

} // namespace twim
