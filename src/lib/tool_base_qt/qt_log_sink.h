#pragma once

#include <orbit_logger.h>

namespace MoonGlare::Tools {

static void QtLogSink(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        AddLogf(Debug, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        AddLogf(Info, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
#endif
    case QtWarningMsg:
        AddLogf(Warning, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        AddLogf(Error, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        AddLogf(Error, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        abort(); //??
    default:
        LogInvalidEnum(type);
        return;
    }
}

} // namespace MoonGlare::Tools
