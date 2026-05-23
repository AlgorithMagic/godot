#include "entity_type_resource.h"

#include "core/object/class_db.h"

namespace {
bool is_valid_unicode_scalar(int32_t p_codepoint) {
	if (p_codepoint < 0 || p_codepoint > 0x10FFFF) {
		return false;
	}

	return !(p_codepoint >= 0xD800 && p_codepoint <= 0xDFFF);
}
} // namespace

void TwimEntityTypeResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_entity_name", "entity_name"), &TwimEntityTypeResource::set_entity_name);
	ClassDB::bind_method(D_METHOD("get_entity_name"), &TwimEntityTypeResource::get_entity_name);
	ClassDB::bind_method(D_METHOD("set_glyph_id", "glyph_id"), &TwimEntityTypeResource::set_glyph_id);
	ClassDB::bind_method(D_METHOD("get_glyph_id"), &TwimEntityTypeResource::get_glyph_id);
	ClassDB::bind_method(D_METHOD("set_glyph", "glyph"), &TwimEntityTypeResource::set_glyph);
	ClassDB::bind_method(D_METHOD("get_glyph"), &TwimEntityTypeResource::get_glyph);
	ClassDB::bind_method(D_METHOD("set_blocking", "blocking"), &TwimEntityTypeResource::set_blocking);
	ClassDB::bind_method(D_METHOD("is_blocking"), &TwimEntityTypeResource::is_blocking);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "entity_name"), "set_entity_name", "get_entity_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "glyph_id", PROPERTY_HINT_RANGE, "0,1114111,1"), "set_glyph_id", "get_glyph_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "glyph"), "set_glyph", "get_glyph");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocking"), "set_blocking", "is_blocking");
}

void TwimEntityTypeResource::set_entity_name(const StringName &p_entity_name) {
	entity_name = p_entity_name;
}

StringName TwimEntityTypeResource::get_entity_name() const {
	return entity_name;
}

void TwimEntityTypeResource::set_glyph_id(int32_t p_glyph_id) {
	if (!is_valid_unicode_scalar(p_glyph_id)) {
		glyph_id = '@';
		return;
	}

	glyph_id = p_glyph_id;
}

int32_t TwimEntityTypeResource::get_glyph_id() const {
	return glyph_id;
}

void TwimEntityTypeResource::set_glyph(const String &p_glyph) {
	if (p_glyph.is_empty()) {
		glyph_id = '@';
		return;
	}

	set_glyph_id(static_cast<int32_t>(p_glyph[0]));
}

String TwimEntityTypeResource::get_glyph() const {
	if (!is_valid_unicode_scalar(glyph_id) || glyph_id == 0) {
		return String();
	}

	return String::chr(static_cast<char32_t>(glyph_id));
}

void TwimEntityTypeResource::set_blocking(bool p_blocking) {
	blocking = p_blocking;
}

bool TwimEntityTypeResource::is_blocking() const {
	return blocking;
}

