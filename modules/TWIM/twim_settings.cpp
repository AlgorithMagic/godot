#include "twim_settings.h"

#include "core/config/project_settings.h"

#ifdef TOOLS_ENABLED
#include "editor/settings/editor_settings.h"
#endif

namespace {
const char *TWIM_WORLD_TICK_RATE = "twim/world/tick_rate_hz";
const char *TWIM_WORLD_CHUNK_SIZE_X = "twim/world/chunk_size_x";
const char *TWIM_WORLD_CHUNK_SIZE_Y = "twim/world/chunk_size_y";
const char *TWIM_WORLD_CHUNK_SIZE_Z = "twim/world/chunk_size_z";
const char *TWIM_RES_TILE_SET = "twim/resources/tile_set";
const char *TWIM_RES_ENTITY_CATALOG = "twim/resources/entity_catalog";
const char *TWIM_RES_ITEM_CATALOG = "twim/resources/item_catalog";
const char *TWIM_RES_WORLD_PROFILE = "twim/resources/world_profile";
const char *TWIM_RENDER_GLYPH_FONT = "twim/rendering/glyph_font";
const char *TWIM_RENDER_GLYPH_FONT_SIZE = "twim/rendering/glyph_font_size";
} // namespace

namespace twim {

void register_project_settings() {
	GLOBAL_DEF(PropertyInfo(Variant::INT, TWIM_WORLD_TICK_RATE, PROPERTY_HINT_RANGE, "1,2048,1,or_greater"), 120);
	GLOBAL_DEF(PropertyInfo(Variant::INT, TWIM_WORLD_CHUNK_SIZE_X, PROPERTY_HINT_RANGE, "1,512,1,or_greater"), 32);
	GLOBAL_DEF(PropertyInfo(Variant::INT, TWIM_WORLD_CHUNK_SIZE_Y, PROPERTY_HINT_RANGE, "1,512,1,or_greater"), 32);
	GLOBAL_DEF(PropertyInfo(Variant::INT, TWIM_WORLD_CHUNK_SIZE_Z, PROPERTY_HINT_RANGE, "1,512,1,or_greater"), 16);

	GLOBAL_DEF(PropertyInfo(Variant::STRING, TWIM_RES_TILE_SET, PROPERTY_HINT_FILE, "*.tres"), String());
	GLOBAL_DEF(PropertyInfo(Variant::STRING, TWIM_RES_ENTITY_CATALOG, PROPERTY_HINT_FILE, "*.tres"), String());
	GLOBAL_DEF(PropertyInfo(Variant::STRING, TWIM_RES_ITEM_CATALOG, PROPERTY_HINT_FILE, "*.tres"), String());
	GLOBAL_DEF(PropertyInfo(Variant::STRING, TWIM_RES_WORLD_PROFILE, PROPERTY_HINT_FILE, "*.tres"), String());
	GLOBAL_DEF(PropertyInfo(Variant::STRING, TWIM_RENDER_GLYPH_FONT, PROPERTY_HINT_FILE, "*.ttf,*.otf,*.font,*.tres"), String());
	GLOBAL_DEF(PropertyInfo(Variant::INT, TWIM_RENDER_GLYPH_FONT_SIZE, PROPERTY_HINT_RANGE, "1,512,1,or_greater"), 16);
}

#ifdef TOOLS_ENABLED
void register_editor_settings() {
	if (EditorSettings::get_singleton() == nullptr) {
		return;
	}

	EDITOR_DEF("twim/editor/tiles/auto_open_last_tile_set", true);
	EDITOR_DEF("twim/editor/entities/auto_open_last_catalog", true);
	EDITOR_DEF("twim/editor/items/auto_open_last_catalog", true);
	EDITOR_DEF("twim/editor/world/auto_open_last_profile", true);
}
#endif

} // namespace twim


