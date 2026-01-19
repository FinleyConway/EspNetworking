#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger
{
public:
    static void init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);

        s_logger = std::make_shared<spdlog::logger>("Server", console_sink);
        s_logger->set_level(spdlog::level::trace);
        s_logger->flush_on(spdlog::level::trace);
        spdlog::register_logger(s_logger);
    }

    static std::shared_ptr<spdlog::logger>& get_logger() {
        return s_logger;
    }

private:
    inline static std::shared_ptr<spdlog::logger> s_logger;
};

#define LOG_TRACE(...)    ::Logger::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::Logger::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::Logger::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::Logger::get_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Logger::get_logger()->critical(__VA_ARGS__)