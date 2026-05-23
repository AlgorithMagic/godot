#pragma once

namespace twim {

void register_project_settings();

#ifdef TOOLS_ENABLED
void register_editor_settings();
#endif

} // namespace twim

