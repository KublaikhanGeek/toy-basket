/******************************************************************************
 * File name     : Buffer.h
 * Description   :
 * Version       : v1.0
 * Create Time   : 2019/8/29
 * Author        : andy
 * Modify history:
 *******************************************************************************
 * Modify Time   Modify person  Modification
 * ------------------------------------------------------------------------------
 *
 *******************************************************************************/

#ifndef _BUFFER_H
#define _BUFFER_H

#include "StringPiece.h"

#include <algorithm>
#include <endian.h>
#include <vector>

#include <assert.h>
#include <string.h>
//#include <unistd.h>  // ssize_t

namespace toyBasket
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize  = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    // implicit copy-ctor, move-ctor, dtor and assignment are fine
    // NOTE: implicit move-ctor is added in g++ 4.6

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    const char* findCRLF() const
    {
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', static_cast<size_t>(beginWrite() - start));
        return static_cast<const char*>(eol);
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(static_cast<size_t>(end - peek()));
    }

    void retrieveInt64()
    {
        retrieve(sizeof(long long));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(short));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(char));
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    StringPiece toStringPiece() const
    {
        return StringPiece(peek(), static_cast<int>(readableBytes()));
    }

    void append(const StringPiece& str)
    {
        append(str.data(), static_cast<size_t>(str.size()));
    }

    void append(const char* /*restrict*/ data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const void* /*restrict*/ data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    void hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    ///
    /// Append long long using network endian
    ///
    void appendInt64(long long x)
    {
        long long be64 = static_cast<long long>(htobe64(x));
        append(&be64, sizeof(be64));
    }

    ///
    /// Append int using network endian
    ///
    void appendInt32(int x)
    {
        int be32 = static_cast<int>(htobe32(x));
        append(&be32, sizeof(be32));
    }

    void appendInt16(short x)
    {
        short be16 = static_cast<short>(htobe16(x));
        append(&be16, sizeof(be16));
    }

    void appendInt8(char x)
    {
        append(&x, sizeof(x));
    }

    ///
    /// Read long long from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int)
    long long readInt64()
    {
        long long result = peekInt64();
        retrieveInt64();
        return result;
    }

    ///
    /// Read int from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int)
    int readInt32()
    {
        int result = peekInt32();
        retrieveInt32();
        return result;
    }

    short readInt16()
    {
        short result = peekInt16();
        retrieveInt16();
        return result;
    }

    char readInt8()
    {
        char result = peekInt8();
        retrieveInt8();
        return result;
    }

    ///
    /// Peek long long from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(long long)
    long long peekInt64() const
    {
        assert(readableBytes() >= sizeof(long long));
        long long be64 = 0;
        ::memcpy(&be64, peek(), sizeof(be64));
        return static_cast<long long>(be64toh(be64));
    }

    ///
    /// Peek int from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int)
    int peekInt32() const
    {
        assert(readableBytes() >= sizeof(int));
        int be32 = 0;
        ::memcpy(&be32, peek(), sizeof(be32));
        return static_cast<int>(be32toh(be32));
    }

    short peekInt16() const
    {
        assert(readableBytes() >= sizeof(short));
        short be16 = 0;
        ::memcpy(&be16, peek(), sizeof(be16));
        return static_cast<short>(be16toh(be16));
    }

    char peekInt8() const
    {
        assert(readableBytes() >= sizeof(char));
        char x = static_cast<char>(*peek());
        return x;
    }

    ///
    /// Prepend long long using network endian
    ///
    void prependInt64(long long x)
    {
        long long be64 = static_cast<long long>(htobe64(x));
        prepend(&be64, sizeof(be64));
    }

    ///
    /// Prepend int using network endian
    ///
    void prependInt32(int x)
    {
        int be32 = static_cast<int>(htobe32(x));
        prepend(&be32, sizeof(be32));
    }

    void prependInt16(short x)
    {
        short be16 = static_cast<short>(htobe16(x));
        prepend(&be16, sizeof(be16));
    }

    void prependInt8(char x)
    {
        prepend(&x, sizeof(x));
    }

    void prepend(const void* /*restrict*/ data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void shrink(size_t reserve)
    {
        // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
        Buffer other;
        other.ensureWritableBytes(readableBytes() + reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity() const
    {
        return buffer_.capacity();
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t readFd(int fd, int* savedErrno);

private:
    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

} // namespace toyBasket

#endif // _BUFFER_H
