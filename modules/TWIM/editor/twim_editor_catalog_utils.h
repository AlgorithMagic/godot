#pragma once

#include "core/error/error_list.h"
#include "core/variant/array.h"

namespace twim::editor::catalog_utils {

inline bool is_valid_index(const Array &p_entries, int32_t p_index) {
	return p_index >= 0 && p_index < p_entries.size();
}

inline Error swap_entries(Array &p_entries, int32_t p_a, int32_t p_b) {
	if (!is_valid_index(p_entries, p_a) || !is_valid_index(p_entries, p_b)) {
		return ERR_INVALID_PARAMETER;
	}
	if (p_a == p_b) {
		return OK;
	}

	const Variant entry_a = p_entries[p_a];
	p_entries[p_a] = p_entries[p_b];
	p_entries[p_b] = entry_a;
	return OK;
}

inline Error remove_entry(Array &p_entries, int32_t p_index) {
	if (!is_valid_index(p_entries, p_index)) {
		return ERR_INVALID_PARAMETER;
	}

	p_entries.remove_at(p_index);
	return OK;
}

} // namespace twim::editor::catalog_utils


