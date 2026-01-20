#pragma once

#include <memory>
#include <vector>
#include <string>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>

// https://stackoverflow.com/questions/59141827/grabbing-printed-statements-from-console-c
// https://deepwiki.com/gabime/spdlog/10.3-external-sinks-and-extensions
template<typename TMutex>
class logger_sink : public spdlog::sinks::base_sink<TMutex>
{
public:
    std::vector<std::string> get_buffer() {
        std::lock_guard<TMutex> lock(m_mutex);
        return m_buffer;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);

        std::lock_guard<TMutex> lock(m_mutex);

        // clear a chunk of the buffer to avoid over storing
        size_t max_buffer_size = 100;
        if (m_buffer.size() > max_buffer_size) {
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + (max_buffer_size / 4));
        }

        // capture log
        m_buffer.emplace_back(formatted.data(), formatted.size());
    }

    void flush_() override { }

private:
    std::vector<std::string> m_buffer;
    TMutex m_mutex;
};

class logger
{
public:
    static void init() {
        s_logger_sink = std::make_shared<logger_sink<std::mutex>>();
        s_logger_sink->set_pattern("%^[%T] %n: %v%$");

        s_logger = std::make_shared<spdlog::logger>("Server", s_logger_sink);
        s_logger->set_level(spdlog::level::trace);
        s_logger->flush_on(spdlog::level::trace);

        spdlog::register_logger(s_logger);
    }

    static std::shared_ptr<spdlog::logger>& get_logger() {
        return s_logger;
    }

    static std::shared_ptr<logger_sink<std::mutex>>& get_sink() {
        return s_logger_sink;
    }

private:
    inline static std::shared_ptr<spdlog::logger> s_logger;
    inline static std::shared_ptr<logger_sink<std::mutex>> s_logger_sink;
};

#define LOG_TRACE(...)    ::logger::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::logger::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::logger::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::logger::get_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::logger::get_logger()->critical(__VA_ARGS__)