#ifndef __LOGGER_H_CUSTOM__
#define __LOGGER_H_CUSTOM__
#include <cstddef>
#include <sstream>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <vector>

enum class log_level {
    info,
    warning,
    severe,
    fatal
};

#define cal_HERE() \
    std::make_pair(std::string(__FILE__), int(__LINE__))
std::string format_here(std::pair <std::string, int> const & here);


class logger;
class logstream : public std::ostringstream {
    logger& m_logger;
    log_level m_level;

    const std::string get_level_string();
    const std::string get_time_string();
public:
    logstream(logger& log, log_level);
    ~logstream();
};


class logger {
    std::queue<std::string> m_q;
    std::mutex m_q_mu;

    std::mutex m_stdout_mu;
    std::mutex m_stderr_mu;

    std::thread m_print_thread;
    bool m_print = true;
    static void print_routine(logger *instance, std::chrono::duration<double, std::milli> interval);

    logger();
    ~logger();
public:
    logger(logger const&) = delete;
    void operator=(logger const&) = delete;

    static logstream log(log_level level = log_level::info) {
        static logger m_handler;
        return logstream(m_handler, level);
    }

    void push(std::string fmt_msg);
};


#endif // __LOGGER_H_CUSTOM__
