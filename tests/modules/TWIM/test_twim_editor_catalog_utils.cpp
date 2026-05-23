#include "tests/test_macros.h"

TEST_FORCE_LINK(test_twim_editor_catalog_utils)

#include "modules/TWIM/editor/twim_editor_catalog_utils.h"
#include "modules/TWIM/simulation/twim_async_generation_guard.h"

namespace TestTwim {

TEST_CASE("[TWIM] Catalog utils validate indices and support swaps/removals") {
	Array entries;
	entries.push_back(String("a"));
	entries.push_back(String("b"));
	entries.push_back(String("c"));

	CHECK(twim::editor::catalog_utils::is_valid_index(entries, 0));
	CHECK(!twim::editor::catalog_utils::is_valid_index(entries, -1));
	CHECK(!twim::editor::catalog_utils::is_valid_index(entries, 3));

	CHECK_EQ(twim::editor::catalog_utils::swap_entries(entries, 0, 2), OK);
	CHECK_EQ(String(entries[0]), String("c"));
	CHECK_EQ(String(entries[2]), String("a"));

	CHECK_EQ(twim::editor::catalog_utils::remove_entry(entries, 1), OK);
	CHECK_EQ(entries.size(), 2);
	CHECK_EQ(String(entries[1]), String("a"));

	CHECK_EQ(twim::editor::catalog_utils::swap_entries(entries, -1, 0), ERR_INVALID_PARAMETER);
	CHECK_EQ(twim::editor::catalog_utils::remove_entry(entries, 5), ERR_INVALID_PARAMETER);
}

TEST_CASE("[TWIM] Async generation guard rejects stale staged results") {
	twim::TwimAsyncGenerationGuard guard;
	const uint64_t initial_generation = guard.get_current_generation();
	CHECK(guard.is_generation_current(initial_generation));
	CHECK(guard.try_accept_staged_result(initial_generation));

	const uint64_t next_generation = guard.begin_new_generation();
	CHECK(next_generation > initial_generation);
	CHECK(!guard.try_accept_staged_result(initial_generation));
	CHECK(guard.try_accept_staged_result(next_generation));

	guard.cancel_pending_work();
	CHECK(!guard.try_accept_staged_result(next_generation));
	CHECK(guard.is_generation_current(guard.get_current_generation()));
}

} // namespace TestTwim

