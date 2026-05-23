#pragma once

#include <atomic>
#include <cstdint>

namespace twim {

// Tracks authoritative generation IDs so stale async results can be rejected safely.
class TwimAsyncGenerationGuard {
public:
	uint64_t get_current_generation() const {
		return generation_id.load(std::memory_order_acquire);
	}

	uint64_t begin_new_generation() {
		return generation_id.fetch_add(1, std::memory_order_acq_rel) + 1;
	}

	void cancel_pending_work() {
		generation_id.fetch_add(1, std::memory_order_acq_rel);
	}

	bool is_generation_current(uint64_t p_generation_id) const {
		return p_generation_id == get_current_generation();
	}

	bool try_accept_staged_result(uint64_t p_generation_id) const {
		return is_generation_current(p_generation_id);
	}

private:
	std::atomic<uint64_t> generation_id{ 1 };
};

} // namespace twim

