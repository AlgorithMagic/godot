#include "twim_editor_plugin.h"

#include "modules/TWIM/entities/entity_catalog_resource.h"
#include "modules/TWIM/entities/entity_type_resource.h"
#include "modules/TWIM/editor/twim_editor_catalog_utils.h"
#include "modules/TWIM/items/item_catalog_resource.h"
#include "modules/TWIM/items/item_type_resource.h"
#include "modules/TWIM/terrain/tile_set_resource.h"
#include "modules/TWIM/terrain/tile_type_resource.h"
#include "modules/TWIM/world/twim_world_server.h"
#include "modules/TWIM/world/world_profile_resource.h"

#include "core/config/project_settings.h"
#include "core/io/resource_loader.h"
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/io/resource_saver.h"
#include "core/variant/variant.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/gui/editor_file_dialog.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/box_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/option_button.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/separator.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/tab_container.h"
#include "scene/main/window.h"

namespace {
bool is_valid_unicode_scalar_editor(int32_t p_codepoint) {
	if (p_codepoint < 0 || p_codepoint > 0x10FFFF) {
		return false;
	}

	return !(p_codepoint >= 0xD800 && p_codepoint <= 0xDFFF);
}
} // namespace

String TwimEditorPlugin::get_plugin_name() const {
	return "TWIM";
}

void TwimEditorPlugin::edit(Object *p_object) {
	(void)p_object;
}

bool TwimEditorPlugin::handles(Object *p_object) const {
	(void)p_object;
	return false;
}

void TwimEditorPlugin::make_visible(bool p_visible) {
	if (main_screen != nullptr) {
		main_screen->set_visible(p_visible);
	}
}

void TwimEditorPlugin::_destroy_world_server() {
	if (world_server == nullptr) {
		return;
	}

	memdelete(world_server);
	world_server = nullptr;
}

void TwimEditorPlugin::_notification(int p_what) {
	if (p_what == NOTIFICATION_PREDELETE) {
		_destroy_world_server();
	}
}

TwimEditorPlugin::~TwimEditorPlugin() {
	_destroy_world_server();
}

void TwimEditorPlugin::_refresh_summary() {
	if (summary_label == nullptr) {
		return;
	}

	summary_label->set_text(
			"TWIM project settings live under Project Settings > TWIM.\n"
			"Use this tab to bootstrap resources and drive world/entity workflows directly from the editor.");
}

void TwimEditorPlugin::_refresh_world_status() {
	if (world_status_label == nullptr || world_server == nullptr) {
		return;
	}

	if (!world_server->has_world()) {
		String status = "No world loaded.";
		if (!last_error_text.is_empty()) {
			status += " Last error: " + last_error_text;
		}
		world_status_label->set_text(status);
		return;
	}

	const Dictionary info = world_server->get_world_debug_info();
	const String world_name = info.get("world_name", String());
	const int64_t tick_index = info.get("tick_index", int64_t(0));
	const int64_t chunk_count = info.get("loaded_chunk_count", int64_t(0));
	const int64_t entity_count = info.get("loaded_entity_count", int64_t(0));
	const String glyph_font_path = info.get("glyph_font_resource_path", String());
	const int64_t glyph_font_size = info.get("glyph_font_size", int64_t(16));

	String status = vformat("World '%s' | tick=%d | chunks=%d | entities=%d | font=%s@%d", world_name, tick_index, chunk_count, entity_count, glyph_font_path, glyph_font_size);
	if (!last_error_text.is_empty()) {
		status += " | last_error=" + last_error_text;
	}
	world_status_label->set_text(status);
}

void TwimEditorPlugin::_refresh_tile_catalog_status() {
	if (tile_catalog_status_label == nullptr) {
		return;
	}

	tile_catalog_status_label->set_text(vformat("Tile entries: %d", tile_catalog_entries.size()));
	if (!last_error_text.is_empty()) {
		tile_catalog_status_label->set_text(tile_catalog_status_label->get_text() + " | last_error=" + last_error_text);
	}
}

void TwimEditorPlugin::_refresh_entity_catalog_status() {
	if (entity_catalog_status_label == nullptr) {
		return;
	}

	entity_catalog_status_label->set_text(vformat("Entity entries: %d", entity_catalog_entries.size()));
	if (!last_error_text.is_empty()) {
		entity_catalog_status_label->set_text(entity_catalog_status_label->get_text() + " | last_error=" + last_error_text);
	}
}

void TwimEditorPlugin::_refresh_item_catalog_status() {
	if (item_catalog_status_label == nullptr) {
		return;
	}

	item_catalog_status_label->set_text(vformat("Item entries: %d", item_catalog_entries.size()));
	if (!last_error_text.is_empty()) {
		item_catalog_status_label->set_text(item_catalog_status_label->get_text() + " | last_error=" + last_error_text);
	}
}

