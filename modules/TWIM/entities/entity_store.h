#pragma once

#include "core/error/error_list.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"

#include "modules/TWIM/entities/entity_instance.h"

namespace twim {

class EntityStore {
public:
	void clear();

	Error spawn_entity(const StringName &p_type_name, int64_t p_type_id, const Vector3i &p_tile_coord, int64_t &r_entity_id);
	Error move_entity(int64_t p_entity_id, const Vector3i &p_new_tile_coord);
	Error remove_entity(int64_t p_entity_id);

	bool has_entity(int64_t p_entity_id) const;
	const EntityInstance *get_entity(int64_t p_entity_id) const;
	int64_t get_entity_count() const;

	Vector<EntityInstance> export_entities() const;
	Error import_entities(const Vector<EntityInstance> &p_entities);

private:
	HashMap<int64_t, EntityInstance> entities;
	HashMap<int64_t, int64_t> coord_to_entity;
	int64_t next_entity_id = 1;
};

} // namespace twim

