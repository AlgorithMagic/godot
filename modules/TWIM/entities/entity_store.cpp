#include "entity_store.h"

#include "core/error/error_macros.h"

#include "modules/TWIM/world/world_coord.h"

namespace twim {

void EntityStore::clear() {
	entities.clear();
	coord_to_entity.clear();
	next_entity_id = 1;
}

Error EntityStore::spawn_entity(const StringName &p_type_name, int64_t p_type_id, const Vector3i &p_tile_coord, int64_t &r_entity_id) {
	ERR_FAIL_COND_V_MSG(p_type_name == StringName(), ERR_INVALID_PARAMETER, "Entity type name cannot be empty.");
	ERR_FAIL_COND_V_MSG(!is_packable_world_coord(p_tile_coord), ERR_INVALID_PARAMETER, "Entity coordinate is outside supported world coordinate range.");

	const int64_t coord_key = pack_world_coord_key(p_tile_coord);
	ERR_FAIL_COND_V_MSG(coord_to_entity.has(coord_key), ERR_ALREADY_EXISTS, "Another entity is already occupying this tile coordinate.");

	const int64_t entity_id = next_entity_id++;
	EntityInstance instance;
	instance.id = entity_id;
	instance.type_name = p_type_name;
	instance.type_id = p_type_id;
	instance.tile_coord = p_tile_coord;

	entities.insert(entity_id, instance);
	coord_to_entity.insert(coord_key, entity_id);
	r_entity_id = entity_id;
	return OK;
}

Error EntityStore::move_entity(int64_t p_entity_id, const Vector3i &p_new_tile_coord) {
	ERR_FAIL_COND_V_MSG(!is_packable_world_coord(p_new_tile_coord), ERR_INVALID_PARAMETER, "Entity coordinate is outside supported world coordinate range.");

	EntityInstance *instance = entities.getptr(p_entity_id);
	ERR_FAIL_NULL_V_MSG(instance, ERR_DOES_NOT_EXIST, "Entity ID does not exist.");

	const int64_t new_coord_key = pack_world_coord_key(p_new_tile_coord);
	if (coord_to_entity.has(new_coord_key) && coord_to_entity[new_coord_key] != p_entity_id) {
		return ERR_ALREADY_EXISTS;
	}

	const int64_t old_coord_key = pack_world_coord_key(instance->tile_coord);
	coord_to_entity.erase(old_coord_key);
	coord_to_entity.insert(new_coord_key, p_entity_id);
	instance->tile_coord = p_new_tile_coord;
	return OK;
}

Error EntityStore::remove_entity(int64_t p_entity_id) {
	EntityInstance *instance = entities.getptr(p_entity_id);
	ERR_FAIL_NULL_V_MSG(instance, ERR_DOES_NOT_EXIST, "Entity ID does not exist.");

	coord_to_entity.erase(pack_world_coord_key(instance->tile_coord));
	entities.erase(p_entity_id);
	return OK;
}

bool EntityStore::has_entity(int64_t p_entity_id) const {
	return entities.has(p_entity_id);
}

const EntityInstance *EntityStore::get_entity(int64_t p_entity_id) const {
	return entities.getptr(p_entity_id);
}

int64_t EntityStore::get_entity_count() const {
	return entities.size();
}

Vector<EntityInstance> EntityStore::export_entities() const {
	Vector<EntityInstance> out;
	out.resize(entities.size());
	int32_t index = 0;
	for (const KeyValue<int64_t, EntityInstance> &entry : entities) {
		out.write[index++] = entry.value;
	}
	if (index < out.size()) {
		out.resize(index);
	}
	return out;
}

Error EntityStore::import_entities(const Vector<EntityInstance> &p_entities) {
	clear();

	int64_t max_id = 0;
	for (const EntityInstance &instance : p_entities) {
		ERR_FAIL_COND_V_MSG(instance.id <= 0, ERR_INVALID_DATA, "Imported entity ID must be positive.");
		ERR_FAIL_COND_V_MSG(instance.type_name == StringName(), ERR_INVALID_DATA, "Imported entity type name cannot be empty.");
		ERR_FAIL_COND_V_MSG(!is_packable_world_coord(instance.tile_coord), ERR_INVALID_DATA, "Imported entity coordinate is outside supported range.");

		const int64_t coord_key = pack_world_coord_key(instance.tile_coord);
		ERR_FAIL_COND_V_MSG(entities.has(instance.id), ERR_ALREADY_EXISTS, "Duplicate entity ID encountered while importing entities.");
		ERR_FAIL_COND_V_MSG(coord_to_entity.has(coord_key), ERR_ALREADY_EXISTS, "Duplicate entity tile coordinate encountered while importing entities.");

		entities.insert(instance.id, instance);
		coord_to_entity.insert(coord_key, instance.id);
		max_id = instance.id > max_id ? instance.id : max_id;
	}

	next_entity_id = max_id + 1;
	return OK;
}

} // namespace twim


