#ifndef __LOGGER_H_CUSTOM__
#define __LOGGER_H_CUSTOM__
#include <cstddef>
#include <sstream>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include "Environment.h"

enum class log_level {
    none     = 0, ///< Undefined
    debug    = 1, ///< Debug
    info     = 2, ///< Info
    warning  = 3, ///< Warning
    error    = 4, ///< Error
    critical = 5  ///< Critical
};

#define cal_HERE() \
    std::make_pair(std::string(__FILE__), int(__LINE__))
std::string format_here(std::pair <std::string, int> const & here);

/**
* \class Logger
* \brief Global logger singleton.
*/
extern "C"
{

class  Logger {
    

    public:

        /**Singleton access*/
        static Logger & get();

        void debug(char const * msg);
        void debug(char const * msg,
                   std::pair <std::string, int> const & here);
        void info(char const * msg);
        void info(char const * msg,
                  std::pair <std::string, int> const & here);
        void warning(char const * msg);
        void warning(char const * msg,
                     std::pair <std::string, int> const & here);
        void error(char const * msg);
        void error(char const * msg,
                   std::pair <std::string, int> const & here);
        void critical(char const * msg);
        void critical(char const * msg,
                      std::pair <std::string, int> const & here);

    private:

        /**This class is a singleton- constructor is private. Private constructor to prevent instancing.*/
        Logger();
        void check_level();

        log_level level_;
        std::string prefix_;
};


}
#endif // __LOGGER_H_CUSTOM__