void TwimEditorPlugin::_request_save_tile_set() {
	pending_save_type = "tile_set";
	save_dialog->clear_filters();
	save_dialog->add_filter("*.tres");
	save_dialog->set_current_file("twim_tile_set.tres");
	save_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_request_save_entity_catalog() {
	pending_save_type = "entity_catalog";
	save_dialog->clear_filters();
	save_dialog->add_filter("*.tres");
	save_dialog->set_current_file("twim_entity_catalog.tres");
	save_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_request_save_item_catalog() {
	pending_save_type = "item_catalog";
	save_dialog->clear_filters();
	save_dialog->add_filter("*.tres");
	save_dialog->set_current_file("twim_item_catalog.tres");
	save_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_request_save_world_profile() {
	pending_save_type = "world_profile";
	save_dialog->clear_filters();
	save_dialog->add_filter("*.tres");
	save_dialog->set_current_file("twim_world_profile.tres");
	save_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_save_resource_to_path(const String &p_path) {
	if (pending_save_type.is_empty()) {
		return;
	}

	Ref<Resource> resource;
	if (pending_save_type == "tile_set") {
		Ref<TwimTileSetResource> tile_set(memnew(TwimTileSetResource));
		tile_set->set_tile_types(tile_catalog_entries);
		resource = tile_set;
	} else if (pending_save_type == "entity_catalog") {
		Ref<TwimEntityCatalogResource> entity_catalog(memnew(TwimEntityCatalogResource));
		entity_catalog->set_entity_types(entity_catalog_entries);
		resource = entity_catalog;
	} else if (pending_save_type == "item_catalog") {
		Ref<TwimItemCatalogResource> item_catalog(memnew(TwimItemCatalogResource));
		item_catalog->set_item_types(item_catalog_entries);
		resource = item_catalog;
	} else if (pending_save_type == "world_profile") {
		Ref<TwimWorldProfileResource> world_profile(memnew(TwimWorldProfileResource));
		if (world_font_path_edit != nullptr) {
			world_profile->set_glyph_font_resource_path(world_font_path_edit->get_text().strip_edges());
		}
		if (world_font_size_spin_box != nullptr) {
			world_profile->set_glyph_font_size(static_cast<int32_t>(world_font_size_spin_box->get_value()));
		}
		resource = world_profile;
	}

	if (resource.is_null()) {
		pending_save_type = String();
		return;
	}

	const Error save_error = ResourceSaver::save(resource, p_path);
	if (save_error == OK) {
		pending_save_type = String();
		last_error_text = String();
	} else {
		last_error_text = vformat("save_resource:%d", save_error);
	}
}

void TwimEditorPlugin::_add_tile_catalog_entry() {
	if (tile_name_edit == nullptr || tile_glyph_edit == nullptr || tile_solid_checkbox == nullptr || tile_opaque_checkbox == nullptr || tile_liquid_checkbox == nullptr) {
		return;
	}

	const StringName tile_name = tile_name_edit->get_text().strip_edges();
	if (tile_name == StringName()) {
		return;
	}

	Ref<TwimTileTypeResource> entry(memnew(TwimTileTypeResource));
	entry->set_tile_name(tile_name);
	entry->set_glyph(tile_glyph_edit->get_text());
	entry->set_solid(tile_solid_checkbox->is_pressed());
	entry->set_opaque(tile_opaque_checkbox->is_pressed());
	entry->set_liquid(tile_liquid_checkbox->is_pressed());
	tile_catalog_entries.push_back(entry);

	tile_name_edit->set_text(String());
	tile_glyph_edit->set_text(String());
	tile_solid_checkbox->set_pressed(false);
	tile_opaque_checkbox->set_pressed(false);
	tile_liquid_checkbox->set_pressed(false);
	selected_tile_catalog_index = -1;
	last_error_text = String();
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_update_selected_tile_catalog_entry() {
	if (selected_tile_catalog_index < 0 || selected_tile_catalog_index >= tile_catalog_entries.size()) {
		last_error_text = "update_tile_catalog:no_selection";
		_refresh_tile_catalog_status();
		return;
	}

	Ref<TwimTileTypeResource> entry = tile_catalog_entries[selected_tile_catalog_index];
	if (entry.is_null()) {
		last_error_text = "update_tile_catalog:null_entry";
		_refresh_tile_catalog_status();
		return;
	}

	const StringName tile_name = tile_name_edit->get_text().strip_edges();
	if (tile_name == StringName()) {
		last_error_text = "update_tile_catalog:empty_name";
		_refresh_tile_catalog_status();
		return;
	}

	entry->set_tile_name(tile_name);
	entry->set_glyph(tile_glyph_edit->get_text());
	entry->set_solid(tile_solid_checkbox->is_pressed());
	entry->set_opaque(tile_opaque_checkbox->is_pressed());
	entry->set_liquid(tile_liquid_checkbox->is_pressed());
	last_error_text = String();
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_remove_selected_tile_catalog_entry() {
	if (twim::editor::catalog_utils::remove_entry(tile_catalog_entries, selected_tile_catalog_index) != OK) {
		last_error_text = "remove_tile_catalog:no_selection";
		_refresh_tile_catalog_status();
		return;
	}

	selected_tile_catalog_index = -1;
	last_error_text = String();
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_move_selected_tile_catalog_entry_up() {
	if (selected_tile_catalog_index <= 0 || selected_tile_catalog_index >= tile_catalog_entries.size()) {
		last_error_text = "move_tile_catalog_up:no_selection";
		_refresh_tile_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(tile_catalog_entries, selected_tile_catalog_index, selected_tile_catalog_index - 1);
	if (swap_error != OK) {
		last_error_text = "move_tile_catalog_up:swap_failed";
		_refresh_tile_catalog_status();
		return;
	}
	selected_tile_catalog_index -= 1;
	last_error_text = String();
	_refresh_tile_catalog_item_list();
	if (tile_catalog_item_list != nullptr) {
		tile_catalog_item_list->select(selected_tile_catalog_index);
	}
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_move_selected_tile_catalog_entry_down() {
	if (selected_tile_catalog_index < 0 || selected_tile_catalog_index >= tile_catalog_entries.size() - 1) {
		last_error_text = "move_tile_catalog_down:no_selection";
		_refresh_tile_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(tile_catalog_entries, selected_tile_catalog_index, selected_tile_catalog_index + 1);
	if (swap_error != OK) {
		last_error_text = "move_tile_catalog_down:swap_failed";
		_refresh_tile_catalog_status();
		return;
	}
	selected_tile_catalog_index += 1;
	last_error_text = String();
	_refresh_tile_catalog_item_list();
	if (tile_catalog_item_list != nullptr) {
		tile_catalog_item_list->select(selected_tile_catalog_index);
	}
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_on_tile_catalog_item_selected(int32_t p_index) {
	if (p_index < 0 || p_index >= tile_catalog_entries.size()) {
		return;
	}

	Ref<TwimTileTypeResource> entry = tile_catalog_entries[p_index];
	if (entry.is_null()) {
		return;
	}

	selected_tile_catalog_index = p_index;
	tile_name_edit->set_text(String(entry->get_tile_name()));
	tile_glyph_edit->set_text(entry->get_glyph());
	tile_solid_checkbox->set_pressed(entry->is_solid());
	tile_opaque_checkbox->set_pressed(entry->is_opaque());
	tile_liquid_checkbox->set_pressed(entry->is_liquid());
}

void TwimEditorPlugin::_refresh_tile_catalog_item_list() {
	if (tile_catalog_item_list == nullptr) {
		return;
	}

	tile_catalog_item_list->clear();
	for (int32_t i = 0; i < tile_catalog_entries.size(); ++i) {
		Ref<TwimTileTypeResource> entry = tile_catalog_entries[i];
		if (entry.is_null()) {
			tile_catalog_item_list->add_item(vformat("%d: <null>", i));
			continue;
		}
		tile_catalog_item_list->add_item(vformat("%d: %s (%s)", i, String(entry->get_tile_name()), entry->get_glyph()));
	}
}

void TwimEditorPlugin::_clear_tile_catalog_entries() {
	tile_catalog_entries.clear();
	selected_tile_catalog_index = -1;
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_save_tile_catalog() {
	if (tile_catalog_path_edit == nullptr) {
		return;
	}

	const String path = tile_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<TwimTileSetResource> tile_set(memnew(TwimTileSetResource));
	tile_set->set_tile_types(tile_catalog_entries);
	const Error save_error = ResourceSaver::save(tile_set, path);
	if (save_error != OK) {
		last_error_text = vformat("save_tile_catalog:%d", save_error);
		_refresh_tile_catalog_item_list();
		_refresh_tile_catalog_status();
		return;
	}
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/tile_set", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_tile_catalog_status();
	_refresh_world_status();
}

void TwimEditorPlugin::_load_tile_catalog() {
	if (tile_catalog_path_edit == nullptr) {
		return;
	}

	const String path = tile_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<Resource> loaded = ResourceLoader::load(path);
	Ref<TwimTileSetResource> tile_set = loaded;
	if (tile_set.is_null()) {
		last_error_text = "load_tile_catalog";
		_refresh_tile_catalog_item_list();
		_refresh_tile_catalog_status();
		return;
	}

	tile_catalog_entries = tile_set->get_tile_types();
	selected_tile_catalog_index = -1;
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/tile_set", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
}

void TwimEditorPlugin::_add_entity_catalog_entry() {
	if (entity_name_edit == nullptr || entity_glyph_edit == nullptr || entity_blocking_checkbox == nullptr) {
		return;
	}

	const StringName entity_name = entity_name_edit->get_text().strip_edges();
	if (entity_name == StringName()) {
		return;
	}

	Ref<TwimEntityTypeResource> entry(memnew(TwimEntityTypeResource));
	entry->set_entity_name(entity_name);
	entry->set_glyph(entity_glyph_edit->get_text());
	entry->set_blocking(entity_blocking_checkbox->is_pressed());
	entity_catalog_entries.push_back(entry);

	entity_name_edit->set_text(String());
	entity_glyph_edit->set_text(String());
	entity_blocking_checkbox->set_pressed(false);
	selected_entity_catalog_index = -1;
	last_error_text = String();
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_update_selected_entity_catalog_entry() {
	if (selected_entity_catalog_index < 0 || selected_entity_catalog_index >= entity_catalog_entries.size()) {
		last_error_text = "update_entity_catalog:no_selection";
		_refresh_entity_catalog_status();
		return;
	}

	Ref<TwimEntityTypeResource> entry = entity_catalog_entries[selected_entity_catalog_index];
	if (entry.is_null()) {
		last_error_text = "update_entity_catalog:null_entry";
		_refresh_entity_catalog_status();
		return;
	}

	const StringName entity_name = entity_name_edit->get_text().strip_edges();
	if (entity_name == StringName()) {
		last_error_text = "update_entity_catalog:empty_name";
		_refresh_entity_catalog_status();
		return;
	}

	entry->set_entity_name(entity_name);
	entry->set_glyph(entity_glyph_edit->get_text());
	entry->set_blocking(entity_blocking_checkbox->is_pressed());
	last_error_text = String();
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_remove_selected_entity_catalog_entry() {
	if (twim::editor::catalog_utils::remove_entry(entity_catalog_entries, selected_entity_catalog_index) != OK) {
		last_error_text = "remove_entity_catalog:no_selection";
		_refresh_entity_catalog_status();
		return;
	}

	selected_entity_catalog_index = -1;
	last_error_text = String();
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_move_selected_entity_catalog_entry_up() {
	if (selected_entity_catalog_index <= 0 || selected_entity_catalog_index >= entity_catalog_entries.size()) {
		last_error_text = "move_entity_catalog_up:no_selection";
		_refresh_entity_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(entity_catalog_entries, selected_entity_catalog_index, selected_entity_catalog_index - 1);
	if (swap_error != OK) {
		last_error_text = "move_entity_catalog_up:swap_failed";
		_refresh_entity_catalog_status();
		return;
	}
	selected_entity_catalog_index -= 1;
	last_error_text = String();
	_refresh_entity_catalog_item_list();
	if (entity_catalog_item_list != nullptr) {
		entity_catalog_item_list->select(selected_entity_catalog_index);
	}
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_move_selected_entity_catalog_entry_down() {
	if (selected_entity_catalog_index < 0 || selected_entity_catalog_index >= entity_catalog_entries.size() - 1) {
		last_error_text = "move_entity_catalog_down:no_selection";
		_refresh_entity_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(entity_catalog_entries, selected_entity_catalog_index, selected_entity_catalog_index + 1);
	if (swap_error != OK) {
		last_error_text = "move_entity_catalog_down:swap_failed";
		_refresh_entity_catalog_status();
		return;
	}
	selected_entity_catalog_index += 1;
	last_error_text = String();
	_refresh_entity_catalog_item_list();
	if (entity_catalog_item_list != nullptr) {
		entity_catalog_item_list->select(selected_entity_catalog_index);
	}
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_on_entity_catalog_item_selected(int32_t p_index) {
	if (p_index < 0 || p_index >= entity_catalog_entries.size()) {
		return;
	}

	Ref<TwimEntityTypeResource> entry = entity_catalog_entries[p_index];
	if (entry.is_null()) {
		return;
	}

	selected_entity_catalog_index = p_index;
	entity_name_edit->set_text(String(entry->get_entity_name()));
	entity_glyph_edit->set_text(entry->get_glyph());
	entity_blocking_checkbox->set_pressed(entry->is_blocking());
}

void TwimEditorPlugin::_refresh_entity_catalog_item_list() {
	if (entity_catalog_item_list == nullptr) {
		return;
	}

	entity_catalog_item_list->clear();
	for (int32_t i = 0; i < entity_catalog_entries.size(); ++i) {
		Ref<TwimEntityTypeResource> entry = entity_catalog_entries[i];
		if (entry.is_null()) {
			entity_catalog_item_list->add_item(vformat("%d: <null>", i));
			continue;
		}
		entity_catalog_item_list->add_item(vformat("%d: %s (%s)", i, String(entry->get_entity_name()), entry->get_glyph()));
	}
}

void TwimEditorPlugin::_clear_entity_catalog_entries() {
	entity_catalog_entries.clear();
	selected_entity_catalog_index = -1;
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_save_entity_catalog() {
	if (entity_catalog_path_edit == nullptr) {
		return;
	}

	const String path = entity_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<TwimEntityCatalogResource> catalog(memnew(TwimEntityCatalogResource));
	catalog->set_entity_types(entity_catalog_entries);
	const Error save_error = ResourceSaver::save(catalog, path);
	if (save_error != OK) {
		last_error_text = vformat("save_entity_catalog:%d", save_error);
		_refresh_entity_catalog_item_list();
		_refresh_entity_catalog_status();
		return;
	}
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/entity_catalog", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_entity_catalog_status();
	_refresh_world_status();
}

void TwimEditorPlugin::_load_entity_catalog() {
	if (entity_catalog_path_edit == nullptr) {
		return;
	}

	const String path = entity_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<Resource> loaded = ResourceLoader::load(path);
	Ref<TwimEntityCatalogResource> catalog = loaded;
	if (catalog.is_null()) {
		last_error_text = "load_entity_catalog";
		_refresh_entity_catalog_item_list();
		_refresh_entity_catalog_status();
		return;
	}

	entity_catalog_entries = catalog->get_entity_types();
	selected_entity_catalog_index = -1;
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/entity_catalog", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
}

void TwimEditorPlugin::_add_item_catalog_entry() {
	if (item_name_edit == nullptr || item_glyph_edit == nullptr || item_stackable_checkbox == nullptr || item_weight_spin_box == nullptr) {
		return;
	}

	const StringName item_name = item_name_edit->get_text().strip_edges();
	if (item_name == StringName()) {
		last_error_text = "add_item_catalog:empty_name";
		_refresh_item_catalog_status();
		return;
	}

	Ref<TwimItemTypeResource> entry(memnew(TwimItemTypeResource));
	entry->set_item_name(item_name);
	entry->set_glyph(item_glyph_edit->get_text());
	entry->set_stackable(item_stackable_checkbox->is_pressed());
	entry->set_unit_weight(item_weight_spin_box->get_value());
	item_catalog_entries.push_back(entry);

	item_name_edit->set_text(String());
	item_glyph_edit->set_text(String());
	item_stackable_checkbox->set_pressed(true);
	item_weight_spin_box->set_value(1.0);
	selected_item_catalog_index = -1;
	last_error_text = String();
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_update_selected_item_catalog_entry() {
	if (selected_item_catalog_index < 0 || selected_item_catalog_index >= item_catalog_entries.size()) {
		last_error_text = "update_item_catalog:no_selection";
		_refresh_item_catalog_status();
		return;
	}

	Ref<TwimItemTypeResource> entry = item_catalog_entries[selected_item_catalog_index];
	if (entry.is_null()) {
		last_error_text = "update_item_catalog:null_entry";
		_refresh_item_catalog_status();
		return;
	}

	const StringName item_name = item_name_edit->get_text().strip_edges();
	if (item_name == StringName()) {
		last_error_text = "update_item_catalog:empty_name";
		_refresh_item_catalog_status();
		return;
	}

	entry->set_item_name(item_name);
	entry->set_glyph(item_glyph_edit->get_text());
	entry->set_stackable(item_stackable_checkbox->is_pressed());
	entry->set_unit_weight(item_weight_spin_box->get_value());
	last_error_text = String();
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_remove_selected_item_catalog_entry() {
	if (twim::editor::catalog_utils::remove_entry(item_catalog_entries, selected_item_catalog_index) != OK) {
		last_error_text = "remove_item_catalog:no_selection";
		_refresh_item_catalog_status();
		return;
	}

	selected_item_catalog_index = -1;
	last_error_text = String();
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_move_selected_item_catalog_entry_up() {
	if (selected_item_catalog_index <= 0 || selected_item_catalog_index >= item_catalog_entries.size()) {
		last_error_text = "move_item_catalog_up:no_selection";
		_refresh_item_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(item_catalog_entries, selected_item_catalog_index, selected_item_catalog_index - 1);
	if (swap_error != OK) {
		last_error_text = "move_item_catalog_up:swap_failed";
		_refresh_item_catalog_status();
		return;
	}
	selected_item_catalog_index -= 1;
	last_error_text = String();
	_refresh_item_catalog_item_list();
	if (item_catalog_item_list != nullptr) {
		item_catalog_item_list->select(selected_item_catalog_index);
	}
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_move_selected_item_catalog_entry_down() {
	if (selected_item_catalog_index < 0 || selected_item_catalog_index >= item_catalog_entries.size() - 1) {
		last_error_text = "move_item_catalog_down:no_selection";
		_refresh_item_catalog_status();
		return;
	}

	const Error swap_error = twim::editor::catalog_utils::swap_entries(item_catalog_entries, selected_item_catalog_index, selected_item_catalog_index + 1);
	if (swap_error != OK) {
		last_error_text = "move_item_catalog_down:swap_failed";
		_refresh_item_catalog_status();
		return;
	}
	selected_item_catalog_index += 1;
	last_error_text = String();
	_refresh_item_catalog_item_list();
	if (item_catalog_item_list != nullptr) {
		item_catalog_item_list->select(selected_item_catalog_index);
	}
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_on_item_catalog_item_selected(int32_t p_index) {
	if (p_index < 0 || p_index >= item_catalog_entries.size()) {
		return;
	}

	Ref<TwimItemTypeResource> entry = item_catalog_entries[p_index];
	if (entry.is_null()) {
		return;
	}

	selected_item_catalog_index = p_index;
	item_name_edit->set_text(String(entry->get_item_name()));
	item_glyph_edit->set_text(entry->get_glyph());
	item_stackable_checkbox->set_pressed(entry->is_stackable());
	item_weight_spin_box->set_value(entry->get_unit_weight());
}

void TwimEditorPlugin::_refresh_item_catalog_item_list() {
	if (item_catalog_item_list == nullptr) {
		return;
	}

	item_catalog_item_list->clear();
	for (int32_t i = 0; i < item_catalog_entries.size(); ++i) {
		Ref<TwimItemTypeResource> entry = item_catalog_entries[i];
		if (entry.is_null()) {
			item_catalog_item_list->add_item(vformat("%d: <null>", i));
			continue;
		}
		item_catalog_item_list->add_item(vformat("%d: %s (%s)", i, String(entry->get_item_name()), entry->get_glyph()));
	}
}

void TwimEditorPlugin::_clear_item_catalog_entries() {
	item_catalog_entries.clear();
	selected_item_catalog_index = -1;
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_save_item_catalog() {
	if (item_catalog_path_edit == nullptr) {
		return;
	}

	const String path = item_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<TwimItemCatalogResource> catalog(memnew(TwimItemCatalogResource));
	catalog->set_item_types(item_catalog_entries);
	const Error save_error = ResourceSaver::save(catalog, path);
	if (save_error != OK) {
		last_error_text = vformat("save_item_catalog:%d", save_error);
		_refresh_item_catalog_item_list();
		_refresh_item_catalog_status();
		return;
	}
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/item_catalog", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_load_item_catalog() {
	if (item_catalog_path_edit == nullptr) {
		return;
	}

	const String path = item_catalog_path_edit->get_text().strip_edges();
	if (path.is_empty()) {
		return;
	}

	Ref<Resource> loaded = ResourceLoader::load(path);
	Ref<TwimItemCatalogResource> catalog = loaded;
	if (catalog.is_null()) {
		last_error_text = "load_item_catalog";
		_refresh_item_catalog_item_list();
		_refresh_item_catalog_status();
		return;
	}

	item_catalog_entries = catalog->get_item_types();
	selected_item_catalog_index = -1;
	last_error_text = String();
	if (ProjectSettings::get_singleton() != nullptr) {
		ProjectSettings::get_singleton()->set_setting("twim/resources/item_catalog", path);
		const Error project_save_error = ProjectSettings::get_singleton()->save();
		if (project_save_error != OK) {
			last_error_text = vformat("save_project_settings:%d", project_save_error);
		}
	}
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
}

void TwimEditorPlugin::_refresh_type_options() {
	if (tile_type_option == nullptr || entity_type_option == nullptr || world_server == nullptr) {
		return;
	}

	tile_type_option->clear();
	entity_type_option->clear();

	if (!world_server->has_world()) {
		return;
	}

	const PackedStringArray tile_names = world_server->get_tile_type_names();
	for (int32_t i = 0; i < tile_names.size(); ++i) {
		tile_type_option->add_item(tile_names[i]);
	}
	if (tile_type_option->get_item_count() > 0) {
		tile_type_option->select(0);
	}

	const PackedStringArray entity_names = world_server->get_entity_type_names();
	for (int32_t i = 0; i < entity_names.size(); ++i) {
		entity_type_option->add_item(entity_names[i]);
	}
	if (entity_type_option->get_item_count() > 0) {
		entity_type_option->select(0);
	}

	if (entity_type_option->get_item_count() > 0 && entity_type_edit != nullptr) {
		entity_type_edit->set_text(entity_type_option->get_item_text(entity_type_option->get_selected()));
	}
}

void TwimEditorPlugin::_create_world() {
	if (world_server == nullptr || world_name_edit == nullptr || seed_spin_box == nullptr) {
		return;
	}

	const String world_name = world_name_edit->get_text().strip_edges();
	const int64_t seed = static_cast<int64_t>(seed_spin_box->get_value());
	const Error error = world_server->create_world(world_name, seed);
	last_error_text = error == OK ? String() : vformat("create_world:%d", error);
	_refresh_type_options();
	_refresh_snapshot();
	_refresh_world_status();
}

void TwimEditorPlugin::_unload_world() {
	if (world_server == nullptr) {
		return;
	}

	world_server->unload_world();
	last_error_text = String();
	_refresh_type_options();
	_refresh_snapshot();
	_refresh_world_status();
}

void TwimEditorPlugin::_step_world() {
	if (world_server == nullptr || tick_spin_box == nullptr) {
		return;
	}

	const int32_t ticks = static_cast<int32_t>(tick_spin_box->get_value());
	const Error error = world_server->step_ticks(ticks);
	last_error_text = error == OK ? String() : vformat("step_ticks:%d", error);
	_refresh_snapshot();
	_refresh_world_status();
}

void TwimEditorPlugin::_spawn_entity() {
	if (world_server == nullptr || entity_type_edit == nullptr || entity_x_spin_box == nullptr || entity_y_spin_box == nullptr || entity_z_spin_box == nullptr) {
		return;
	}

	StringName entity_type_name = entity_type_edit->get_text().strip_edges();
	if (entity_type_option != nullptr && entity_type_option->get_item_count() > 0) {
		entity_type_name = entity_type_option->get_item_text(entity_type_option->get_selected());
	}
	const Vector3i coord(
			static_cast<int32_t>(entity_x_spin_box->get_value()),
			static_cast<int32_t>(entity_y_spin_box->get_value()),
			static_cast<int32_t>(entity_z_spin_box->get_value()));
	const int64_t entity_id = world_server->spawn_entity(entity_type_name, coord);
	last_error_text = entity_id >= 0 ? String() : "spawn_entity";
	_refresh_world_status();
}

void TwimEditorPlugin::_set_tile_type() {
	if (world_server == nullptr || tile_type_option == nullptr || tile_x_spin_box == nullptr || tile_y_spin_box == nullptr || tile_z_spin_box == nullptr) {
		return;
	}
	if (tile_type_option->get_item_count() == 0) {
		last_error_text = "set_tile_type:no_types";
		_refresh_world_status();
		return;
	}

	const StringName type_name = tile_type_option->get_item_text(tile_type_option->get_selected());
	const int32_t type_id = world_server->get_tile_type_id_by_name(type_name);
	if (type_id < 0) {
		last_error_text = "set_tile_type:unknown_name";
		_refresh_world_status();
		return;
	}

	const Vector3i coord(
			static_cast<int32_t>(tile_x_spin_box->get_value()),
			static_cast<int32_t>(tile_y_spin_box->get_value()),
			static_cast<int32_t>(tile_z_spin_box->get_value()));

	const Error error = world_server->debug_set_tile_type_id(coord, type_id);
	last_error_text = error == OK ? String() : vformat("set_tile_type:%d", error);
	_refresh_snapshot();
	_refresh_world_status();
}

void TwimEditorPlugin::_refresh_snapshot() {
	if (snapshot_label == nullptr || world_server == nullptr || snapshot_origin_x_spin_box == nullptr || snapshot_origin_y_spin_box == nullptr || snapshot_layer_spin_box == nullptr || snapshot_width_spin_box == nullptr || snapshot_height_spin_box == nullptr) {
		return;
	}
	if (!world_server->has_world()) {
		snapshot_label->set_text("No world loaded.");
		return;
	}

	const Vector3i origin(
			static_cast<int32_t>(snapshot_origin_x_spin_box->get_value()),
			static_cast<int32_t>(snapshot_origin_y_spin_box->get_value()),
			static_cast<int32_t>(snapshot_layer_spin_box->get_value()));
	const Vector2i size(
			static_cast<int32_t>(snapshot_width_spin_box->get_value()),
			static_cast<int32_t>(snapshot_height_spin_box->get_value()));

	const Dictionary snapshot = world_server->build_debug_render_snapshot(origin, size, origin.z);
	if (!snapshot.get("available", false)) {
		snapshot_label->set_text(vformat("Snapshot unavailable: %s", String(snapshot.get("reason", String("unknown")))));
		return;
	}

	const Array cells = snapshot.get("cells", Array());
	String text;
	text.reserve(size.x * size.y * 2);
	for (int32_t y = 0; y < size.y; ++y) {
		for (int32_t x = 0; x < size.x; ++x) {
			const int32_t index = y * size.x + x;
			if (index < 0 || index >= cells.size() || cells[index].get_type() != Variant::DICTIONARY) {
				text += String("?");
				continue;
			}

			const Dictionary cell = cells[index];
			const int32_t glyph_id = int32_t(cell.get("glyph_id", 63));
			if (is_valid_unicode_scalar_editor(glyph_id) && glyph_id != 0) {
				text += String::chr(static_cast<char32_t>(glyph_id));
			} else {
				text += String("?");
			}
		}
		text += "\n";
	}

	snapshot_label->set_text(text);
}

void TwimEditorPlugin::_request_save_world_state() {
	pending_world_state_action = "save";
	world_state_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	world_state_dialog->clear_filters();
	world_state_dialog->add_filter("*.twim");
	world_state_dialog->set_current_file("world_state.twim");
	world_state_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_request_load_world_state() {
	pending_world_state_action = "load";
	world_state_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
	world_state_dialog->clear_filters();
	world_state_dialog->add_filter("*.twim");
	world_state_dialog->set_current_file("world_state.twim");
	world_state_dialog->popup_file_dialog();
}

void TwimEditorPlugin::_process_world_state_dialog_path(const String &p_path) {
	if (world_server == nullptr) {
		return;
	}

	if (pending_world_state_action == "save") {
		const Error error = world_server->save_world_state(p_path);
		last_error_text = error == OK ? String() : vformat("save_world_state:%d", error);
	} else if (pending_world_state_action == "load") {
		const Error error = world_server->load_world_state(p_path);
		last_error_text = error == OK ? String() : vformat("load_world_state:%d", error);
		if (error == OK) {
			_refresh_type_options();
			_refresh_snapshot();
		}
	}

	pending_world_state_action = String();
	_refresh_world_status();
}

void TwimEditorPlugin::_apply_world_render_settings() {
	if (world_font_path_edit == nullptr || world_font_size_spin_box == nullptr || ProjectSettings::get_singleton() == nullptr) {
		return;
	}

	ProjectSettings::get_singleton()->set_setting("twim/rendering/glyph_font", world_font_path_edit->get_text().strip_edges());
	ProjectSettings::get_singleton()->set_setting("twim/rendering/glyph_font_size", static_cast<int32_t>(world_font_size_spin_box->get_value()));
	const Error save_error = ProjectSettings::get_singleton()->save();
	last_error_text = save_error == OK ? String() : vformat("save_project_settings:%d", save_error);

	_refresh_world_status();
}

TwimEditorPlugin::TwimEditorPlugin() {
	world_server = memnew(TwimWorldServer);

	main_screen = memnew(VBoxContainer);
	main_screen->set_name("TWIM");
	main_screen->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

	summary_label = memnew(Label);
	main_screen->add_child(summary_label);

	HSeparator *separator = memnew(HSeparator);
	main_screen->add_child(separator);

	TabContainer *tabs = memnew(TabContainer);
	tabs->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_screen->add_child(tabs);

	VBoxContainer *tile_tab = memnew(VBoxContainer);
	tile_tab->set_name("Tiles");
	HBoxContainer *tile_path_row = memnew(HBoxContainer);
	tile_tab->add_child(tile_path_row);
	tile_catalog_path_edit = memnew(LineEdit);
	tile_catalog_path_edit->set_placeholder("res://twim_tile_set.tres");
	tile_catalog_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	tile_path_row->add_child(tile_catalog_path_edit);
	Button *load_tile_catalog = memnew(Button);
	load_tile_catalog->set_text("Load Catalog");
	load_tile_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_load_tile_catalog));
	tile_path_row->add_child(load_tile_catalog);
	Button *save_tile_catalog = memnew(Button);
	save_tile_catalog->set_text("Save Catalog");
	save_tile_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_save_tile_catalog));
	tile_path_row->add_child(save_tile_catalog);

	HBoxContainer *tile_entry_row = memnew(HBoxContainer);
	tile_tab->add_child(tile_entry_row);
	tile_name_edit = memnew(LineEdit);
	tile_name_edit->set_placeholder("tile_name");
	tile_entry_row->add_child(tile_name_edit);
	tile_glyph_edit = memnew(LineEdit);
	tile_glyph_edit->set_placeholder("glyph");
	tile_glyph_edit->set_max_length(4);
	tile_entry_row->add_child(tile_glyph_edit);
	tile_solid_checkbox = memnew(CheckBox);
	tile_solid_checkbox->set_text("Solid");
	tile_entry_row->add_child(tile_solid_checkbox);
	tile_opaque_checkbox = memnew(CheckBox);
	tile_opaque_checkbox->set_text("Opaque");
	tile_entry_row->add_child(tile_opaque_checkbox);
	tile_liquid_checkbox = memnew(CheckBox);
	tile_liquid_checkbox->set_text("Liquid");
	tile_entry_row->add_child(tile_liquid_checkbox);
	Button *add_tile = memnew(Button);
	add_tile->set_text("Add Tile");
	add_tile->connect("pressed", callable_mp(this, &TwimEditorPlugin::_add_tile_catalog_entry));
	tile_entry_row->add_child(add_tile);
	Button *clear_tiles = memnew(Button);
	clear_tiles->set_text("Clear");
	clear_tiles->connect("pressed", callable_mp(this, &TwimEditorPlugin::_clear_tile_catalog_entries));
	tile_entry_row->add_child(clear_tiles);
	Button *update_tile = memnew(Button);
	update_tile->set_text("Update Selected");
	update_tile->connect("pressed", callable_mp(this, &TwimEditorPlugin::_update_selected_tile_catalog_entry));
	tile_entry_row->add_child(update_tile);
	Button *remove_tile = memnew(Button);
	remove_tile->set_text("Remove Selected");
	remove_tile->connect("pressed", callable_mp(this, &TwimEditorPlugin::_remove_selected_tile_catalog_entry));
	tile_entry_row->add_child(remove_tile);
	Button *move_tile_up = memnew(Button);
	move_tile_up->set_text("Move Up");
	move_tile_up->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_tile_catalog_entry_up));
	tile_entry_row->add_child(move_tile_up);
	Button *move_tile_down = memnew(Button);
	move_tile_down->set_text("Move Down");
	move_tile_down->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_tile_catalog_entry_down));
	tile_entry_row->add_child(move_tile_down);

	tile_catalog_item_list = memnew(ItemList);
	tile_catalog_item_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	tile_catalog_item_list->set_custom_minimum_size(Size2(0, 140));
	tile_catalog_item_list->connect("item_selected", callable_mp(this, &TwimEditorPlugin::_on_tile_catalog_item_selected));
	tile_tab->add_child(tile_catalog_item_list);

	tile_catalog_status_label = memnew(Label);
	tile_catalog_status_label->set_text("Tile entries: 0");
	tile_tab->add_child(tile_catalog_status_label);

	Button *new_tile_set = memnew(Button);
	new_tile_set->set_text("Create Tile Set Resource");
	new_tile_set->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_save_tile_set));
	tile_tab->add_child(new_tile_set);
	tabs->add_child(tile_tab);

	VBoxContainer *entity_tab = memnew(VBoxContainer);
	entity_tab->set_name("Entities");
	HBoxContainer *entity_path_row = memnew(HBoxContainer);
	entity_tab->add_child(entity_path_row);
	entity_catalog_path_edit = memnew(LineEdit);
	entity_catalog_path_edit->set_placeholder("res://twim_entity_catalog.tres");
	entity_catalog_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	entity_path_row->add_child(entity_catalog_path_edit);
	Button *load_entity_catalog = memnew(Button);
	load_entity_catalog->set_text("Load Catalog");
	load_entity_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_load_entity_catalog));
	entity_path_row->add_child(load_entity_catalog);
	Button *save_entity_catalog = memnew(Button);
	save_entity_catalog->set_text("Save Catalog");
	save_entity_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_save_entity_catalog));
	entity_path_row->add_child(save_entity_catalog);

	HBoxContainer *entity_entry_row = memnew(HBoxContainer);
	entity_tab->add_child(entity_entry_row);
	entity_name_edit = memnew(LineEdit);
	entity_name_edit->set_placeholder("entity_name");
	entity_entry_row->add_child(entity_name_edit);
	entity_glyph_edit = memnew(LineEdit);
	entity_glyph_edit->set_placeholder("glyph");
	entity_glyph_edit->set_max_length(4);
	entity_entry_row->add_child(entity_glyph_edit);
	entity_blocking_checkbox = memnew(CheckBox);
	entity_blocking_checkbox->set_text("Blocking");
	entity_entry_row->add_child(entity_blocking_checkbox);
	Button *add_entity = memnew(Button);
	add_entity->set_text("Add Entity");
	add_entity->connect("pressed", callable_mp(this, &TwimEditorPlugin::_add_entity_catalog_entry));
	entity_entry_row->add_child(add_entity);
	Button *clear_entities = memnew(Button);
	clear_entities->set_text("Clear");
	clear_entities->connect("pressed", callable_mp(this, &TwimEditorPlugin::_clear_entity_catalog_entries));
	entity_entry_row->add_child(clear_entities);
	Button *update_entity = memnew(Button);
	update_entity->set_text("Update Selected");
	update_entity->connect("pressed", callable_mp(this, &TwimEditorPlugin::_update_selected_entity_catalog_entry));
	entity_entry_row->add_child(update_entity);
	Button *remove_entity = memnew(Button);
	remove_entity->set_text("Remove Selected");
	remove_entity->connect("pressed", callable_mp(this, &TwimEditorPlugin::_remove_selected_entity_catalog_entry));
	entity_entry_row->add_child(remove_entity);
	Button *move_entity_up = memnew(Button);
	move_entity_up->set_text("Move Up");
	move_entity_up->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_entity_catalog_entry_up));
	entity_entry_row->add_child(move_entity_up);
	Button *move_entity_down = memnew(Button);
	move_entity_down->set_text("Move Down");
	move_entity_down->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_entity_catalog_entry_down));
	entity_entry_row->add_child(move_entity_down);

	entity_catalog_item_list = memnew(ItemList);
	entity_catalog_item_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	entity_catalog_item_list->set_custom_minimum_size(Size2(0, 140));
	entity_catalog_item_list->connect("item_selected", callable_mp(this, &TwimEditorPlugin::_on_entity_catalog_item_selected));
	entity_tab->add_child(entity_catalog_item_list);

	entity_catalog_status_label = memnew(Label);
	entity_catalog_status_label->set_text("Entity entries: 0");
	entity_tab->add_child(entity_catalog_status_label);

	Button *new_entity_catalog = memnew(Button);
	new_entity_catalog->set_text("Create Entity Catalog Resource");
	new_entity_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_save_entity_catalog));
	entity_tab->add_child(new_entity_catalog);
	tabs->add_child(entity_tab);

	VBoxContainer *item_tab = memnew(VBoxContainer);
	item_tab->set_name("Items");
	HBoxContainer *item_path_row = memnew(HBoxContainer);
	item_tab->add_child(item_path_row);
	item_catalog_path_edit = memnew(LineEdit);
	item_catalog_path_edit->set_placeholder("res://twim_item_catalog.tres");
	item_catalog_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	item_path_row->add_child(item_catalog_path_edit);
	Button *load_item_catalog = memnew(Button);
	load_item_catalog->set_text("Load Catalog");
	load_item_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_load_item_catalog));
	item_path_row->add_child(load_item_catalog);
	Button *save_item_catalog = memnew(Button);
	save_item_catalog->set_text("Save Catalog");
	save_item_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_save_item_catalog));
	item_path_row->add_child(save_item_catalog);

	HBoxContainer *item_entry_row = memnew(HBoxContainer);
	item_tab->add_child(item_entry_row);
	item_name_edit = memnew(LineEdit);
	item_name_edit->set_placeholder("item_name");
	item_entry_row->add_child(item_name_edit);
	item_glyph_edit = memnew(LineEdit);
	item_glyph_edit->set_placeholder("glyph");
	item_glyph_edit->set_max_length(4);
	item_entry_row->add_child(item_glyph_edit);
	item_stackable_checkbox = memnew(CheckBox);
	item_stackable_checkbox->set_text("Stackable");
	item_stackable_checkbox->set_pressed(true);
	item_entry_row->add_child(item_stackable_checkbox);
	item_weight_spin_box = memnew(SpinBox);
	item_weight_spin_box->set_min(0.0);
	item_weight_spin_box->set_max(1000000.0);
	item_weight_spin_box->set_step(0.01);
	item_weight_spin_box->set_value(1.0);
	item_entry_row->add_child(item_weight_spin_box);
	Button *add_item = memnew(Button);
	add_item->set_text("Add Item");
	add_item->connect("pressed", callable_mp(this, &TwimEditorPlugin::_add_item_catalog_entry));
	item_entry_row->add_child(add_item);
	Button *clear_items = memnew(Button);
	clear_items->set_text("Clear");
	clear_items->connect("pressed", callable_mp(this, &TwimEditorPlugin::_clear_item_catalog_entries));
	item_entry_row->add_child(clear_items);
	Button *update_item = memnew(Button);
	update_item->set_text("Update Selected");
	update_item->connect("pressed", callable_mp(this, &TwimEditorPlugin::_update_selected_item_catalog_entry));
	item_entry_row->add_child(update_item);
	Button *remove_item = memnew(Button);
	remove_item->set_text("Remove Selected");
	remove_item->connect("pressed", callable_mp(this, &TwimEditorPlugin::_remove_selected_item_catalog_entry));
	item_entry_row->add_child(remove_item);
	Button *move_item_up = memnew(Button);
	move_item_up->set_text("Move Up");
	move_item_up->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_item_catalog_entry_up));
	item_entry_row->add_child(move_item_up);
	Button *move_item_down = memnew(Button);
	move_item_down->set_text("Move Down");
	move_item_down->connect("pressed", callable_mp(this, &TwimEditorPlugin::_move_selected_item_catalog_entry_down));
	item_entry_row->add_child(move_item_down);

	item_catalog_item_list = memnew(ItemList);
	item_catalog_item_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	item_catalog_item_list->set_custom_minimum_size(Size2(0, 140));
	item_catalog_item_list->connect("item_selected", callable_mp(this, &TwimEditorPlugin::_on_item_catalog_item_selected));
	item_tab->add_child(item_catalog_item_list);

	item_catalog_status_label = memnew(Label);
	item_catalog_status_label->set_text("Item entries: 0");
	item_tab->add_child(item_catalog_status_label);

	Button *new_item_catalog = memnew(Button);
	new_item_catalog->set_text("Create Item Catalog Resource");
	new_item_catalog->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_save_item_catalog));
	item_tab->add_child(new_item_catalog);
	tabs->add_child(item_tab);

	VBoxContainer *world_tab = memnew(VBoxContainer);
	world_tab->set_name("World");

	HBoxContainer *world_create_row = memnew(HBoxContainer);
	world_tab->add_child(world_create_row);
	world_name_edit = memnew(LineEdit);
	world_name_edit->set_placeholder("World Name");
	world_name_edit->set_text("twim_world");
	world_name_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	world_create_row->add_child(world_name_edit);
	seed_spin_box = memnew(SpinBox);
	seed_spin_box->set_min(0);
	seed_spin_box->set_max(2147483647);
	seed_spin_box->set_step(1);
	seed_spin_box->set_value(1);
	world_create_row->add_child(seed_spin_box);
	Button *create_world = memnew(Button);
	create_world->set_text("Create World");
	create_world->connect("pressed", callable_mp(this, &TwimEditorPlugin::_create_world));
	world_create_row->add_child(create_world);

	HBoxContainer *world_runtime_row = memnew(HBoxContainer);
	world_tab->add_child(world_runtime_row);
	tick_spin_box = memnew(SpinBox);
	tick_spin_box->set_min(1);
	tick_spin_box->set_max(100000);
	tick_spin_box->set_step(1);
	tick_spin_box->set_value(1);
	world_runtime_row->add_child(tick_spin_box);
	Button *step_world = memnew(Button);
	step_world->set_text("Step Ticks");
	step_world->connect("pressed", callable_mp(this, &TwimEditorPlugin::_step_world));
	world_runtime_row->add_child(step_world);
	Button *unload_world = memnew(Button);
	unload_world->set_text("Unload World");
	unload_world->connect("pressed", callable_mp(this, &TwimEditorPlugin::_unload_world));
	world_runtime_row->add_child(unload_world);

	HBoxContainer *world_state_row = memnew(HBoxContainer);
	world_tab->add_child(world_state_row);
	Button *save_state = memnew(Button);
	save_state->set_text("Save World State");
	save_state->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_save_world_state));
	world_state_row->add_child(save_state);
	Button *load_state = memnew(Button);
	load_state->set_text("Load World State");
	load_state->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_load_world_state));
	world_state_row->add_child(load_state);
	Button *refresh_types = memnew(Button);
	refresh_types->set_text("Refresh Types");
	refresh_types->connect("pressed", callable_mp(this, &TwimEditorPlugin::_refresh_type_options));
	world_state_row->add_child(refresh_types);

	HBoxContainer *world_render_row = memnew(HBoxContainer);
	world_tab->add_child(world_render_row);
	world_font_path_edit = memnew(LineEdit);
	world_font_path_edit->set_placeholder("res://fonts/my_unicode_font.ttf");
	world_font_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	world_render_row->add_child(world_font_path_edit);
	world_font_size_spin_box = memnew(SpinBox);
	world_font_size_spin_box->set_min(1);
	world_font_size_spin_box->set_max(512);
	world_font_size_spin_box->set_step(1);
	world_font_size_spin_box->set_value(16);
	world_render_row->add_child(world_font_size_spin_box);
	Button *apply_render_settings = memnew(Button);
	apply_render_settings->set_text("Apply Font Settings");
	apply_render_settings->connect("pressed", callable_mp(this, &TwimEditorPlugin::_apply_world_render_settings));
	world_render_row->add_child(apply_render_settings);

	HBoxContainer *tile_row = memnew(HBoxContainer);
	world_tab->add_child(tile_row);
	tile_type_option = memnew(OptionButton);
	tile_type_option->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	tile_row->add_child(tile_type_option);
	tile_x_spin_box = memnew(SpinBox);
	tile_x_spin_box->set_min(-1048576);
	tile_x_spin_box->set_max(1048575);
	tile_row->add_child(tile_x_spin_box);
	tile_y_spin_box = memnew(SpinBox);
	tile_y_spin_box->set_min(-1048576);
	tile_y_spin_box->set_max(1048575);
	tile_row->add_child(tile_y_spin_box);
	tile_z_spin_box = memnew(SpinBox);
	tile_z_spin_box->set_min(-1048576);
	tile_z_spin_box->set_max(1048575);
	tile_row->add_child(tile_z_spin_box);
	Button *set_tile_type = memnew(Button);
	set_tile_type->set_text("Set Tile");
	set_tile_type->connect("pressed", callable_mp(this, &TwimEditorPlugin::_set_tile_type));
	tile_row->add_child(set_tile_type);

	HBoxContainer *entity_row = memnew(HBoxContainer);
	world_tab->add_child(entity_row);
	entity_type_edit = memnew(LineEdit);
	entity_type_edit->set_placeholder("Entity Type Name");
	entity_type_edit->set_text("citizen");
	entity_row->add_child(entity_type_edit);
	entity_type_option = memnew(OptionButton);
	entity_type_option->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	entity_row->add_child(entity_type_option);
	entity_x_spin_box = memnew(SpinBox);
	entity_x_spin_box->set_min(-1048576);
	entity_x_spin_box->set_max(1048575);
	entity_row->add_child(entity_x_spin_box);
	entity_y_spin_box = memnew(SpinBox);
	entity_y_spin_box->set_min(-1048576);
	entity_y_spin_box->set_max(1048575);
	entity_row->add_child(entity_y_spin_box);
	entity_z_spin_box = memnew(SpinBox);
	entity_z_spin_box->set_min(-1048576);
	entity_z_spin_box->set_max(1048575);
	entity_row->add_child(entity_z_spin_box);
	Button *spawn_entity = memnew(Button);
	spawn_entity->set_text("Spawn Entity");
	spawn_entity->connect("pressed", callable_mp(this, &TwimEditorPlugin::_spawn_entity));
	entity_row->add_child(spawn_entity);

	HBoxContainer *snapshot_controls_row = memnew(HBoxContainer);
	world_tab->add_child(snapshot_controls_row);
	snapshot_origin_x_spin_box = memnew(SpinBox);
	snapshot_origin_x_spin_box->set_min(-1048576);
	snapshot_origin_x_spin_box->set_max(1048575);
	snapshot_controls_row->add_child(snapshot_origin_x_spin_box);
	snapshot_origin_y_spin_box = memnew(SpinBox);
	snapshot_origin_y_spin_box->set_min(-1048576);
	snapshot_origin_y_spin_box->set_max(1048575);
	snapshot_controls_row->add_child(snapshot_origin_y_spin_box);
	snapshot_layer_spin_box = memnew(SpinBox);
	snapshot_layer_spin_box->set_min(-1048576);
	snapshot_layer_spin_box->set_max(1048575);
	snapshot_controls_row->add_child(snapshot_layer_spin_box);
	snapshot_width_spin_box = memnew(SpinBox);
	snapshot_width_spin_box->set_min(1);
	snapshot_width_spin_box->set_max(256);
	snapshot_width_spin_box->set_value(32);
	snapshot_controls_row->add_child(snapshot_width_spin_box);
	snapshot_height_spin_box = memnew(SpinBox);
	snapshot_height_spin_box->set_min(1);
	snapshot_height_spin_box->set_max(256);
	snapshot_height_spin_box->set_value(16);
	snapshot_controls_row->add_child(snapshot_height_spin_box);
	Button *refresh_snapshot = memnew(Button);
	refresh_snapshot->set_text("Refresh Snapshot");
	refresh_snapshot->connect("pressed", callable_mp(this, &TwimEditorPlugin::_refresh_snapshot));
	snapshot_controls_row->add_child(refresh_snapshot);

	snapshot_label = memnew(RichTextLabel);
	snapshot_label->set_custom_minimum_size(Size2(0, 220));
	snapshot_label->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	snapshot_label->set_fit_content(true);
	snapshot_label->set_text("No world loaded.");
	world_tab->add_child(snapshot_label);

	world_status_label = memnew(Label);
	world_status_label->set_text("No world loaded.");
	world_tab->add_child(world_status_label);

	Button *new_world_profile = memnew(Button);
	new_world_profile->set_text("Create World Profile Resource");
	new_world_profile->connect("pressed", callable_mp(this, &TwimEditorPlugin::_request_save_world_profile));
	world_tab->add_child(new_world_profile);

	tabs->add_child(world_tab);

	save_dialog = memnew(EditorFileDialog);
	save_dialog->set_title("Save TWIM Resource");
	save_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
	save_dialog->connect("file_selected", callable_mp(this, &TwimEditorPlugin::_save_resource_to_path));
	EditorNode::get_singleton()->get_gui_base()->add_child(save_dialog);

	world_state_dialog = memnew(EditorFileDialog);
	world_state_dialog->set_title("TWIM World State");
	world_state_dialog->set_access(EditorFileDialog::ACCESS_USERDATA);
	world_state_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	world_state_dialog->connect("file_selected", callable_mp(this, &TwimEditorPlugin::_process_world_state_dialog_path));
	EditorNode::get_singleton()->get_gui_base()->add_child(world_state_dialog);

	EditorNode::get_singleton()->get_editor_main_screen()->add_child(main_screen);
	main_screen->hide();

	if (ProjectSettings::get_singleton()->has_setting("twim/resources/tile_set") && tile_catalog_path_edit != nullptr) {
		tile_catalog_path_edit->set_text(ProjectSettings::get_singleton()->get_setting("twim/resources/tile_set"));
	}
	if (ProjectSettings::get_singleton()->has_setting("twim/resources/entity_catalog") && entity_catalog_path_edit != nullptr) {
		entity_catalog_path_edit->set_text(ProjectSettings::get_singleton()->get_setting("twim/resources/entity_catalog"));
	}
	if (ProjectSettings::get_singleton()->has_setting("twim/resources/item_catalog") && item_catalog_path_edit != nullptr) {
		item_catalog_path_edit->set_text(ProjectSettings::get_singleton()->get_setting("twim/resources/item_catalog"));
	}
	if (ProjectSettings::get_singleton()->has_setting("twim/rendering/glyph_font") && world_font_path_edit != nullptr) {
		world_font_path_edit->set_text(ProjectSettings::get_singleton()->get_setting("twim/rendering/glyph_font"));
	}
	if (ProjectSettings::get_singleton()->has_setting("twim/rendering/glyph_font_size") && world_font_size_spin_box != nullptr) {
		world_font_size_spin_box->set_value(ProjectSettings::get_singleton()->get_setting("twim/rendering/glyph_font_size"));
	}

	_refresh_summary();
	_refresh_world_status();
	_refresh_tile_catalog_item_list();
	_refresh_tile_catalog_status();
	_refresh_entity_catalog_item_list();
	_refresh_entity_catalog_status();
	_refresh_item_catalog_item_list();
	_refresh_item_catalog_status();
	_refresh_type_options();
	_refresh_snapshot();
}






