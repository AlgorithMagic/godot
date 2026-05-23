#pragma once

#include "core/io/resource.h"

class TwimWorldProfileResource : public Resource {
	GDCLASS(TwimWorldProfileResource, Resource);

private:
	int32_t tick_rate_hz = 120;
	int32_t chunk_size_x = 32;
	int32_t chunk_size_y = 32;
	int32_t chunk_size_z = 16;
	String glyph_font_resource_path;
	int32_t glyph_font_size = 16;

protected:
	static void _bind_methods();

public:
	void set_tick_rate_hz(int32_t p_tick_rate_hz);
	int32_t get_tick_rate_hz() const;

	void set_chunk_size_x(int32_t p_chunk_size_x);
	int32_t get_chunk_size_x() const;

	void set_chunk_size_y(int32_t p_chunk_size_y);
	int32_t get_chunk_size_y() const;

	void set_chunk_size_z(int32_t p_chunk_size_z);
	int32_t get_chunk_size_z() const;

	void set_glyph_font_resource_path(const String &p_glyph_font_resource_path);
	String get_glyph_font_resource_path() const;

	void set_glyph_font_size(int32_t p_glyph_font_size);
	int32_t get_glyph_font_size() const;
};

