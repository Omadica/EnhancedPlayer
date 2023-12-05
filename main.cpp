#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{

    const spdlog::level::level_enum log_level = spdlog::level::debug;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Native_log.txt", true);

    console_sink->set_level(log_level);
    file_sink->set_level(log_level);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    auto logger  = std::make_shared<spdlog::logger>("NativeLog", begin(sinks), end(sinks));
    logger->set_level(log_level);
    logger->enable_backtrace(32);

    // register
    spdlog::register_logger(logger);

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "EnhancedPlayer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
