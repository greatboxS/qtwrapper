#include "QLogger.hpp"
#include <string.h>

namespace qtwrapper
{
    const QLoggingCategory &QLoggerCategory() {
        static QLoggingCategory category("QLOGGER");
        static int init = 0;
        if (!init) {
            init = 1;
            category.setEnabled(QtMsgType::QtDebugMsg, true);
            category.setEnabled(QtMsgType::QtInfoMsg, true);
            category.setEnabled(QtMsgType::QtWarningMsg, true);
            category.setEnabled(QtMsgType::QtCriticalMsg, true);
        }
        return category;
    }

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
    static int IsActivatedVirtualTerminal = 0;
#endif

#ifdef DBG_FLUSH_ALWAYS
    static int StdoutBuffDisabled = 0;
#endif

    int __DEBUG_LEVEL__ = (int)DBG_LVL_INFO;

    void DBG_SetLevel(int level) {

#ifdef DBG_FLUSH_ALWAYS
        if (!StdoutBuffDisabled) {
            StdoutBuffDisabled = 1;
            setbuf(stdout, NULL);
        }
#endif
        __DEBUG_LEVEL__ = level;

#if defined(WIN32) || defined(_WIN32)
#ifdef USE_VIRTUAL_TERMINAL
        ActivateVirtualTerminal();
#endif
#endif
    }

    const char *DBG_GetClassName(const char *name) {
        size_t len = strlen(name);
        size_t index = 0;

        for (index = 0; index < len; index++)
            if (!(name[index] >= '0' && name[index] <= '9')) break;

        return &name[index];
    }

#if defined(WIN32) || defined(_WIN32)
    void ActivateVirtualTerminal() {
        if (!IsActivatedVirtualTerminal) {
            HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD consoleMode;
            GetConsoleMode(handleOut, &consoleMode);
            consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;
            SetConsoleMode(handleOut, consoleMode);
            IsActivatedVirtualTerminal = 1;
        }
    }

#endif
} // namespace qtwrapper
