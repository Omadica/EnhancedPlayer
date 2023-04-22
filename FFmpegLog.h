#ifndef __FFMPEGLOG_H__
#define __FFMPEGLOG_H__

#include <functional>
#include <string>


class my_libav
{
public:
    typedef void (*STREAM_LOGGING_CALLBACK_CB)(std::string msg, void* p_object);

    my_libav();
    my_libav(const my_libav &) = delete;
    ~my_libav();
    my_libav& operator=(const my_libav &) = delete;

    void SetStreamLoggingCallback(STREAM_LOGGING_CALLBACK_CB p_stream_logger, void* p_object);

private:
    STREAM_LOGGING_CALLBACK_CB  mp_stream_logging_callback = nullptr;
    void*                       mp_stream_logging_object = nullptr;

    static void RedirectLoggingOutputs( void *ptr, int level, const char *fmt, va_list vargs );
};
#endif
