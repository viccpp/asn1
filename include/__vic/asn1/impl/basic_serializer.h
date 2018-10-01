// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_BASIC_SERIALIZER_H
#define __VIC_ASN1_IMPL_BASIC_SERIALIZER_H

#include<__vic/asn1/ber_coder.h>
#include<__vic/asn1/types.h>
#include<type_traits>

namespace __vic { namespace ASN1 {

//////////////////////////////////////////////////////////////////////////////
class SerializerBase
{
    template<class Serializer>
    struct seq_serializer // "generic lambda"
    {
        Serializer &s;

        template<class T>
        void operator()(const T &v) const { s.serialize(v); }

        template<class T>
        void operator()(const OPTIONAL<T> &v) const
            { if(v.is_set()) s.serialize(v.get()); }
    };
    template<class Serializer>
    struct choice_option_serializer // "generic lambda"
    {
        Serializer &s;

        template<class T>
        void operator()(const T &opt) const { s.serialize(opt); }
    };
protected:
    template<class... > struct always_false : std::false_type {};

    template<class... Opts>
    void serialize_value(const CHOICE<Opts...> & ) // non-instantiable
    {
        static_assert(always_false<Opts...>(), "CHOICE can't be encoded!"
            " (Invalid use of IMPLICIT CHOICE in ASN.1 ?)");
    }

    template<class Serializer, class... Elems>
    static void serialize_seq_value(Serializer &s, const SEQUENCE<Elems...> &seq)
    {
        seq.for_each(seq_serializer<Serializer>{s});
    }
    template<class Serializer, class... Opts>
    static void serialize_choice(Serializer &s, const CHOICE<Opts...> &ch)
    {
        ch.apply(choice_option_serializer<Serializer>{s});
    }
    template<class Serializer, class OID, class... Opts>
    static void serialize_class(Serializer &s, const CLASS_CHOICE<OID,Opts...> &ch)
    {
        ch.validate(); // throws
        s.serialize(ch.oid()); // serialize discriminant-field
        ch.apply_unchecked(choice_option_serializer<Serializer>{s});
    }
};
//////////////////////////////////////////////////////////////////////////////
template<class StreamWriter>
class BasicSerializer : public SerializerBase
{
    BER::Coder<StreamWriter> wr;
protected:
    template<class... Args>
    explicit BasicSerializer(Args&&... args)
        : wr(std::forward<Args>(args)...) {}

    void write_type(type_tag_t tag, BER::primitive_constructed p_c)
        { wr.write_type(tag, p_c); }
    void write_primitive_type(type_tag_t tag)
        { write_type(tag, BER::primitive); }
    void write_constructed_type(type_tag_t tag)
        { write_type(tag, BER::constructed); }
    void write_primitive_type_short(type_tag_t tag)
        { wr.write_type_short(tag, BER::primitive); }
    void write_constructed_type_short(type_tag_t tag)
        { wr.write_type_short(tag, BER::constructed); }
    void write_length(size_t len) { wr.write_length(len); }
    void write_length_short(size_t len) { wr.write_length_short(len); }
    template<class Str>
    void write_value_bytes(Str &s) { wr.write(s.bytes(), s.length()); }
    void write_indefined_length() { wr.write_indefined_length(); }
    void write_eoc_tlv() { wr.write_eoc_tlv(); }

    using SerializerBase::serialize_value;
    void serialize_value(const OCTET_STRING &v)
        { write_value_bytes(v); }
    void serialize_value(const CHARACTER_STRING &v)
        { write_value_bytes(v); }
    void serialize_value(BOOLEAN v)
        { wr.write_boolean(v); }
    template<class Int>
    void serialize_value(integer<Int> v)
        { wr.write_integer(v.as_int()); }
    template<class Enum>
    void serialize_value(ENUMERATED<Enum> v)
        { wr.write_integer(v.as_int()); }
    void serialize_value(NULL_ ) {}
    void serialize_value(const integer<raw> &v)
        { write_value_bytes(v); }
    void serialize_value(const REAL<raw> &v)
        { write_value_bytes(v); }

    template<class Int>
    void serialize_int(integer<Int> );

    void serialize_raw(const raw & , type_tag_t );
    void serialize_str(const CHARACTER_STRING & , type_tag_t );
public:
    typedef StreamWriter stream_writer_type;
    stream_writer_type &get_stream_writer() { return wr.get_stream_writer(); }
    const stream_writer_type &get_stream_writer() const
        { return wr.get_stream_writer(); }

    void serialize(const OCTET_STRING &v) { serialize_raw(v, v.tag()); }
    void serialize(const CHARACTER_STRING &v) { serialize_str(v, v.tag()); }
    void serialize(const PrintableString &v) { serialize_str(v, v.tag()); }
    void serialize(const GraphicString &v) { serialize_str(v, v.tag()); }
    void serialize(const VisibleString &v) { serialize_str(v, v.tag()); }
    void serialize(const NumericString &v) { serialize_str(v, v.tag()); }
    void serialize(const UTF8String &v) { serialize_str(v, v.tag()); }
    void serialize(const ObjectDescriptor &v) { serialize_str(v, v.tag()); }
    void serialize(const UTCTime &v) { serialize_str(v, v.tag()); }
    void serialize(const GeneralizedTime &v) { serialize_str(v, v.tag()); }

    template<class T, size_t L, size_t H>
    void serialize(const SIZE<T,L,H> &v) { serialize(v.unwrap()); }

    void serialize(BOOLEAN );
    void serialize(NULL_ );

    template<class Int>
    void serialize(integer<Int> v) { serialize_int(v); }

    template<class Int, Int L, Int H>
    void serialize(INTEGER<Int,L,H> v) { serialize_int(v); }

    template<class Enum>
    void serialize(ENUMERATED<Enum> );

    void serialize(const integer<raw> &v) { serialize_raw(v, v.tag()); }
    void serialize(const REAL<raw> &v) { serialize_raw(v, v.tag()); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class SW>
void BasicSerializer<SW>::serialize_raw(const raw &v, type_tag_t tag)
{
    write_primitive_type_short(tag);
    write_length(v.length());
    write_value_bytes(v);
}
//----------------------------------------------------------------------------
template<class SW>
void BasicSerializer<SW>::serialize_str(const CHARACTER_STRING &v, type_tag_t tag)
{
    write_primitive_type_short(tag);
    write_length(v.length());
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class Int>
void BasicSerializer<SW>::serialize_int(integer<Int> v)
{
    write_primitive_type_short(v.tag());
    wr.write_integer_with_length(v.as_int());
}
//----------------------------------------------------------------------------
template<class SW>
template<class Enum>
void BasicSerializer<SW>::serialize(ENUMERATED<Enum> v)
{
    write_primitive_type_short(v.tag());
    wr.write_integer_with_length(v.as_int());
}
//----------------------------------------------------------------------------
template<class SW>
void BasicSerializer<SW>::serialize(BOOLEAN v)
{
    write_primitive_type_short(v.tag());
    write_length_short(1);
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
void BasicSerializer<SW>::serialize(NULL_ )
{
    wr.write_null_tlv();
}
//----------------------------------------------------------------------------

}} // namespace

#endif // header guard
