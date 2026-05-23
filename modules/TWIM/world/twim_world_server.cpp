//
// Created by Kyle Maillet on 2026-05-22.
//

/*	  TWIM World Server Responsibilities
*	_________________________________________
*	- owning the active TWIM world runtime
*	- exposing stable Godot/ClassDB methods
*	- creating/loading/unloading worlds
*	- stepping deterministic simulation
*	- accepting high-level commands
*	- exposing safe read-only debug/tooling queries
*	- producing immutable render/debug snapshots
*	- coordinating save/load requests
*/

#include "twim_world_server.h"

#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"

#include "modules/TWIM/world/world_runtime.h"

TwimWorldServer::TwimWorldServer() = default;

TwimWorldServer::~TwimWorldServer() {
	destroy_runtime();
}

void TwimWorldServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_world", "world_name", "seed"), &TwimWorldServer::create_world);
	ClassDB::bind_method(D_METHOD("unload_world"), &TwimWorldServer::unload_world);

	ClassDB::bind_method(D_METHOD("has_world"), &TwimWorldServer::has_world);

	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &TwimWorldServer::set_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &TwimWorldServer::is_paused);

	ClassDB::bind_method(D_METHOD("tick"), &TwimWorldServer::tick);
	ClassDB::bind_method(D_METHOD("step_ticks", "tick_count"), &TwimWorldServer::step_ticks);
	ClassDB::bind_method(D_METHOD("get_tick_index"), &TwimWorldServer::get_tick_index);

	ClassDB::bind_method(D_METHOD("debug_set_tile_type_id", "tile_coord", "tile_type_id"), &TwimWorldServer::debug_set_tile_type_id);
	ClassDB::bind_method(D_METHOD("get_tile_type_id", "tile_coord"), &TwimWorldServer::get_tile_type_id);
	ClassDB::bind_method(D_METHOD("get_tile_type_id_by_name", "tile_type_name"), &TwimWorldServer::get_tile_type_id_by_name);
	ClassDB::bind_method(D_METHOD("get_tile_type_names"), &TwimWorldServer::get_tile_type_names);
	ClassDB::bind_method(D_METHOD("get_entity_type_names"), &TwimWorldServer::get_entity_type_names);
	ClassDB::bind_method(D_METHOD("get_item_type_names"), &TwimWorldServer::get_item_type_names);
	ClassDB::bind_method(D_METHOD("spawn_entity", "entity_type_name", "tile_coord"), &TwimWorldServer::spawn_entity);
	ClassDB::bind_method(D_METHOD("move_entity", "entity_id", "tile_coord"), &TwimWorldServer::move_entity);
	ClassDB::bind_method(D_METHOD("remove_entity", "entity_id"), &TwimWorldServer::remove_entity);
	ClassDB::bind_method(D_METHOD("get_entity_debug_info", "entity_id"), &TwimWorldServer::get_entity_debug_info);
	ClassDB::bind_method(D_METHOD("get_entity_count"), &TwimWorldServer::get_entity_count);

	ClassDB::bind_method(D_METHOD("save_world_state", "path"), &TwimWorldServer::save_world_state);
	ClassDB::bind_method(D_METHOD("load_world_state", "path"), &TwimWorldServer::load_world_state);

	ClassDB::bind_method(D_METHOD("build_debug_render_snapshot", "origin", "size", "z_layer"), &TwimWorldServer::build_debug_render_snapshot);

	ClassDB::bind_method(D_METHOD("get_world_debug_info"), &TwimWorldServer::get_world_debug_info);
	ClassDB::bind_method(D_METHOD("get_chunk_debug_info", "chunk_coord"), &TwimWorldServer::get_chunk_debug_info);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "paused"), "set_paused", "is_paused");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "world_loaded"), "", "has_world");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_index"), "", "get_tick_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "entity_count"), "", "get_entity_count");
}

Error TwimWorldServer::create_world(const String &p_world_name, int64_t p_seed) {
	ERR_FAIL_COND_V_MSG(p_world_name.is_empty(), ERR_INVALID_PARAMETER, "TWIM world name cannot be empty.");
	ERR_FAIL_COND_V_MSG(p_seed < 0, ERR_INVALID_PARAMETER, "TWIM world seed must be non-negative.");

	destroy_runtime();

	runtime = memnew(twim::WorldRuntime);

	const Error error = runtime->create_world(p_world_name, static_cast<uint64_t>(p_seed));
	if (error != OK) {
		destroy_runtime();
		return error;
	}

	paused = false;
	return OK;
}

void TwimWorldServer::unload_world() {
	destroy_runtime();
	paused = false;
}

bool TwimWorldServer::has_world() const {
	return has_runtime();
}

void TwimWorldServer::set_paused(bool p_paused) {
	paused = p_paused;
}

bool TwimWorldServer::is_paused() const {
	return paused;
}

Error TwimWorldServer::tick() {
	return step_ticks(1);
}

Error TwimWorldServer::step_ticks(int32_t p_tick_count) {
	ERR_FAIL_COND_V_MSG(!has_runtime(), ERR_UNCONFIGURED, "No TWIM world is loaded.");
	ERR_FAIL_COND_V_MSG(p_tick_count < 0, ERR_INVALID_PARAMETER, "Tick count cannot be negative.");

	if (p_tick_count == 0) {
		return OK;
	}

	if (paused) {
		return OK;
	}

	return runtime->step_ticks(static_cast<uint32_t>(p_tick_count));
}

