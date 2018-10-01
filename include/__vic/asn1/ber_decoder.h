// Primitives for ASN.1 BER decoding
//
// Platform: ISO C++ 98 / 11
// $Id$

#ifndef __VIC_ASN1_BER_DECODER_H
#define __VIC_ASN1_BER_DECODER_H

#include<__vic/asn1/ber.h>
#include<__vic/string_buffer.h>
#include<__vic/error.h>
#include<algorithm>
#include<vector>

namespace __vic { namespace ASN1 { namespace BER {

//////////////////////////////////////////////////////////////////////////////
class DecoderBase
{
    class limit
    {
        size_t total_, avail_;
    public:
        explicit limit(size_t lim) : total_(lim), avail_(lim) {}

        size_t total() const { return total_; }
        size_t avail() const { return avail_; }

        limit &operator-=(size_t n)
            { assert(avail() >= n); avail_ -= n; return *this; }
        limit &operator--()
            { assert(avail() > 0); avail_--; return *this; }
    };
    std::vector<limit> limits; // stack

    limit &limits_top() { return limits.back(); }
    const limit &limits_top() const { return limits.back(); }
    void limits_push(size_t n) { limits.push_back(limit(n)); }
    void limits_drop() { limits.pop_back(); }
protected:
    size_t bytes_avail() const
        { assert(is_limited()); return limits_top().avail(); }
    void consume_1() { assert(is_limited()); --limits_top(); }
    void consume(size_t n) { assert(is_limited()); limits_top() -= n; }
public:
    // Exceptions
    class error : public __vic::exception
    {
    protected:
        explicit error(const char *msg) : __vic::exception(msg) {}
    };
    // Bad input stream format (error in the stream generator)
    struct format_error : public error
    {
        explicit format_error(const char * );
    };
    // Implementation restriction is violated (Input stream is valid)
    struct decoding_error : public error
    {
        explicit decoding_error(const char * );
    };

    // Constructed form is used for EOC
    struct constructed_eoc_error : public format_error
    {
        constructed_eoc_error();
    };
    struct truncated_stream_error : public format_error
    {
        truncated_stream_error();
    };

    void push_limit(size_t );
    void pop_limit();
    void clear_limits_stack() { limits.clear(); }
    bool is_unlimited() const { return limits.empty(); }
    bool is_limited() const { return !is_unlimited(); }

    bool have_more_bytes() const
        { return is_unlimited() || limits_top().avail() != 0; }

    static bool is_eoc(const type_field_t & );
};
//////////////////////////////////////////////////////////////////////////////
// Desired StreamReader's interface:
//
//  struct StreamReader
//  {
//      StreamReader(const StreamReader & ); or StreamReader();
//      bool read(uint8_t &byte);
//      size_t read_max(void *bytes, size_t n);
//      // optional members
//      size_t skip_max(size_t n); // used only for skip_...() implementation
//  };
//////////////////////////////////////////////////////////////////////////////
template<class StreamReader>
class Decoder : public DecoderBase
{
    StreamReader r;
    bool next_byte_is_0(size_t & );
    template<class TUInt> TUInt read_type_raw_rest(uint8_t );
public:
    typedef StreamReader stream_reader_type;
    stream_reader_type &get_stream_reader() { return r; }
    const stream_reader_type &get_stream_reader() const { return r; }

#if __cplusplus >= 201103L // C++11
    template<class... Args>
    explicit Decoder(Args&&... args) : r(std::forward<Args>(args)...) {}
#else // C++98
    Decoder() {}
    explicit Decoder(const StreamReader &s) : r(s) {}
#endif

    bool read(uint8_t & );
    size_t read_max(void * , size_t );
    void skip_exact(size_t );
    void skip_rest();

    // Returns false if no bytes to read
    bool read_type(tag_class_t & , tag_number_t & , primitive_constructed & );
    bool read_type(type_tag_t &t, primitive_constructed &p_c)
        { return read_type(t.class_ref(), t.number_ref(), p_c); }
    bool read_type(type_field_t &t)
        { return read_type(t.tag_class_ref(), t.tag_number_ref(), t.p_c_ref()); }
    template<class TUInt> bool read_type_raw(TUInt & );

    // Returns false if length is indefinite
    bool read_length(size_t & );
    size_t read_definite_length();

    template<class TInt> TInt read_integer(size_t );
    template<class TInt> TInt read_integer_lv();

