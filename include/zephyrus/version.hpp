// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#pragma once

#include <fmt/format.h>
#include <string>

#include <zephyrus/platform/extern.hpp>

namespace zephyrus {
ZEPHYRUS_EXTERN extern const char *release_version;
ZEPHYRUS_EXTERN extern const char *branch_name;
ZEPHYRUS_EXTERN extern const char *commit_hash;

inline const std::string &full_version() noexcept {
    static const std::string full_version_static{
        fmt::format("Zephyrus ACR {} [git {}:{}]"
#if !defined(NDEBUG) || defined(_DEBUG)
                    " [debug]"
#endif
                    ,
                    release_version, branch_name, commit_hash)};
    return full_version_static;
}
} // namespace zephyrus
