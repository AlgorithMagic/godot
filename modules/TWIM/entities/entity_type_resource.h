#pragma once

#include "core/io/resource.h"

class TwimEntityTypeResource : public Resource {
	GDCLASS(TwimEntityTypeResource, Resource);

private:
	StringName entity_name;
	int32_t glyph_id = '@';
	bool blocking = false;

protected:
	static void _bind_methods();

public:
	void set_entity_name(const StringName &p_entity_name);
	StringName get_entity_name() const;

	void set_glyph_id(int32_t p_glyph_id);
	int32_t get_glyph_id() const;

	void set_glyph(const String &p_glyph);
	String get_glyph() const;

	void set_blocking(bool p_blocking);
	bool is_blocking() const;
};

