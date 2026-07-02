// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#include <zephyrus/log.hpp>
#include <zephyrus/log/logging_manager.hpp>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace zephyrus::log {
logging_manager::result_type logging_manager::init_impl() noexcept {
    using namespace spdlog;
    if (get(ZEPHYRUS_LOGGER_NAME) || spdlog::thread_pool()) { // already exists
        return std::unexpected{"logger already initialized"};
    }
    init_thread_pool(8192, 1);
    auto console_sink{std::make_shared<sinks::stdout_color_sink_mt>()};
    auto file_sink{std::make_shared<sinks::rotating_file_sink_mt>(
        "zephyrus.log", 10240, 10, true)};
    auto zephyrus_logger{std::make_shared<async_logger>(
        ZEPHYRUS_LOGGER_NAME, sinks_init_list{console_sink, file_sink},
        thread_pool())};
    zephyrus_logger->set_level(
#ifndef NDEBUG
        spdlog::level::trace
#else
        spdlog::level::info
#endif
    );
    register_logger(zephyrus_logger);
    spdlog::set_default_logger(zephyrus_logger);
    return {};
}
} // namespace zephyrus::log
