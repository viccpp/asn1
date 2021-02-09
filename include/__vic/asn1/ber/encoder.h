// Primitives for ASN.1 BER encoding
//
// Platform: ISO C++ 98 / 11
// $Id$

#ifndef __VIC_ASN1_BER_ENCODER_H
#define __VIC_ASN1_BER_ENCODER_H

#include<__vic/asn1/ber/defs.h>
#include<cassert>

namespace __vic { namespace asn1 { namespace ber {

//////////////////////////////////////////////////////////////////////////////
// Desired StreamWriter's interface:
//
//  struct StreamWriter
//  {
//      StreamWriter(const StreamWriter & ); or StreamWriter();
//      void write(uint8_t byte);
//      void write(const void *bytes, size_t n);
//  };
//////////////////////////////////////////////////////////////////////////////
template<class StreamWriter>
class encoder
{
    StreamWriter w;
public:
    typedef StreamWriter stream_writer_type;
    stream_writer_type &get_stream_writer() { return w; }
    const stream_writer_type &get_stream_writer() const { return w; }

#if __cplusplus >= 201103L // C++11
    template<class... Args>
    explicit encoder(Args&&... args) : w(std::forward<Args>(args)...) {}
#else // C++98
    encoder() {}
    explicit encoder(const StreamWriter &s) : w(s) {}
#endif

    void write(uint8_t byte) { w.write(byte); }
    void write(const void *bytes, size_t n) { w.write(bytes, n); }

    void write_type(tag_class_t , tag_number_t , primitive_constructed );
    void write_type(type_tag_t t, primitive_constructed p_c)
        { write_type(t.class_(), t.number(), p_c); }
    void write_type(type_field_t t)
        { write_type(t.tag_class(), t.tag_number(), t.p_c()); }
    void write_length(size_t );

    void write_type_short(
        tag_class_t cls, tag_number_t tag, primitive_constructed p_c)
    {
        assert(tag < 0x1FU);
        write((((cls << 1) | p_c) << 5) | tag);
    }
    void write_type_short(type_tag_t t, primitive_constructed p_c)
        { write_type_short(t.class_(), t.number(), p_c); }
    void write_type_short(type_field_t t)
        { write_type_short(t.tag_class(), t.tag_number(), t.p_c()); }
    void write_length_short(size_t len)
    {
        assert(len <= 0x7FU);
        write(len);
    }
    void write_indefined_length() { write(0x80); }

    template<class TInt> void write_integer(TInt );
    template<class TInt> void write_integer_with_length(TInt );

    void write_boolean(bool v) { write(v ? 0xFF : 0); }

    void write_null_tlv() { write(0x05); write(0x00); }
    void write_eoc_tlv() { write(0x00); write(0x00); }
};
//////////////////////////////////////////////////////////////////////////////
namespace impl {
//----------------------------------------------------------------------------
template<class TInt>
class integer_encoder
{
    // buffer for bytes of the integer
    uint8_t buf[sizeof(TInt) + (TInt(-1)<TInt(0) ? 0 : 1)];
    int buf_ptr;
public:
    explicit integer_encoder(TInt );
    template<class Writer>
    void write(Writer &wr) const
        { int c = buf_ptr; while(c--) wr.write(buf[c]); }
    size_t length() const { return buf_ptr; }
};
//----------------------------------------------------------------------------
template<class TInt>
integer_encoder<TInt>::integer_encoder(TInt v) : buf_ptr(1)
{
    const bool is_positive = v > TInt(0);
    buf[0] = v & TInt(0xFF);
    v >>= 8;
    while(v != TInt(0) && ~v != TInt(0))
    {
        buf[buf_ptr++] = v & TInt(0xFF);
        v >>= 8;
    }
    // if value is positive but MSB is 1
    if(is_positive && (buf[buf_ptr - 1] & uint8_t(0x80)))
        buf[buf_ptr++] = 0; // then add leading zero byte
}
//----------------------------------------------------------------------------
} // namespace
//----------------------------------------------------------------------------
template<class SW>
void encoder<SW>::write_type(
    tag_class_t cls, tag_number_t tag, primitive_constructed p_c)
{
    if(tag < 0x1FU) // trivial case - low-tag-number form
    {
        write_type_short(cls, tag, p_c);
        return;
    }
    // Using a high-tag-number form
    uint8_t digits[sizeof tag * 8 / 7 + 1]; // Stack for digits
    int idx = 0;
    do {
        digits[idx++] = tag & 0x7FU; // less significant 7 bits
        tag >>= 7; // /= 128
    } while(tag);
    write((((cls << 1) | p_c) << 5) | 0x1F); // long form indicator
    // Print digits in the reverse order (Big-Endian)
    while(--idx) write(digits[idx] | 0x80U);
    write(digits[0]);
}
//----------------------------------------------------------------------------
template<class SW>
void encoder<SW>::write_length(size_t len)
{
    if(len <= 0x7FU) // trivial case - short form
    {
        write_length_short(len);
        return;
    }
    // Using a long form
    uint8_t digits[sizeof len]; // Stack for digits
    int idx = 0;
    do {
        digits[idx++] = len & 0xFFU;
        len >>= 8; // /= 256
    } while(len);
    write(idx | 0x80); // "length of the length field"
    do write(digits[--idx]); while(idx);
}
//----------------------------------------------------------------------------
template<class SW>
template<class TInt>
void encoder<SW>::write_integer(TInt v)
{
    impl::integer_encoder<TInt>(v).write(*this);
}
//----------------------------------------------------------------------------
template<class SW>
template<class TInt>
void encoder<SW>::write_integer_with_length(TInt v)
{
    impl::integer_encoder<TInt> enc(v);
    // wrile length-field
#if __cplusplus >= 201103L // C++11
    static_assert(sizeof(TInt) <= 0x7FU, "Integer can't be encoded with the "
        "short length form. Use write_length()+write_integer()");
#endif
    write_length_short(enc.length());
    // wrile value-field
    enc.write(*this);
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
