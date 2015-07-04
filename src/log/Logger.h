/*
 * Logger.h
 *
 *  Created on: 2015年5月11日
 *      Author: zhusheng
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <sstream>
#include <string>
#include <stdarg.h>

#ifndef logTag
#define logTag __FUNCTION__
#endif

#ifndef logDebugOn
#define logDebugOn 1
#endif

#ifndef logInfoOn
#define logInfoOn 1
#endif

#define logDebug Logger::writer(logDebugOn, __FILE__, __LINE__, logTag, "debug").write
#define logInfo  Logger::writer(logInfoOn, __FILE__, __LINE__, logTag, "info").write
#define logWarn  Logger::writer(1, __FILE__, __LINE__, logTag, "warn").write
#define logError Logger::writer(1, __FILE__, __LINE__, logTag, "error").write
#define logAssert(c, ...) do{ if(!(c)){logError("assert " #c " failed");  Logger::logger().flush(); exit(1);}} while(0)

#ifndef _FMT_LIKE
#ifdef __GNUC__
#define _FMT_LIKE(m,n) __attribute__((format(printf, m, n)))
#else
#define _FMT_LIKE(m,n)
#endif
#endif // _FMT_LIKE

class Logger
{
public:
    class Writer;

    class Device
    {
    public:
        virtual ~Device() = 0;
        virtual void write(const char* str, int len) = 0;
        virtual void flush() = 0;
    };

    class Buffer
    {
    public:
        Buffer(Device* d);
        int ref;
        Device* device;
        std::stringstream sstream;
    };

    // logDebug()<< logFmt(123, 10, 10, 10) << logFmt(213);

    class Writer
    {
    public:
        Writer(Device* d);
        Writer(const Writer& other);
        ~Writer();

        Writer& operator=(const Writer& other);

        template<typename T>
        Writer& operator <<(const T&t)
        {
            if (buf)
                buf->sstream << t;
            return *this;
        }

        Writer& append(const char* s, int len);
        Writer& appendv(const char* fmt, va_list ap);
        Writer& appendf(const char* fmt, ...) _FMT_LIKE(2,3);

        Writer& write();
        Writer& write(const char* fmt, ...);

        Writer& hex(const char* str, int len);
        Writer& bin(const char* str, int len);

    protected:
        Buffer* buf;
    };
    
public:
    Logger(Device* d);
    ~Logger();
    void setFile(FILE* f);
    void setLocalFile(const std::string& path);
    void setRotate(const std::string& prefix);
    void setDevice(Device* d);
    void flush();
    Device* getDevice();
    static Logger& logger();
    static Writer writer(int on, const char* file, int line, const char* func, const char* level);
protected:
    Device* device;
};

#endif /* LOGGER_H_ */
