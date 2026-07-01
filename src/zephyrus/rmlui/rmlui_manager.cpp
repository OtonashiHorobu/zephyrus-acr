// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#include <zephyrus/rmlui/rmlui_manager.hpp>

#include <RmlUi/Core/Core.h>

namespace zephyrus::rmlui {
rmlui_manager::result_type rmlui_manager::init_impl() noexcept {
    Rml::SetFileInterface(file_interface_.get());
    return {};
}

rmlui_manager::~rmlui_manager() noexcept { Rml::SetFileInterface(nullptr); }
} // namespace zephyrus::rmlui
