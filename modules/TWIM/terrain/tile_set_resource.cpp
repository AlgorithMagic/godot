#include "tile_set_resource.h"

#include "tile_type_resource.h"

#include "core/object/class_db.h"

void TwimTileSetResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tile_types", "tile_types"), &TwimTileSetResource::set_tile_types);
	ClassDB::bind_method(D_METHOD("get_tile_types"), &TwimTileSetResource::get_tile_types);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "tile_types", PROPERTY_HINT_ARRAY_TYPE, "TwimTileTypeResource"), "set_tile_types", "get_tile_types");
}

void TwimTileSetResource::set_tile_types(const Array &p_tile_types) {
	tile_types = p_tile_types;
}

Array TwimTileSetResource::get_tile_types() const {
	return tile_types;
}

