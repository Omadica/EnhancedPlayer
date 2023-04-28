#include "Logger.h"
#include <cstring>
class Environment;

std::string format_here(std::pair <std::string, int> const & here) {
    std::ostringstream h;
    h << "file \"" << here.first << "\", line " << here.second;
    return std::string(h.str());
}

Logger::Logger() {
    // Prefix for messages
    prefix_ = std::string("EnCommandSDK:");
    return;
}

Logger& Logger::get() {
    static Logger instance;

    // Check the level every time we get a reference to the singleton,
    // in case the level has been changed manually during runtime.
    instance.check_level();
    return instance;
}

void Logger::check_level() {
    auto& env = Environment::get();
    std::string val = env.log_level();
    if (strncmp(val.c_str(), "DEBUG", 5) == 0) {
        level_ = log_level::debug;
    } else if (strncmp(val.c_str(), "INFO", 4) == 0) {
        level_ = log_level::info;
    } else if (strncmp(val.c_str(), "WARNING", 7) == 0) {
        level_ = log_level::warning;
    } else if (strncmp(val.c_str(), "ERROR", 5) == 0) {
        level_ = log_level::error;
    } else if (strncmp(val.c_str(), "CRITICAL", 8) == 0) {
        level_ = log_level::critical;
    } else {
        level_ = log_level::none;
    }
    return;
}

void Logger::debug(char const * msg) {
    if (level_ <= log_level::debug) {
        fprintf(stdout, "%sDEBUG: %s\n", prefix_.c_str(), msg);
        fflush(stdout);
    }
    return;
}

void Logger::debug(char const * msg, std::pair <std::string, int> const & here) {
    if (level_ <= log_level::debug) {
        std::string hstr = format_here(here);
        fprintf(stdout, "%sDEBUG: %s (%s)\n", prefix_.c_str(), msg,
                hstr.c_str());
        fflush(stdout);
    }
    return;
}

void Logger::info(char const * msg) {
    if (level_ <= log_level::info) {
        fprintf(stdout, "%sINFO: %s\n", prefix_.c_str(), msg);
        fflush(stdout);
    }
    return;
}

void Logger::info(char const * msg, std::pair <std::string, int> const & here) {
    if (level_ <= log_level::info) {
        std::string hstr = format_here(here);
        fprintf(stdout, "%sINFO: %s (%s)\n", prefix_.c_str(), msg,
                hstr.c_str());
        fflush(stdout);
    }
    return;
}

void Logger::warning(char const * msg) {
    if (level_ <= log_level::warning) {
        fprintf(stdout, "%sWARNING: %s\n", prefix_.c_str(), msg);
        fflush(stdout);
    }
    return;
}

void Logger::warning(char const * msg, std::pair <std::string, int> const & here) {
    if (level_ <= log_level::warning) {
        std::string hstr = format_here(here);
        fprintf(stdout, "%sWARNING: %s (%s)\n", prefix_.c_str(), msg,
                hstr.c_str());
        fflush(stdout);
    }
    return;
}

void Logger::error(char const * msg) {
    if (level_ <= log_level::error) {
        fprintf(stdout, "%sERROR: %s\n", prefix_.c_str(), msg);
        fflush(stdout);
    }
    return;
}

void Logger::error(char const * msg, std::pair <std::string, int> const & here) {
    if (level_ <= log_level::error) {
        std::string hstr = format_here(here);
        fprintf(stdout, "%sERROR: %s (%s)\n", prefix_.c_str(), msg,
                hstr.c_str());
        fflush(stdout);
    }
    return;
}

void Logger::critical(char const * msg) {
    if (level_ <= log_level::critical) {
        fprintf(stdout, "%sCRITICAL: %s\n", prefix_.c_str(), msg);
        fflush(stdout);
    }
    return;
}

void Logger::critical(char const * msg, std::pair <std::string, int> const & here) {
    if (level_ <= log_level::critical) {
        std::string hstr = format_here(here);
        fprintf(stdout, "%sCRITICAL: %s (%s)\n", prefix_.c_str(), msg,
                hstr.c_str());
        fflush(stdout);
    }
    return;
}
