#include "FFmpegLog.h"
#include <memory>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
}

static my_libav *logging = nullptr;

my_libav::my_libav()
{
    av_log_set_level(AV_LOG_VERBOSE);
    if (logging) throw std::runtime_error("Only 1 instance of my_libav is allowed at a time!");
    logging = this;
    av_log_set_callback( &my_libav::RedirectLoggingOutputs );
}

my_libav::~my_libav()
{
    av_log_set_callback( nullptr );
    logging = nullptr;
}

void my_libav::SetStreamLoggingCallback(STREAM_LOGGING_CALLBACK_CB p_stream_logger, void* p_object)
{
    mp_stream_logging_callback = p_stream_logger;
    mp_stream_logging_object = p_object;
}

void my_libav::RedirectLoggingOutputs(void *ptr, int level, const char *fmt, va_list vargs )
{
    static int print_prefix = 2;
    std::string msg;
    msg.resize(1024);
    av_log_format_line(ptr, level, fmt, vargs, msg.data() , 1024, &print_prefix);
    msg.resize(strlen(msg.data()));
    printf("%s", msg.data());
    if (logging->mp_stream_logging_callback)
        logging->mp_stream_logging_callback(msg, logging->mp_stream_logging_object);
}
