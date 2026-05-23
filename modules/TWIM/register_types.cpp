//
// Created by busin on 2026-05-22.
//

#include "register_types.h"

#include "core/object/class_db.h"

#include "modules/TWIM/entities/entity_catalog_resource.h"
#include "modules/TWIM/entities/entity_type_resource.h"
#include "modules/TWIM/items/item_catalog_resource.h"
#include "modules/TWIM/items/item_type_resource.h"
#include "modules/TWIM/terrain/tile_set_resource.h"
#include "modules/TWIM/terrain/tile_type_resource.h"
#include "modules/TWIM/twim_settings.h"
#include "modules/TWIM/world/twim_world_server.h"
#include "modules/TWIM/world/world_profile_resource.h"

#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#include "modules/TWIM/editor/twim_editor_plugin.h"
#endif

void initialize_TWIM_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		twim::register_project_settings();

		GDREGISTER_CLASS(TwimTileTypeResource);
		GDREGISTER_CLASS(TwimTileSetResource);
		GDREGISTER_CLASS(TwimEntityTypeResource);
		GDREGISTER_CLASS(TwimEntityCatalogResource);
		GDREGISTER_CLASS(TwimItemTypeResource);
		GDREGISTER_CLASS(TwimItemCatalogResource);
		GDREGISTER_CLASS(TwimWorldProfileResource);
		GDREGISTER_CLASS(TwimWorldServer);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		twim::register_editor_settings();
		GDREGISTER_CLASS(TwimEditorPlugin);
		EditorPlugins::add_by_type<TwimEditorPlugin>();
	}
#endif
}

void uninitialize_TWIM_module(ModuleInitializationLevel p_level) {
	(void)p_level;
}
