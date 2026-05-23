#include "tile_type_resource.h"

#include "core/object/class_db.h"

namespace {
bool is_valid_unicode_scalar(int32_t p_codepoint) {
	if (p_codepoint < 0 || p_codepoint > 0x10FFFF) {
		return false;
	}

	return !(p_codepoint >= 0xD800 && p_codepoint <= 0xDFFF);
}
} // namespace

void TwimTileTypeResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tile_name", "tile_name"), &TwimTileTypeResource::set_tile_name);
	ClassDB::bind_method(D_METHOD("get_tile_name"), &TwimTileTypeResource::get_tile_name);
	ClassDB::bind_method(D_METHOD("set_solid", "solid"), &TwimTileTypeResource::set_solid);
	ClassDB::bind_method(D_METHOD("is_solid"), &TwimTileTypeResource::is_solid);
	ClassDB::bind_method(D_METHOD("set_opaque", "opaque"), &TwimTileTypeResource::set_opaque);
	ClassDB::bind_method(D_METHOD("is_opaque"), &TwimTileTypeResource::is_opaque);
	ClassDB::bind_method(D_METHOD("set_liquid", "liquid"), &TwimTileTypeResource::set_liquid);
	ClassDB::bind_method(D_METHOD("is_liquid"), &TwimTileTypeResource::is_liquid);
	ClassDB::bind_method(D_METHOD("set_glyph_id", "glyph_id"), &TwimTileTypeResource::set_glyph_id);
	ClassDB::bind_method(D_METHOD("get_glyph_id"), &TwimTileTypeResource::get_glyph_id);
	ClassDB::bind_method(D_METHOD("set_glyph", "glyph"), &TwimTileTypeResource::set_glyph);
	ClassDB::bind_method(D_METHOD("get_glyph"), &TwimTileTypeResource::get_glyph);
	ClassDB::bind_method(D_METHOD("set_foreground_rgba", "foreground_rgba"), &TwimTileTypeResource::set_foreground_rgba);
	ClassDB::bind_method(D_METHOD("get_foreground_rgba"), &TwimTileTypeResource::get_foreground_rgba);
	ClassDB::bind_method(D_METHOD("set_background_rgba", "background_rgba"), &TwimTileTypeResource::set_background_rgba);
	ClassDB::bind_method(D_METHOD("get_background_rgba"), &TwimTileTypeResource::get_background_rgba);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "tile_name"), "set_tile_name", "get_tile_name");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "solid"), "set_solid", "is_solid");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "opaque"), "set_opaque", "is_opaque");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "liquid"), "set_liquid", "is_liquid");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "glyph_id", PROPERTY_HINT_RANGE, "0,1114111,1"), "set_glyph_id", "get_glyph_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "glyph"), "set_glyph", "get_glyph");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "foreground_rgba", PROPERTY_HINT_RANGE, "0,4294967295,1"), "set_foreground_rgba", "get_foreground_rgba");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "background_rgba", PROPERTY_HINT_RANGE, "0,4294967295,1"), "set_background_rgba", "get_background_rgba");
}

void TwimTileTypeResource::set_tile_name(const StringName &p_tile_name) {
	tile_name = p_tile_name;
}

StringName TwimTileTypeResource::get_tile_name() const {
	return tile_name;
}

void TwimTileTypeResource::set_solid(bool p_solid) {
	solid = p_solid;
}

bool TwimTileTypeResource::is_solid() const {
	return solid;
}

void TwimTileTypeResource::set_opaque(bool p_opaque) {
	opaque = p_opaque;
}

bool TwimTileTypeResource::is_opaque() const {
	return opaque;
}

void TwimTileTypeResource::set_liquid(bool p_liquid) {
	liquid = p_liquid;
}

bool TwimTileTypeResource::is_liquid() const {
	return liquid;
}

void TwimTileTypeResource::set_glyph_id(int32_t p_glyph_id) {
	if (!is_valid_unicode_scalar(p_glyph_id)) {
		glyph_id = 0;
		return;
	}

	glyph_id = p_glyph_id;
}

int32_t TwimTileTypeResource::get_glyph_id() const {
	return glyph_id;
}

void TwimTileTypeResource::set_glyph(const String &p_glyph) {
	if (p_glyph.is_empty()) {
		glyph_id = 0;
		return;
	}

	const char32_t codepoint = p_glyph[0];
	set_glyph_id(static_cast<int32_t>(codepoint));
}

String TwimTileTypeResource::get_glyph() const {
	if (!is_valid_unicode_scalar(glyph_id) || glyph_id == 0) {
		return String();
	}

	return String::chr(static_cast<char32_t>(glyph_id));
}

void TwimTileTypeResource::set_foreground_rgba(int64_t p_foreground_rgba) {
	foreground_rgba = p_foreground_rgba;
}

int64_t TwimTileTypeResource::get_foreground_rgba() const {
	return foreground_rgba;
}

void TwimTileTypeResource::set_background_rgba(int64_t p_background_rgba) {
	background_rgba = p_background_rgba;
}

int64_t TwimTileTypeResource::get_background_rgba() const {
	return background_rgba;
}

