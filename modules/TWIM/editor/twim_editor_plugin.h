#pragma once

#include "editor/plugins/editor_plugin.h"

class Button;
class CheckBox;
class EditorFileDialog;
class ItemList;
class Label;
class LineEdit;
class OptionButton;
class RichTextLabel;
class SpinBox;
class TabContainer;
class TwimWorldServer;

class TwimEditorPlugin : public EditorPlugin {
	GDCLASS(TwimEditorPlugin, EditorPlugin);

private:
	Control *main_screen = nullptr;
	Label *summary_label = nullptr;
	Label *world_status_label = nullptr;
	Label *tile_catalog_status_label = nullptr;
	Label *entity_catalog_status_label = nullptr;
	Label *item_catalog_status_label = nullptr;
	EditorFileDialog *save_dialog = nullptr;
	EditorFileDialog *world_state_dialog = nullptr;
	LineEdit *tile_catalog_path_edit = nullptr;
	LineEdit *entity_catalog_path_edit = nullptr;
	LineEdit *item_catalog_path_edit = nullptr;
	LineEdit *tile_name_edit = nullptr;
	LineEdit *tile_glyph_edit = nullptr;
	ItemList *tile_catalog_item_list = nullptr;
	CheckBox *tile_solid_checkbox = nullptr;
	CheckBox *tile_opaque_checkbox = nullptr;
	CheckBox *tile_liquid_checkbox = nullptr;
	LineEdit *entity_name_edit = nullptr;
	LineEdit *entity_glyph_edit = nullptr;
	ItemList *entity_catalog_item_list = nullptr;
	CheckBox *entity_blocking_checkbox = nullptr;
	LineEdit *item_name_edit = nullptr;
	LineEdit *item_glyph_edit = nullptr;
	CheckBox *item_stackable_checkbox = nullptr;
	SpinBox *item_weight_spin_box = nullptr;
	ItemList *item_catalog_item_list = nullptr;
	LineEdit *world_name_edit = nullptr;
	SpinBox *seed_spin_box = nullptr;
	SpinBox *tick_spin_box = nullptr;
	LineEdit *world_font_path_edit = nullptr;
	SpinBox *world_font_size_spin_box = nullptr;
	LineEdit *entity_type_edit = nullptr;
	OptionButton *tile_type_option = nullptr;
	OptionButton *entity_type_option = nullptr;
	SpinBox *entity_x_spin_box = nullptr;
	SpinBox *entity_y_spin_box = nullptr;
	SpinBox *entity_z_spin_box = nullptr;
	SpinBox *tile_x_spin_box = nullptr;
	SpinBox *tile_y_spin_box = nullptr;
	SpinBox *tile_z_spin_box = nullptr;
	SpinBox *snapshot_origin_x_spin_box = nullptr;
	SpinBox *snapshot_origin_y_spin_box = nullptr;
	SpinBox *snapshot_layer_spin_box = nullptr;
	SpinBox *snapshot_width_spin_box = nullptr;
	SpinBox *snapshot_height_spin_box = nullptr;
	RichTextLabel *snapshot_label = nullptr;
	String pending_save_type;
	String pending_world_state_action;
	String last_error_text;
	TwimWorldServer *world_server = nullptr;
	Array tile_catalog_entries;
	Array entity_catalog_entries;
	Array item_catalog_entries;
	int32_t selected_tile_catalog_index = -1;
	int32_t selected_entity_catalog_index = -1;
	int32_t selected_item_catalog_index = -1;

	void _destroy_world_server();

	void _refresh_summary();
	void _refresh_world_status();
	void _refresh_tile_catalog_status();
	void _refresh_entity_catalog_status();
	void _refresh_item_catalog_status();
	void _request_save_tile_set();
	void _request_save_entity_catalog();
	void _request_save_item_catalog();
	void _request_save_world_profile();
	void _save_resource_to_path(const String &p_path);
	void _add_tile_catalog_entry();
	void _update_selected_tile_catalog_entry();
	void _remove_selected_tile_catalog_entry();
	void _move_selected_tile_catalog_entry_up();
	void _move_selected_tile_catalog_entry_down();
	void _on_tile_catalog_item_selected(int32_t p_index);
	void _refresh_tile_catalog_item_list();
	void _clear_tile_catalog_entries();
	void _save_tile_catalog();
	void _load_tile_catalog();
	void _add_entity_catalog_entry();
	void _update_selected_entity_catalog_entry();
	void _remove_selected_entity_catalog_entry();
	void _move_selected_entity_catalog_entry_up();
	void _move_selected_entity_catalog_entry_down();
	void _on_entity_catalog_item_selected(int32_t p_index);
	void _refresh_entity_catalog_item_list();
	void _clear_entity_catalog_entries();
	void _save_entity_catalog();
	void _load_entity_catalog();
	void _add_item_catalog_entry();
	void _update_selected_item_catalog_entry();
	void _remove_selected_item_catalog_entry();
	void _move_selected_item_catalog_entry_up();
	void _move_selected_item_catalog_entry_down();
	void _on_item_catalog_item_selected(int32_t p_index);
	void _refresh_item_catalog_item_list();
	void _clear_item_catalog_entries();
	void _save_item_catalog();
	void _load_item_catalog();
	void _refresh_type_options();

	void _create_world();
	void _unload_world();
	void _step_world();
	void _spawn_entity();
	void _set_tile_type();
	void _refresh_snapshot();
	void _request_save_world_state();
	void _request_load_world_state();
	void _process_world_state_dialog_path(const String &p_path);
	void _apply_world_render_settings();

protected:
	void _notification(int p_what);

public:
	String get_plugin_name() const override;
	bool has_main_screen() const override { return true; }
	void edit(Object *p_object) override;
	bool handles(Object *p_object) const override;
	void make_visible(bool p_visible) override;

	TwimEditorPlugin();
	~TwimEditorPlugin() override;
};

