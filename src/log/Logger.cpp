/*
 * Logger.cpp
 *
 *  Created on: 2015年5月11日
 *      Author: zhusheng
 */

#include "Logger.h"
#include <stdio.h>
#include <pthread.h>
#include <memory>
#include <string.h>

static const char HEX[] = "0123456789ABCDEF";

static const char* baseName(const char* path)
{
    if (path)
    {
        char* name = strrchr(path, '/');
        if (name)
            return name + 1;
    }
    return path;
}

class File
{
public:
    File(const char* path)
            : file(fopen(path, "w")), owned(true)
    {
        if (file == NULL)
        {
            fprintf(stderr, "cannot write log file %s \n!", path);
        }
    }

    File(FILE*f = stderr)
            : file(f), owned(false)
    {
    }

    File(int fd)
        : file(fdopen(fd, "w")), owned(true)
    {
    }

    ~File()
    {
        if (file && owned)
        {
            ::fclose(file);
            file = NULL;
            owned = false;
        }
    }

    size_t write(const char* buf, size_t len)
    {
        size_t r = -1;
        if (file)
        {
            r = fwrite(buf, 1, len, file);
            fflush(file);
        }
        return r;
    }

    void flush()
    {
        if(file)
        {
            fflush(file);
        }
    }

private:
    File(const File&);
    File operator=(const File&);
    protected:
    FILE* file;
    bool owned;
};

class LockGuard
{
    pthread_mutex_t& m;
    public:
    LockGuard(pthread_mutex_t&m)
            : m(m)
    {
        pthread_mutex_lock(&m);
    }
    ~LockGuard()
    {
        pthread_mutex_unlock(&m);
    }
private:
    LockGuard(const LockGuard&);
    LockGuard operator=(const LockGuard&);
};

class FileDevice: public File, public Logger::Device
{
public:
    FileDevice(FILE* f = stderr)
            : File(f)
    {
    }
    FileDevice(const std::string& path)
            : File(path.c_str())
    {
    }
    virtual ~FileDevice()
    {
    }
    virtual void write(const char* str, int len)
    {
        File::write(str, len);
    }
    virtual void flush()
    {
        File::flush();
    }
};

class RotateFileDevice: public Logger::Device
{
public:
    RotateFileDevice(const std::string& prefix)
            : prefix(prefix), stamp(0), file(NULL)
    {
    }
    virtual ~RotateFileDevice()
    {
    }
    virtual void write(const char* str, int len)
    {
        openFile();

        std::shared_ptr<File> f = file;
        f->write(str, len);
    }
    virtual void flush()
    {
        std::shared_ptr<File> f = file;
        f->flush();
    }
protected:
    void openFile()
    {
        time_t now = time(NULL);
        struct tm tm = {};
        localtime_r(&now, &tm);

        int newstamp = ((tm.tm_year * 100 + (tm.tm_mon + 1)) * 100 + tm.tm_mday) * 100 + tm.tm_hour;
        if (stamp < newstamp || !file)
        {
            char buf[100];
            sprintf(buf, ".%04d-%02d-%02d-%02d", tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);

            file.reset(new File((prefix + buf).c_str()));
            stamp = newstamp;
        }
    }
protected:
    std::string prefix;
    int stamp;
    std::shared_ptr<File> file;
};

//---------------------------------------------------------------------
Logger::Buffer::Buffer(Device* d) : ref(1), device(d)
{
}

//---------------------------------------------------------------------
Logger::Device::~Device()
{
}

//---------------------------------------------------------------------
Logger::Logger(Logger::Device *d)
        : device(d)
{
}

Logger::~Logger()
{
}

Logger& Logger::logger()
{
    static Logger instance(new FileDevice());
    return instance;
}

void Logger::setDevice(Logger::Device* d)
{
    delete device;
    device = d;
}

Logger::Device* Logger::getDevice()
{
    return device;
}

void Logger::flush()
{
    if(device)
    {
        device->flush();
    }
}

void Logger::setFile(FILE* f)
{
    setDevice(new FileDevice(f));
}