int64_t TwimWorldServer::get_tick_index() const {
	if (!has_runtime()) {
		return 0;
	}

	return static_cast<int64_t>(runtime->get_tick_index());
}

Error TwimWorldServer::debug_set_tile_type_id(Vector3i p_tile_coord, int32_t p_tile_type_id) {
	ERR_FAIL_COND_V_MSG(!has_runtime(), ERR_UNCONFIGURED, "No TWIM world is loaded.");
	ERR_FAIL_COND_V_MSG(p_tile_type_id < 0, ERR_INVALID_PARAMETER, "Tile type ID cannot be negative.");

	return runtime->debug_set_tile_type_id(p_tile_coord, static_cast<uint16_t>(p_tile_type_id));
}

int32_t TwimWorldServer::get_tile_type_id(Vector3i p_tile_coord) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), -1, "No TWIM world is loaded.");

	return runtime->get_tile_type_id(p_tile_coord);
}

int32_t TwimWorldServer::get_tile_type_id_by_name(StringName p_tile_type_name) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), -1, "No TWIM world is loaded.");
	return runtime->get_tile_type_id_by_name(p_tile_type_name);
}

PackedStringArray TwimWorldServer::get_tile_type_names() const {
	if (!has_runtime()) {
		return PackedStringArray();
	}
	return runtime->get_tile_type_names();
}

PackedStringArray TwimWorldServer::get_entity_type_names() const {
	if (!has_runtime()) {
		return PackedStringArray();
	}
	return runtime->get_entity_type_names();
}

PackedStringArray TwimWorldServer::get_item_type_names() const {
	if (!has_runtime()) {
		return PackedStringArray();
	}
	return runtime->get_item_type_names();
}

int64_t TwimWorldServer::spawn_entity(StringName p_entity_type_name, Vector3i p_tile_coord) {
	ERR_FAIL_COND_V_MSG(!has_runtime(), -1, "No TWIM world is loaded.");

	int64_t entity_id = -1;
	const Error error = runtime->spawn_entity(p_entity_type_name, p_tile_coord, entity_id);
	ERR_FAIL_COND_V_MSG(error != OK, -1, "Failed to spawn TWIM entity.");
	return entity_id;
}

Error TwimWorldServer::move_entity(int64_t p_entity_id, Vector3i p_tile_coord) {
	ERR_FAIL_COND_V_MSG(!has_runtime(), ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return runtime->move_entity(p_entity_id, p_tile_coord);
}

Error TwimWorldServer::remove_entity(int64_t p_entity_id) {
	ERR_FAIL_COND_V_MSG(!has_runtime(), ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return runtime->remove_entity(p_entity_id);
}

Dictionary TwimWorldServer::get_entity_debug_info(int64_t p_entity_id) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), Dictionary(), "No TWIM world is loaded.");
	return runtime->get_entity_debug_info(p_entity_id);
}

int64_t TwimWorldServer::get_entity_count() const {
	if (!has_runtime()) {
		return 0;
	}
	return runtime->get_entity_count();
}

Error TwimWorldServer::save_world_state(const String &p_path) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), ERR_UNCONFIGURED, "No TWIM world is loaded.");
	return runtime->save_world_state(p_path);
}

Error TwimWorldServer::load_world_state(const String &p_path) {
	twim::WorldRuntime *loaded_runtime = memnew(twim::WorldRuntime);
	const Error error = loaded_runtime->load_world_state(p_path);
	if (error != OK) {
		memdelete(loaded_runtime);
		return error;
	}

	destroy_runtime();
	runtime = loaded_runtime;

	paused = false;
	return OK;
}

Dictionary TwimWorldServer::build_debug_render_snapshot(Vector3i p_origin, Vector2i p_size, int32_t p_z_layer) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), Dictionary(), "No TWIM world is loaded.");
	ERR_FAIL_COND_V_MSG(p_size.x <= 0 || p_size.y <= 0, Dictionary(), "Snapshot size must be positive.");
	ERR_FAIL_COND_V_MSG(p_z_layer < 0, Dictionary(), "Snapshot z-layer cannot be negative.");

	return runtime->build_debug_render_snapshot(p_origin, p_size, p_z_layer);
}

Dictionary TwimWorldServer::get_world_debug_info() const {
	if (!has_runtime()) {
		Dictionary info;
		info["world_loaded"] = false;
		info["paused"] = paused;
		info["tick_index"] = 0;
		return info;
	}

	Dictionary info = runtime->get_world_debug_info();
	info["world_loaded"] = true;
	info["paused"] = paused;
	info["tick_index"] = get_tick_index();
	return info;
}

Dictionary TwimWorldServer::get_chunk_debug_info(Vector3i p_chunk_coord) const {
	ERR_FAIL_COND_V_MSG(!has_runtime(), Dictionary(), "No TWIM world is loaded.");

	return runtime->get_chunk_debug_info(p_chunk_coord);
}

void TwimWorldServer::destroy_runtime() {
	if (runtime == nullptr) {
		return;
	}

	memdelete(runtime);
	runtime = nullptr;
}

bool TwimWorldServer::has_runtime() const {
	return runtime != nullptr;
}
