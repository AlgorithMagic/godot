#include "item_type_resource.h"

#include "core/object/class_db.h"

namespace {
bool is_valid_unicode_scalar(int32_t p_codepoint) {
	if (p_codepoint < 0 || p_codepoint > 0x10FFFF) {
		return false;
	}

	return !(p_codepoint >= 0xD800 && p_codepoint <= 0xDFFF);
}
} // namespace

void TwimItemTypeResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item_name", "item_name"), &TwimItemTypeResource::set_item_name);
	ClassDB::bind_method(D_METHOD("get_item_name"), &TwimItemTypeResource::get_item_name);
	ClassDB::bind_method(D_METHOD("set_glyph_id", "glyph_id"), &TwimItemTypeResource::set_glyph_id);
	ClassDB::bind_method(D_METHOD("get_glyph_id"), &TwimItemTypeResource::get_glyph_id);
	ClassDB::bind_method(D_METHOD("set_glyph", "glyph"), &TwimItemTypeResource::set_glyph);
	ClassDB::bind_method(D_METHOD("get_glyph"), &TwimItemTypeResource::get_glyph);
	ClassDB::bind_method(D_METHOD("set_stackable", "stackable"), &TwimItemTypeResource::set_stackable);
	ClassDB::bind_method(D_METHOD("is_stackable"), &TwimItemTypeResource::is_stackable);
	ClassDB::bind_method(D_METHOD("set_unit_weight", "unit_weight"), &TwimItemTypeResource::set_unit_weight);
	ClassDB::bind_method(D_METHOD("get_unit_weight"), &TwimItemTypeResource::get_unit_weight);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_name"), "set_item_name", "get_item_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "glyph_id", PROPERTY_HINT_RANGE, "0,1114111,1"), "set_glyph_id", "get_glyph_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "glyph"), "set_glyph", "get_glyph");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stackable"), "set_stackable", "is_stackable");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "unit_weight", PROPERTY_HINT_RANGE, "0.0,1000000.0,0.01,or_greater"), "set_unit_weight", "get_unit_weight");
}

void TwimItemTypeResource::set_item_name(const StringName &p_item_name) {
	item_name = p_item_name;
}

StringName TwimItemTypeResource::get_item_name() const {
	return item_name;
}

void TwimItemTypeResource::set_glyph_id(int32_t p_glyph_id) {
	if (!is_valid_unicode_scalar(p_glyph_id)) {
		glyph_id = '!';
		return;
	}

	glyph_id = p_glyph_id;
}

int32_t TwimItemTypeResource::get_glyph_id() const {
	return glyph_id;
}

void TwimItemTypeResource::set_glyph(const String &p_glyph) {
	if (p_glyph.is_empty()) {
		glyph_id = '!';
		return;
	}

	set_glyph_id(static_cast<int32_t>(p_glyph[0]));
}

String TwimItemTypeResource::get_glyph() const {
	if (!is_valid_unicode_scalar(glyph_id) || glyph_id == 0) {
		return String();
	}

	return String::chr(static_cast<char32_t>(glyph_id));
}

void TwimItemTypeResource::set_stackable(bool p_stackable) {
	stackable = p_stackable;
}

bool TwimItemTypeResource::is_stackable() const {
	return stackable;
}

void TwimItemTypeResource::set_unit_weight(double p_unit_weight) {
	unit_weight = p_unit_weight;
}

double TwimItemTypeResource::get_unit_weight() const {
	return unit_weight;
}

