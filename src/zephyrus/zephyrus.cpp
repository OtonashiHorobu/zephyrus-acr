// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#include <zephyrus/asset/ui.hpp>
#include <zephyrus/log.hpp>
#include <zephyrus/log/logging_manager.hpp>
#include <zephyrus/rmlui/rmlui_manager.hpp>
#include <zephyrus/version.hpp>
#include <zephyrus/zephyrus.hpp>

#include <fmt/format.h>

#include <boost/di.hpp>

namespace zephyrus {
int zephyrus::init() {
    if (auto result{logging_manager_.init()}; !result) {
        fmt::println("zephyrus: failed to initialize logging system: {}",
                     result.error());
        return 1;
    }
    ZEPHYRUS_INFO(version::full_version());
    if (auto result{rmlui_manager_.init()}; !result) {
        ZEPHYRUS_CRITICAL("failed to initialize rmlui: {}", result.error());
        return 1;
    }
    return 0;
}

zephyrus &g_zephyrus() noexcept {
    using namespace boost::di;
    static zephyrus static_zephyrus{
        make_injector(bind<>.to(cmrc::zephyrus::asset::ui::get_filesystem()))
            .create<zephyrus>()};
    return static_zephyrus;
}
} // namespace zephyrus
