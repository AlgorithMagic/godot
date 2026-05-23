#include "item_catalog_resource.h"

#include "item_type_resource.h"

#include "core/object/class_db.h"

void TwimItemCatalogResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item_types", "item_types"), &TwimItemCatalogResource::set_item_types);
	ClassDB::bind_method(D_METHOD("get_item_types"), &TwimItemCatalogResource::get_item_types);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "item_types", PROPERTY_HINT_ARRAY_TYPE, "TwimItemTypeResource"), "set_item_types", "get_item_types");
}

void TwimItemCatalogResource::set_item_types(const Array &p_item_types) {
	item_types = p_item_types;
}

Array TwimItemCatalogResource::get_item_types() const {
	return item_types;
}

