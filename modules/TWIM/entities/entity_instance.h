#pragma once

#include "core/math/vector3i.h"
#include "core/string/string_name.h"

#include <cstdint>

namespace twim {

struct EntityInstance {
	int64_t id = 0;
	StringName type_name;
	int64_t type_id = -1;
	Vector3i tile_coord;
};

} // namespace twim

