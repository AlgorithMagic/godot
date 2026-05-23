#include "tests/test_macros.h"

TEST_FORCE_LINK(test_twim_world_runtime)

#include "core/config/project_settings.h"
#include "core/io/resource_saver.h"

#include "modules/TWIM/entities/entity_catalog_resource.h"
#include "modules/TWIM/entities/entity_store.h"
#include "modules/TWIM/entities/entity_type_resource.h"
#include "modules/TWIM/items/item_catalog_resource.h"
#include "modules/TWIM/items/item_type_resource.h"
#include "modules/TWIM/persistence/world_state_persistence.h"
#include "modules/TWIM/simulation/twim_simulation_scheduler.h"
#include "modules/TWIM/terrain/chunk_store.h"
#include "modules/TWIM/terrain/tile_registry.h"
#include "modules/TWIM/terrain/tile_type_resource.h"
#include "modules/TWIM/world/world_config.h"
#include "modules/TWIM/world/world_runtime.h"

namespace TestTwim {

TEST_CASE("[TWIM] Builtin tile registry includes expanded material set") {
	twim::TileDefinitionRegistry registry;
	CHECK_EQ(registry.register_builtin_definitions(), OK);

	CHECK(registry.has_definition_name("air"));
	CHECK(registry.has_definition_name("water"));
	CHECK(registry.has_definition_name("oak_wood"));
	CHECK(registry.has_definition_name("granite"));
	CHECK(registry.has_definition_name("obsidian"));
	CHECK(registry.get_definition_count() >= 20);
}

TEST_CASE("[TWIM] Chunk store respects custom chunk metrics") {
	twim::chunk_metrics metrics;
	metrics.size_x = 8;
	metrics.size_y = 8;
	metrics.size_z = 4;

	twim::chunk_store store(metrics);
	CHECK_EQ(store.create_chunk(twim::chunk_coord{ 0, 0, 0 }), OK);

	twim::tile_coord tile{ 7, 7, 3 };
	CHECK_EQ(store.set_tile_type_id(tile, twim::tile_type_id(3)), OK);
	CHECK_EQ(store.get_tile_type_id(tile).value, 3);

	const twim::chunk *loaded = store.get_chunk(twim::chunk_coord{ 0, 0, 0 });
	REQUIRE(loaded != nullptr);
	CHECK_EQ(loaded->get_tile_count(), 8 * 8 * 4);
}

TEST_CASE("[TWIM] World config validates chunk metrics") {
	twim::WorldConfig config("test", 42);
	config.metrics.size_z = 0;
	CHECK_EQ(config.validate(), ERR_INVALID_PARAMETER);
}

TEST_CASE("[TWIM] World runtime creates and steps world") {
	twim::WorldRuntime runtime;
	CHECK_EQ(runtime.create_world("test_world", 1234), OK);
	CHECK(runtime.has_world());
	CHECK_EQ(runtime.step_ticks(8), OK);
	CHECK_EQ(runtime.get_tick_index(), 8);

	Dictionary world_info = runtime.get_world_debug_info();
	CHECK(world_info.has("chunk_size_x"));
	CHECK(world_info.has("entity_definition_count"));
}

TEST_CASE("[TWIM] Runtime exposes tile/entity type names for editor workflows") {
	twim::WorldConfig config("type_world", 1001);

	Ref<TwimEntityTypeResource> entity_type(memnew(TwimEntityTypeResource));
	entity_type->set_entity_name("citizen");
	entity_type->set_glyph("@");
	Array entities_array;
	entities_array.push_back(entity_type);
	Ref<TwimEntityCatalogResource> catalog(memnew(TwimEntityCatalogResource));
	catalog->set_entity_types(entities_array);

	const String catalog_path = "user://twim_type_catalog.tres";
	CHECK_EQ(ResourceSaver::save(catalog, catalog_path), OK);
	config.entity_catalog_resource_path = catalog_path;

	twim::WorldRuntime runtime;
	CHECK_EQ(runtime.create_world(config), OK);

	const PackedStringArray tile_names = runtime.get_tile_type_names();
	CHECK(tile_names.has("air"));
	CHECK(tile_names.has("water"));
	CHECK(runtime.get_tile_type_id_by_name("air") >= 0);

	const PackedStringArray entity_names = runtime.get_entity_type_names();
	CHECK(entity_names.has("citizen"));
}

TEST_CASE("[TWIM] Runtime exposes item type names for editor workflows") {
	twim::WorldConfig config("item_world", 1002);

	Ref<TwimItemTypeResource> item_type(memnew(TwimItemTypeResource));
	item_type->set_item_name("rope");
	item_type->set_glyph(String::chr(static_cast<char32_t>(0x2693)));
	item_type->set_unit_weight(2.5);
	Array items_array;
	items_array.push_back(item_type);
	Ref<TwimItemCatalogResource> catalog(memnew(TwimItemCatalogResource));
	catalog->set_item_types(items_array);

	const String catalog_path = "user://twim_item_catalog.tres";
	CHECK_EQ(ResourceSaver::save(catalog, catalog_path), OK);
	config.item_catalog_resource_path = catalog_path;

	twim::WorldRuntime runtime;
	CHECK_EQ(runtime.create_world(config), OK);

	const PackedStringArray item_names = runtime.get_item_type_names();
	CHECK(item_names.has("rope"));
}

TEST_CASE("[TWIM] Entity store supports spawn move and remove") {
	twim::EntityStore entities;
	int64_t entity_id = -1;
	CHECK_EQ(entities.spawn_entity("citizen", 1, Vector3i(1, 2, 3), entity_id), OK);
	CHECK(entity_id > 0);
	CHECK_EQ(entities.get_entity_count(), 1);

	CHECK_EQ(entities.move_entity(entity_id, Vector3i(2, 2, 3)), OK);
	const twim::EntityInstance *instance = entities.get_entity(entity_id);
	REQUIRE(instance != nullptr);
	CHECK_EQ(instance->tile_coord, Vector3i(2, 2, 3));

	CHECK_EQ(entities.remove_entity(entity_id), OK);
	CHECK_EQ(entities.get_entity_count(), 0);
}

TEST_CASE("[TWIM] Persistence roundtrip restores chunks and entities") {
	twim::WorldConfig config("persisted_world", 77);
	config.tick_rate_hz = 60;
	config.metrics.size_x = 4;
	config.metrics.size_y = 4;
	config.metrics.size_z = 4;

	twim::chunk_store source_chunks(config.metrics);
	CHECK_EQ(source_chunks.create_chunk(twim::chunk_coord{ 0, 0, 0 }), OK);
	CHECK_EQ(source_chunks.set_tile_type_id(twim::tile_coord{ 1, 1, 1 }, twim::tile_type_id(twim::TileDefinitionRegistry::TILE_WATER)), OK);

	Vector<twim::EntityInstance> source_entities;
	source_entities.resize(1);
	source_entities.write[0].id = 1;
	source_entities.write[0].type_name = "citizen";
	source_entities.write[0].type_id = 10;
	source_entities.write[0].tile_coord = Vector3i(2, 2, 1);

	const String save_path = ProjectSettings::get_singleton()->globalize_path("user://twim_state_roundtrip.twim");
	CHECK_EQ(twim::WorldStatePersistence::save_world_state(save_path, config, 42, source_chunks, source_entities), OK);

	twim::WorldConfig loaded_config;
	uint64_t loaded_tick_index = 0;
	twim::chunk_store loaded_chunks;
	Vector<twim::EntityInstance> loaded_entities;
	CHECK_EQ(twim::WorldStatePersistence::load_world_state(save_path, loaded_config, loaded_tick_index, loaded_chunks, loaded_entities), OK);

	CHECK_EQ(loaded_config.world_name, config.world_name);
	CHECK_EQ(loaded_tick_index, 42);
	CHECK_EQ(loaded_chunks.get_tile_type_id(twim::tile_coord{ 1, 1, 1 }).value, twim::TileDefinitionRegistry::TILE_WATER);
	CHECK_EQ(loaded_entities.size(), 1);
	CHECK_EQ(loaded_entities[0].type_name, StringName("citizen"));
}

TEST_CASE("[TWIM] Scheduler applies liquid settling in chunk") {
	twim::chunk_metrics metrics;
	metrics.size_x = 4;
	metrics.size_y = 4;
	metrics.size_z = 4;

	twim::chunk_store chunks(metrics);
	CHECK_EQ(chunks.create_chunk(twim::chunk_coord{ 0, 0, 0 }), OK);

	twim::TileDefinitionRegistry tile_definitions;
	CHECK_EQ(tile_definitions.register_builtin_definitions(), OK);

	const twim::tile_coord from{ 1, 1, 2 };
	const twim::tile_coord to{ 1, 1, 1 };
	CHECK_EQ(chunks.set_tile_type_id(from, twim::tile_type_id(twim::TileDefinitionRegistry::TILE_WATER)), OK);
	CHECK_EQ(chunks.set_tile_type_id(to, twim::tile_type_id(twim::TileDefinitionRegistry::TILE_AIR)), OK);

	twim::TwimSimulationScheduler scheduler;
	CHECK_EQ(scheduler.run_step(chunks, tile_definitions, 1), OK);
	CHECK_EQ(chunks.get_tile_type_id(from).value, twim::TileDefinitionRegistry::TILE_AIR);
	CHECK_EQ(chunks.get_tile_type_id(to).value, twim::TileDefinitionRegistry::TILE_WATER);
}

TEST_CASE("[TWIM] Tile and entity resources support Unicode glyph input") {
	Ref<TwimTileTypeResource> tile_type(memnew(TwimTileTypeResource));
	const String tile_glyph = String::chr(static_cast<char32_t>(0x1F702));
	tile_type->set_glyph(tile_glyph);
	CHECK_EQ(tile_type->get_glyph(), tile_glyph);
	CHECK_EQ(tile_type->get_glyph_id(), 0x1F702);

	Ref<TwimEntityTypeResource> entity_type(memnew(TwimEntityTypeResource));
	const String entity_glyph = String::chr(static_cast<char32_t>(0x1F9DD));
	entity_type->set_glyph(entity_glyph);
	CHECK_EQ(entity_type->get_glyph(), entity_glyph);
	CHECK_EQ(entity_type->get_glyph_id(), 0x1F9DD);

	Ref<TwimItemTypeResource> item_type(memnew(TwimItemTypeResource));
	const String item_glyph = String::chr(static_cast<char32_t>(0x2692));
	item_type->set_glyph(item_glyph);
	CHECK_EQ(item_type->get_glyph(), item_glyph);
	CHECK_EQ(item_type->get_glyph_id(), 0x2692);
}

TEST_CASE("[TWIM] Persistence roundtrip stores glyph font settings") {
	twim::WorldConfig config("font_world", 13);
	config.glyph_font_resource_path = "res://fonts/unicode_font.ttf";
	config.glyph_font_size = 22;
	config.item_catalog_resource_path = "res://data/twim_items.tres";

	twim::chunk_store source_chunks(config.metrics);
	CHECK_EQ(source_chunks.create_chunk(twim::chunk_coord{ 0, 0, 0 }), OK);

	Vector<twim::EntityInstance> source_entities;
	const String save_path = ProjectSettings::get_singleton()->globalize_path("user://twim_font_roundtrip.twim");
	CHECK_EQ(twim::WorldStatePersistence::save_world_state(save_path, config, 7, source_chunks, source_entities), OK);

	twim::WorldConfig loaded_config;
	uint64_t loaded_tick_index = 0;
	twim::chunk_store loaded_chunks;
	Vector<twim::EntityInstance> loaded_entities;
	CHECK_EQ(twim::WorldStatePersistence::load_world_state(save_path, loaded_config, loaded_tick_index, loaded_chunks, loaded_entities), OK);
	CHECK_EQ(loaded_config.glyph_font_resource_path, String("res://fonts/unicode_font.ttf"));
	CHECK_EQ(loaded_config.glyph_font_size, 22);
	CHECK_EQ(loaded_config.item_catalog_resource_path, String("res://data/twim_items.tres"));
}

} // namespace TestTwim

