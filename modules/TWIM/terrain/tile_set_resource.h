#pragma once

#include "core/io/resource.h"

class TwimTileSetResource : public Resource {
	GDCLASS(TwimTileSetResource, Resource);

private:
	Array tile_types;

protected:
	static void _bind_methods();

public:
	void set_tile_types(const Array &p_tile_types);
	Array get_tile_types() const;
};

