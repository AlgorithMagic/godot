#include "entity_catalog_resource.h"

#include "entity_type_resource.h"

#include "core/object/class_db.h"

void TwimEntityCatalogResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_entity_types", "entity_types"), &TwimEntityCatalogResource::set_entity_types);
	ClassDB::bind_method(D_METHOD("get_entity_types"), &TwimEntityCatalogResource::get_entity_types);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entity_types", PROPERTY_HINT_ARRAY_TYPE, "TwimEntityTypeResource"), "set_entity_types", "get_entity_types");
}

void TwimEntityCatalogResource::set_entity_types(const Array &p_entity_types) {
	entity_types = p_entity_types;
}

Array TwimEntityCatalogResource::get_entity_types() const {
	return entity_types;
}

