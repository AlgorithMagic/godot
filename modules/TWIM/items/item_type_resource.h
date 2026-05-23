#pragma once

#include "core/io/resource.h"

class TwimItemTypeResource : public Resource {
	GDCLASS(TwimItemTypeResource, Resource);

private:
	StringName item_name;
	int32_t glyph_id = '!';
	bool stackable = true;
	double unit_weight = 1.0;

protected:
	static void _bind_methods();

public:
	void set_item_name(const StringName &p_item_name);
	StringName get_item_name() const;

	void set_glyph_id(int32_t p_glyph_id);
	int32_t get_glyph_id() const;

	void set_glyph(const String &p_glyph);
	String get_glyph() const;

	void set_stackable(bool p_stackable);
	bool is_stackable() const;

	void set_unit_weight(double p_unit_weight);
	double get_unit_weight() const;
};

