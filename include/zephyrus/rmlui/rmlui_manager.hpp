// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#pragma once

#include <memory>

#include <zephyrus/abstract/manager.hpp>
#include <zephyrus/rmlui/cmrc_file_interface.hpp>

namespace zephyrus::rmlui {
class rmlui_manager : public abstract::manager<rmlui_manager> {
  public:
    explicit rmlui_manager(std::shared_ptr<cmrc_file_interface> file_interface)
        : manager<rmlui_manager>{}, file_interface_{std::move(file_interface)} {
    }
    ~rmlui_manager() noexcept;

    result_type init_impl() noexcept;

  private:
    std::shared_ptr<cmrc_file_interface> file_interface_;
};
} // namespace zephyrus::rmlui
