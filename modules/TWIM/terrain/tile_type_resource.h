#pragma once

#include "core/io/resource.h"

class TwimTileTypeResource : public Resource {
	GDCLASS(TwimTileTypeResource, Resource);

private:
	StringName tile_name;
	bool solid = false;
	bool opaque = false;
	bool liquid = false;
	int32_t glyph_id = 0;
	int64_t foreground_rgba = 0xffffffff;
	int64_t background_rgba = 0x000000ff;

protected:
	static void _bind_methods();

public:
	void set_tile_name(const StringName &p_tile_name);
	StringName get_tile_name() const;

	void set_solid(bool p_solid);
	bool is_solid() const;

	void set_opaque(bool p_opaque);
	bool is_opaque() const;

	void set_liquid(bool p_liquid);
	bool is_liquid() const;

	void set_glyph_id(int32_t p_glyph_id);
	int32_t get_glyph_id() const;

	void set_glyph(const String &p_glyph);
	String get_glyph() const;

	void set_foreground_rgba(int64_t p_foreground_rgba);
	int64_t get_foreground_rgba() const;

	void set_background_rgba(int64_t p_background_rgba);
	int64_t get_background_rgba() const;
};

