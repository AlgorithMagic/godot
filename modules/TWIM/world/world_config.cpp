#include "world_config.h"

#include "core/config/project_settings.h"

namespace {
const char *TWIM_SETTING_TICK_RATE = "twim/world/tick_rate_hz";
const char *TWIM_SETTING_CHUNK_SIZE_X = "twim/world/chunk_size_x";
const char *TWIM_SETTING_CHUNK_SIZE_Y = "twim/world/chunk_size_y";
const char *TWIM_SETTING_CHUNK_SIZE_Z = "twim/world/chunk_size_z";
const char *TWIM_SETTING_TILE_SET_PATH = "twim/resources/tile_set";
const char *TWIM_SETTING_ENTITY_CATALOG_PATH = "twim/resources/entity_catalog";
const char *TWIM_SETTING_ITEM_CATALOG_PATH = "twim/resources/item_catalog";
const char *TWIM_SETTING_WORLD_PROFILE_PATH = "twim/resources/world_profile";
const char *TWIM_SETTING_GLYPH_FONT_PATH = "twim/rendering/glyph_font";
const char *TWIM_SETTING_GLYPH_FONT_SIZE = "twim/rendering/glyph_font_size";
} // namespace

namespace twim {

WorldConfig WorldConfig::from_project_settings(const String &p_world_name, uint64_t p_seed) {
	WorldConfig config(p_world_name, p_seed);
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	if (project_settings == nullptr) {
		return config;
	}

	if (project_settings->has_setting(TWIM_SETTING_TICK_RATE)) {
		config.tick_rate_hz = static_cast<uint32_t>(project_settings->get_setting(TWIM_SETTING_TICK_RATE));
	}

	if (project_settings->has_setting(TWIM_SETTING_CHUNK_SIZE_X)) {
		config.metrics.size_x = static_cast<int32_t>(project_settings->get_setting(TWIM_SETTING_CHUNK_SIZE_X));
	}

	if (project_settings->has_setting(TWIM_SETTING_CHUNK_SIZE_Y)) {
		config.metrics.size_y = static_cast<int32_t>(project_settings->get_setting(TWIM_SETTING_CHUNK_SIZE_Y));
	}

	if (project_settings->has_setting(TWIM_SETTING_CHUNK_SIZE_Z)) {
		config.metrics.size_z = static_cast<int32_t>(project_settings->get_setting(TWIM_SETTING_CHUNK_SIZE_Z));
	}

	if (project_settings->has_setting(TWIM_SETTING_TILE_SET_PATH)) {
		config.tile_set_resource_path = project_settings->get_setting(TWIM_SETTING_TILE_SET_PATH);
	}

	if (project_settings->has_setting(TWIM_SETTING_ENTITY_CATALOG_PATH)) {
		config.entity_catalog_resource_path = project_settings->get_setting(TWIM_SETTING_ENTITY_CATALOG_PATH);
	}

	if (project_settings->has_setting(TWIM_SETTING_ITEM_CATALOG_PATH)) {
		config.item_catalog_resource_path = project_settings->get_setting(TWIM_SETTING_ITEM_CATALOG_PATH);
	}

	if (project_settings->has_setting(TWIM_SETTING_WORLD_PROFILE_PATH)) {
		config.world_profile_resource_path = project_settings->get_setting(TWIM_SETTING_WORLD_PROFILE_PATH);
	}

	if (project_settings->has_setting(TWIM_SETTING_GLYPH_FONT_PATH)) {
		config.glyph_font_resource_path = project_settings->get_setting(TWIM_SETTING_GLYPH_FONT_PATH);
	}

	if (project_settings->has_setting(TWIM_SETTING_GLYPH_FONT_SIZE)) {
		config.glyph_font_size = static_cast<int32_t>(project_settings->get_setting(TWIM_SETTING_GLYPH_FONT_SIZE));
	}

	return config;
}

} // namespace twim

