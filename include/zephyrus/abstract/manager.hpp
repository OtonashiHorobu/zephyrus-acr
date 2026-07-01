// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#pragma once

#include <expected>
#include <string>

namespace zephyrus::abstract {
template <typename Derived> class manager {
  public:
    using result_type = std::expected<void, std::string>;

    result_type init() noexcept {
        return static_cast<Derived *>(this)->init_impl();
    }

  protected:
    manager() = default;
    ~manager() = default;
};
} // namespace zephyrus::abstract
