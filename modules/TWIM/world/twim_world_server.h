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

#ifndef TWIM_WORLD_SERVER_H
#define TWIM_WORLD_SERVER_H

#include "core/error/error_list.h"
#include "core/math/vector2i.h"
#include "core/math/vector3i.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

namespace twim {
class WorldRuntime;
}

class TwimWorldServer : public Object {
	GDCLASS(TwimWorldServer, Object);

public:
	TwimWorldServer();
	~TwimWorldServer() override;

	Error create_world(const String &p_world_name, int64_t p_seed);

	void unload_world();

	bool has_world() const;

	void set_paused(bool p_paused);
	bool is_paused() const;

	Error tick();
	Error step_ticks(int32_t p_tick_count);
	int64_t get_tick_index() const;

	Error debug_set_tile_type_id(Vector3i p_tile_coord, int32_t p_tile_type_id);
	int32_t get_tile_type_id(Vector3i p_tile_coord) const;
	int32_t get_tile_type_id_by_name(StringName p_tile_type_name) const;
	PackedStringArray get_tile_type_names() const;
	PackedStringArray get_entity_type_names() const;
	PackedStringArray get_item_type_names() const;

	int64_t spawn_entity(StringName p_entity_type_name, Vector3i p_tile_coord);
	Error move_entity(int64_t p_entity_id, Vector3i p_tile_coord);
	Error remove_entity(int64_t p_entity_id);
	Dictionary get_entity_debug_info(int64_t p_entity_id) const;
	int64_t get_entity_count() const;

	Error save_world_state(const String &p_path) const;
	Error load_world_state(const String &p_path);

	Dictionary build_debug_render_snapshot(Vector3i p_origin, Vector2i p_size, int32_t p_z_layer) const;

	Dictionary get_world_debug_info() const;
	Dictionary get_chunk_debug_info(Vector3i p_chunk_coord) const;

protected:
	static void _bind_methods();

private:
	twim::WorldRuntime *runtime = nullptr;
	bool paused = false;

	void destroy_runtime();
	bool has_runtime() const;
};

#endif // TWIM_WORLD_SERVER_H
