// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#pragma once

#include <zephyrus/abstract/manager.hpp>

namespace zephyrus::log {
class logging_manager : public abstract::manager<logging_manager> {
  public:
    result_type init_impl() noexcept;
    logging_manager() : manager<logging_manager>{} {}
};
} // namespace zephyrus::log
