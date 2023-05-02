#include "Logger.h"
#include <ctime>
#include <iostream>

logger::logger()
{
    m_print_thread = std::thread(print_routine, this, std::chrono::milliseconds(16));
}

logger::~logger()
{
    m_print = false;
    m_print_thread.join();
}

void logger::push(std::string fmt_msg)
{
    std::lock_guard<std::mutex> lock(m_q_mu);
    m_q.push(std::move(fmt_msg));
}

void logger::print_routine(logger *instance, std::chrono::duration<double, std::milli> interval)
{
    while(instance->m_print || !instance->m_q.empty()) {
        auto t1 = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(instance->m_q_mu);
            while(!instance->m_q.empty()) {
                std::cout << instance->m_q.front() << std::endl;
                instance->m_q.pop();
            }
        }
        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> time_took = t2 - t1;
        //sleep
        if(time_took < interval && instance->m_print) {
            std::this_thread::sleep_for(interval - time_took);
        }
    }
}

logstream::logstream(logger& log, log_level level) : m_logger(log), m_level(level)
{}

logstream::~logstream()
{
    //note: not using time yet because it adds 0.015 ms
    m_logger.push(get_time_string() + get_level_string() + str());
    //m_logger.push(get_level_string() + str());

}

const std::string logstream::get_level_string()
{
    std::string temp;
    switch(m_level) {
        case log_level::info: temp = "[INFO]"; break;
        case log_level::warning: temp = "[WARNING]"; break;
        case log_level::severe: temp = "[SEVERE]"; break;
        case log_level::fatal: temp = "[FATAL]"; break;
    }
    return temp;    //copy ellision should be guaranteed with a C++17 compiler
}

const std::string logstream::get_time_string()
{
    std::time_t t = std::time(nullptr);
#ifdef _WIN32
    std::tm time;
    localtime_s(&time, &t);
#else
    std::tm time = *std::localtime(&t);
#endif
    char t_str[20];
    std::strftime(t_str, sizeof(t_str), "%T", &time);

    return ("[" + std::string(t_str) + "]");
}