    uint8_t read_value_byte();
    void read_eoc_length();
    bool read_type_or_eoc(type_field_t & );
    template<class TUInt> bool read_type_raw_or_eoc(TUInt & );
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool DecoderBase::is_eoc(const type_field_t &t)
{
#if __cplusplus >= 201103L // C++11
    if(t.tag() != EOC)
#else // C++98
    if(t.tag_number() != 0 || t.tag_class() != universal)
#endif
        return false;
    if(t.p_c() != primitive) throw constructed_eoc_error();
    return true;
}
//----------------------------------------------------------------------------
template<class SR>
bool Decoder<SR>::read(uint8_t &byte)
{
    if(this->is_unlimited()) return r.read(byte);
    if(this->bytes_avail() == 0 || !r.read(byte)) return false;
    this->consume_1();
    return true;
}
//----------------------------------------------------------------------------
template<class SR>
size_t Decoder<SR>::read_max(void *bytes, size_t n)
{
    if(this->is_unlimited()) return r.read_max(bytes, n);
    size_t res = r.read_max(bytes, std::min(this->bytes_avail(), n));
    this->consume(res);
    return res;
}
//----------------------------------------------------------------------------
template<class SR>
void Decoder<SR>::skip_exact(size_t n)
{
    if(this->is_unlimited())
    {
        if(r.skip_max(n) == n) return;
    }
    else if(this->bytes_avail() >= n)
    {
        size_t res = r.skip_max(n);
        this->consume(res);
        if(res == n) return;
    }
    throw truncated_stream_error(); // not enough bytes of value
}
//----------------------------------------------------------------------------
template<class SR>
void Decoder<SR>::skip_rest()
{
    if(this->is_unlimited())
    {
        const size_t max = ~size_t(0);
        while(r.skip_max(max) == max);
        return;
    }
    size_t n = this->bytes_avail();
    if(n)
    {
        size_t res = r.skip_max(n);
        this->consume(res);
        if(res != n) throw truncated_stream_error();
    }
}
//----------------------------------------------------------------------------
template<class SR>
bool Decoder<SR>::read_type(
    tag_class_t &cls, tag_number_t &tag, primitive_constructed &p_c)
{
    uint8_t b;
    if(!read(b)) return false;

    cls = static_cast<tag_class_t>(b >> 6);
    p_c = static_cast<primitive_constructed>((b >> 5) & 1);
    tag_number_t t = b & 0x1FU;
    if(t != 0x1FU) // trivial case - low-tag-number form
    {
        tag = t;
        return true;
    }

    // It's a high-tag-number form:
    // The tag number is stored in subsequent bytes in base-128
    // in big-endian order where the 8th bit is 1 if more bytes
    // follow and 0 for the last byte of the tag number
    t = 0;
    for(unsigned i = 0;; i++)
    {
        if(!read(b)) throw truncated_stream_error(); // no tag in subsequent bytes
        bool is_last = (b & 0x80U) == 0U;
        t <<= 7; // *= 128
        t |= b & 0x7FU;
        if(is_last)
        {
            tag = t;
            return true; // Done
        }
        else if(i == sizeof t)
        {
            // FixMe:
            // Condition isn't precise cause we use only 7 bits from
            // each byte. In fact we can support a little bigger values
            throw decoding_error("tag value is too long");
        }
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class TUInt>
inline TUInt Decoder<SR>::read_type_raw_rest(uint8_t b)
{
    if((b & 0x1FU) != 0x1FU) return b; // single byte
    // more than one byte
    TUInt res = b;
    for(unsigned i = 0; i < sizeof res - 1; i++)
    {
        if(!read(b)) throw truncated_stream_error(); // no tag in subsequent bytes
        res = (res << 8) | b; // res * 256 + b;
        if((b & 0x80U) == 0U) break;
    }
    if((b & 0x80U) != 0U)
        throw decoding_error("tag field length is longer than expected");
    return res;
}
//----------------------------------------------------------------------------
template<class SR>
template<class TUInt>
bool Decoder<SR>::read_type_raw(TUInt &type_field)
{
    uint8_t b;
    if(!read(b)) return false;
    type_field = read_type_raw_rest<TUInt>(b);
    return true;
}
//----------------------------------------------------------------------------
template<class SR>
bool Decoder<SR>::read_length(size_t &len_)
{
    uint8_t b;
    if(!read(b)) throw truncated_stream_error(); // no length-field

    if((b & 0x80U) == 0U)  // check the high-order bit
    {
        // trivial case - short form
        len_ = b;
        return true;
    }
    if(b == 0x80U) return false; // indefinite length

    // It's a long form:
    // b is a "length of the length field" byte that indicates the number
    // of bytes to follow that represent the length field, followed by that
    // number of bytes encoding the length in big-endian byte order
    unsigned len_field_len = b & 0x7FU; // clear high-order bit
    if(len_field_len > sizeof(size_t)) throw decoding_error(__vic::msg(64)
         << "\"length of the length field\" value is too long: "
         << len_field_len);
    size_t len = 0;
    while(len_field_len--)
    {
        if(!read(b)) throw truncated_stream_error(); // truncated length-field
        len <<= 8; // *= 256
        len |= b; // += b
    }
    len_ = len;
    return true;
}
//----------------------------------------------------------------------------
template<class SR>
size_t Decoder<SR>::read_definite_length()
{
    size_t len;
    if(!read_length(len))
        throw decoding_error("Definite length expected");
    return len;
}
//----------------------------------------------------------------------------
template<class SR>
inline uint8_t Decoder<SR>::read_value_byte()
{
    uint8_t b;
    if(!read(b)) throw truncated_stream_error(); // not enough bytes of value
    return b;
}
//----------------------------------------------------------------------------
template<class SR>
inline bool Decoder<SR>::next_byte_is_0(size_t &len)
{
    uint8_t b = read_value_byte();
    len--;
    return b == 0;
}
//----------------------------------------------------------------------------
template<class SR>
template<class TInt>
TInt Decoder<SR>::read_integer(size_t len)
{
    if(len == 0) throw format_error("length can't be 0");
    size_t c = len;
    if(len > sizeof(TInt) && (TInt(-1)<TInt(0) ||
                    (len > sizeof(TInt) + 1 || !next_byte_is_0(c))))
        throw decoding_error(__vic::msg(64) <<
            "value length is too big: " << len << " bytes");
    TInt res = read_value_byte();
    bool is_neg = res & TInt(0x80);
    if(TInt(-1) >= TInt(0)) // if TInt is unsigned
    {
        if(c == len && is_neg)
            throw decoding_error("can't store negative value");
        is_neg = false;
    }
    while(--c) res <<= 8, res |= read_value_byte();
    if(is_neg)
    {
        // Fill high-order bits with 1
        unsigned diff = (sizeof(TInt) - len) << 3; // (sizeof(TInt) - len) * 8
        res <<= diff;
        res >>= diff;
    }
    return res;
}
//----------------------------------------------------------------------------
template<class SR>
template<class TInt>
inline TInt Decoder<SR>::read_integer_lv()
{
    return read_integer<TInt>(read_definite_length());
}
//----------------------------------------------------------------------------
template<class SR>
inline void Decoder<SR>::read_eoc_length()
{
    size_t len;
    if(!read_length(len))
        throw format_error("indefinite length can't be used for EOC");
    if(len != 0)
        throw format_error("EOC can't have non-zero length");
}
//----------------------------------------------------------------------------
template<class SR>
bool Decoder<SR>::read_type_or_eoc(type_field_t &t)
{
    if(!read_type(t)) throw truncated_stream_error(); // no type-field found (EOF)
    if(!is_eoc(t)) return true;
    read_eoc_length();
    return false;
}
//----------------------------------------------------------------------------
template<class SR>
template<class TUInt>
bool Decoder<SR>::read_type_raw_or_eoc(TUInt &t)
{
    if(!read_type_raw(t)) throw truncated_stream_error(); // no type-field found (EOF)
    switch(t)
    {
        case 0x00U: // EOC
            read_eoc_length();
            return false;
        case 0x20U: // constructed EOC
            throw constructed_eoc_error();
    }
    return true;
}
//----------------------------------------------------------------------------

#if __cplusplus >= 201103L // C++11
//----------------------------------------------------------------------------
inline __vic::string_buffer &operator<<(__vic::string_buffer &s, type_tag_t t)
    { to_text(t, s); return s; }
inline __vic::string_buffer &&operator<<(__vic::string_buffer &&s, type_tag_t t)
    { to_text(t, s); return std::move(s); }
//----------------------------------------------------------------------------
#endif

}}} // namespace

#endif // header guard