void Logger::setLocalFile(const std::string& path)
{
    setDevice(new FileDevice(path));
}

void Logger::setRotate(const std::string& prefix)
{
    setDevice(new RotateFileDevice(prefix));
}

Logger::Writer Logger::writer(int on, const char* file, int line, const char* func, const char* level)
{
    if (on) {
        return Logger::Writer(logger().getDevice())
            << "[" << level << "]\t"
            << func << " ("
            << baseName(file) << ":" << line << ")\t";
    } else {
        return Logger::Writer(0);
    }
}

//---------------------------------------------------------------------
Logger::Writer::Writer(Logger::Device* d)
        : buf(NULL)
{
    if (d)
    {
        buf = new Buffer(d);
    }
}

Logger::Writer::~Writer()
{
    if (buf && --buf->ref == 0)
    {
        buf->sstream << std::endl;
        std::string v = buf->sstream.str();
        buf->device->write(v.c_str(), (int)v.length());
        delete buf;
        buf = NULL;
    }
}

Logger::Writer::Writer(const Logger::Writer& other)
{
    buf = other.buf;
    if (buf)
        buf->ref++;
}

Logger::Writer& Logger::Writer::operator=(const Writer& other)
{
    if (this != &other)
    {
        delete buf;
        buf = other.buf;
        if (buf)
            buf->ref++;
    }
    return *this;
}

Logger::Writer& Logger::Writer::append(const char* s, int len)
{
    if (buf && len != 0)
    {
        buf->sstream.write(s, len < 0 ? strlen(s) : len);
    }
    return *this;
}

Logger::Writer& Logger::Writer::appendv(const char* fmt, va_list ap)
{
    if (buf)
    {
        char buffer[4096];
        int n = vsnprintf(buffer, sizeof(buffer) - 1, fmt, ap);
        if (n > 0)
        {
            buf->sstream.write(buffer, n);
        }
    }
    return *this;
}

Logger::Writer& Logger::Writer::appendf(const char* fmt, ...)
{
    if (buf)
    {
        va_list ap;
        va_start(ap, fmt);
        appendv(fmt, ap);
        va_end(ap);
    }
    return *this;
}

Logger::Writer& Logger::Writer::write()
{
    return *this;
}

Logger::Writer& Logger::Writer::write(const char* fmt, ...)
{
    if (buf)
    {
        va_list ap;
        va_start(ap, fmt);
        appendv(fmt, ap);
        va_end(ap);
    }
    return *this;
}

Logger::Writer& Logger::Writer::hex(const char* str, int len)
{
    if (buf && len > 0)
    {
        const int N = 16;
        char buffer[(N + 1) * 3];

        const unsigned char* s = (const unsigned char*) str;
        while (len > 0)
        {
            int n = len < N ? len : N;
            char* d = buffer;
            for (int i = 0; i < n; i++)
            {
                unsigned char c = s[i];
                *d++ = HEX[c >> 4];
                *d++ = HEX[c & 0xF];
                *d++ = ' ';
            }
            len -= n;
            s += n;

            if (len > 0)
            {
                *d++ = '\n';
            }
            buf->sstream.write(buffer, d - buffer);
        }
    }
    return *this;
}

Logger::Writer& Logger::Writer::bin(const char* str, int len)
{
    if (buf && len > 0)
    {
        const int N = 64;
        char buffer[N + 8];

        int n = 0;
        for (const char* b = str, *e = str + len; b != e; b++)
        {
            unsigned char c = (unsigned char) *b;
            if (c >= 0x20 && c < 0x7F)
            {
                buffer[n++] = *b;
            }
            else
            {
                buffer[n++] = '\\';
                buffer[n++] = 'x';
                buffer[n++] = HEX[c >> 4];
                buffer[n++] = HEX[c & 0xF];
            }
            if (n >= N)
            {
                buffer[n++] = '\n';
                buf->sstream.write(buffer, n);
                n = 0;
            }
        }
        if (n > 0)
        {
            buf->sstream.write(buffer, n);
            n = 0;
        }
    }
    return *this;
}

