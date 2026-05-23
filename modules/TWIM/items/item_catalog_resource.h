#pragma once

#include "core/io/resource.h"

class TwimItemCatalogResource : public Resource {
	GDCLASS(TwimItemCatalogResource, Resource);

private:
	Array item_types;

protected:
	static void _bind_methods();

public:
	void set_item_types(const Array &p_item_types);
	Array get_item_types() const;
};

