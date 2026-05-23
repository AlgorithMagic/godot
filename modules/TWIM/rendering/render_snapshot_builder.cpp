#include "render_snapshot_builder.h"

namespace twim {

RenderSnapshot RenderSnapshotBuilder::build(
		const chunk_store &p_chunks,
		const TileDefinitionRegistry &p_tile_definitions,
		Vector3i p_origin,
		Vector2i p_size,
		int32_t p_z_layer,
		uint64_t p_tick_index,
		const String &p_glyph_font_resource_path,
		int32_t p_glyph_font_size) const {
	RenderSnapshot snapshot;
	snapshot.origin = p_origin;
	snapshot.size = p_size;
	snapshot.z_layer = p_z_layer;
	snapshot.tick_index = p_tick_index;
	snapshot.glyph_font_resource_path = p_glyph_font_resource_path;
	snapshot.glyph_font_size = p_glyph_font_size;

	if (p_size.x <= 0 || p_size.y <= 0) {
		snapshot.reason = "Snapshot size must be positive.";
		return snapshot;
	}

	if (p_size.x > 512 || p_size.y > 512) {
		snapshot.reason = "Debug snapshot size exceeds 512x512 safety limit.";
		return snapshot;
	}

	snapshot.cells.resize(p_size.x * p_size.y);
	for (int32_t y = 0; y < p_size.y; ++y) {
		for (int32_t x = 0; x < p_size.x; ++x) {
			const Vector3i world_tile_coord(p_origin.x + x, p_origin.y + y, p_z_layer);
			const tile_type_id current_tile_type_id = p_chunks.get_tile_type_id(tile_coord::from_vector3_i(world_tile_coord));
			const TileDefinition *definition = p_tile_definitions.get_definition(current_tile_type_id);

			Dictionary cell;
			cell["x"] = x;
			cell["y"] = y;
			cell["world_coord"] = world_tile_coord;
			cell["tile_type_id"] = static_cast<int64_t>(current_tile_type_id.value);

			if (definition != nullptr) {
				cell["tile_name"] = definition->name;
				cell["glyph_id"] = static_cast<int64_t>(definition->glyph_id);
				cell["foreground_rgba"] = static_cast<int64_t>(definition->foreground_rgba);
				cell["background_rgba"] = static_cast<int64_t>(definition->background_rgba);
				cell["solid"] = definition->solid;
				cell["opaque"] = definition->opaque;
				cell["liquid"] = definition->liquid;
			} else {
				cell["tile_name"] = StringName("invalid");
				cell["glyph_id"] = 0;
				cell["foreground_rgba"] = 0;
				cell["background_rgba"] = 0;
				cell["solid"] = false;
				cell["opaque"] = false;
				cell["liquid"] = false;
			}

			snapshot.cells[y * p_size.x + x] = cell;
		}
	}

	snapshot.available = true;
	return snapshot;
}

} // namespace twim

