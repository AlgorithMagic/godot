#pragma once

#include "core/io/resource.h"

class TwimEntityCatalogResource : public Resource {
	GDCLASS(TwimEntityCatalogResource, Resource);

private:
	Array entity_types;

protected:
	static void _bind_methods();

public:
	void set_entity_types(const Array &p_entity_types);
	Array get_entity_types() const;
};

