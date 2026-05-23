#include "world_profile_resource.h"

#include "core/object/class_db.h"

void TwimWorldProfileResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tick_rate_hz", "tick_rate_hz"), &TwimWorldProfileResource::set_tick_rate_hz);
	ClassDB::bind_method(D_METHOD("get_tick_rate_hz"), &TwimWorldProfileResource::get_tick_rate_hz);
	ClassDB::bind_method(D_METHOD("set_chunk_size_x", "chunk_size_x"), &TwimWorldProfileResource::set_chunk_size_x);
	ClassDB::bind_method(D_METHOD("get_chunk_size_x"), &TwimWorldProfileResource::get_chunk_size_x);
	ClassDB::bind_method(D_METHOD("set_chunk_size_y", "chunk_size_y"), &TwimWorldProfileResource::set_chunk_size_y);
	ClassDB::bind_method(D_METHOD("get_chunk_size_y"), &TwimWorldProfileResource::get_chunk_size_y);
	ClassDB::bind_method(D_METHOD("set_chunk_size_z", "chunk_size_z"), &TwimWorldProfileResource::set_chunk_size_z);
	ClassDB::bind_method(D_METHOD("get_chunk_size_z"), &TwimWorldProfileResource::get_chunk_size_z);
	ClassDB::bind_method(D_METHOD("set_glyph_font_resource_path", "glyph_font_resource_path"), &TwimWorldProfileResource::set_glyph_font_resource_path);
	ClassDB::bind_method(D_METHOD("get_glyph_font_resource_path"), &TwimWorldProfileResource::get_glyph_font_resource_path);
	ClassDB::bind_method(D_METHOD("set_glyph_font_size", "glyph_font_size"), &TwimWorldProfileResource::set_glyph_font_size);
	ClassDB::bind_method(D_METHOD("get_glyph_font_size"), &TwimWorldProfileResource::get_glyph_font_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate_hz", PROPERTY_HINT_RANGE, "1,2048,1,or_greater"), "set_tick_rate_hz", "get_tick_rate_hz");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size_x", PROPERTY_HINT_RANGE, "1,512,1,or_greater"), "set_chunk_size_x", "get_chunk_size_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size_y", PROPERTY_HINT_RANGE, "1,512,1,or_greater"), "set_chunk_size_y", "get_chunk_size_y");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size_z", PROPERTY_HINT_RANGE, "1,512,1,or_greater"), "set_chunk_size_z", "get_chunk_size_z");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "glyph_font_resource_path", PROPERTY_HINT_FILE, "*.ttf,*.otf,*.font,*.tres"), "set_glyph_font_resource_path", "get_glyph_font_resource_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "glyph_font_size", PROPERTY_HINT_RANGE, "1,512,1,or_greater"), "set_glyph_font_size", "get_glyph_font_size");
}

void TwimWorldProfileResource::set_tick_rate_hz(int32_t p_tick_rate_hz) {
	tick_rate_hz = p_tick_rate_hz;
}

int32_t TwimWorldProfileResource::get_tick_rate_hz() const {
	return tick_rate_hz;
}

void TwimWorldProfileResource::set_chunk_size_x(int32_t p_chunk_size_x) {
	chunk_size_x = p_chunk_size_x;
}

int32_t TwimWorldProfileResource::get_chunk_size_x() const {
	return chunk_size_x;
}

void TwimWorldProfileResource::set_chunk_size_y(int32_t p_chunk_size_y) {
	chunk_size_y = p_chunk_size_y;
}

int32_t TwimWorldProfileResource::get_chunk_size_y() const {
	return chunk_size_y;
}

void TwimWorldProfileResource::set_chunk_size_z(int32_t p_chunk_size_z) {
	chunk_size_z = p_chunk_size_z;
}

int32_t TwimWorldProfileResource::get_chunk_size_z() const {
	return chunk_size_z;
}

void TwimWorldProfileResource::set_glyph_font_resource_path(const String &p_glyph_font_resource_path) {
	glyph_font_resource_path = p_glyph_font_resource_path;
}

String TwimWorldProfileResource::get_glyph_font_resource_path() const {
	return glyph_font_resource_path;
}

void TwimWorldProfileResource::set_glyph_font_size(int32_t p_glyph_font_size) {
	glyph_font_size = p_glyph_font_size;
}

int32_t TwimWorldProfileResource::get_glyph_font_size() const {
	return glyph_font_size;
}

