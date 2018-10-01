// Internal implementation header
//
// Platform: ISO C++ 11
// $Id: basic_deserializer.h 1944 2015-12-22 09:53:20Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_IMPL_BASIC_DESERIALIZER_H
#define __MFISOFT_JANUARY_ASN1_IMPL_BASIC_DESERIALIZER_H

#include<mfisoft/january/bits.h>
#include<mfisoft/january/error.h>
#include<mfisoft/january/string_buffer.h>
#include<mfisoft/january/asn1/ber_decoder.h>
#include<mfisoft/january/asn1/types.h>
#include<type_traits>
#include<algorithm>

namespace mfisoft { namespace january { namespace ASN1 {

//////////////////////////////////////////////////////////////////////////////
struct DeserializerBase
{
    struct bad_format : public jan::exception
    {
        explicit bad_format(const char *m) : jan::exception(m) {}
    };
protected:
    template<class...> struct always_false : std::false_type {};

    typedef BER::type_field_t type_field_t;
    typedef BER::primitive_constructed pc_t;

    static void check_type(type_tag_t , type_tag_t );
    static void check_constructness(pc_t , pc_t );
    static void check_primitive(pc_t );
    static void check_constructed(pc_t );

    static void throw_unexpected_type(type_tag_t );
    static void throw_unexpected_type(type_tag_t , type_tag_t );
    static void throw_cannot_read(type_tag_t , const std::exception & );
    static void throw_cannot_read(const char * , const std::exception & );
    static void throw_required_field_absent();

    template<class...>
    struct have_non_optional_trait : std::false_type {}; // no args case
    template<class Head, class... Tail>
    struct have_non_optional_trait<Head, Tail...> : std::true_type {};
    template<class Head, class... Tail>
    struct have_non_optional_trait<OPTIONAL<Head>, Tail...> :
        std::integral_constant<bool, have_non_optional_trait<Tail...>::value> {};
    template<class... Elems>
    static constexpr bool have_non_optional()
        { return have_non_optional_trait<Elems...>::value; }

    template<class... Opts>
    void deserialize_lv(CHOICE<Opts...> & , pc_t ) // non-instantiable
    {
        static_assert(always_false<Opts...>(), "CHOICE can't be decoded!"
            " (Invalid use of IMPLICIT CHOICE in ASN.1 ?)");
    }

    template<class Deserializer, class T>
    static void check_and_deserialize(
        Deserializer &ds, T &v, const type_field_t &t)
    { check_type(t.tag(), v.tag()); ds.deserialize_lv(v, t.p_c()); }

    template<class Deserializer, class... Opts>
    static void deserialize_choice_lv(
        Deserializer & , CHOICE<Opts...> & , const type_field_t & );

    template<class Deserializer, class OID, class... Opts>
    static void deserialize_class_lv(
        Deserializer & , CLASS_CHOICE<OID,Opts...> & , pc_t );

    template<class Deserializer, class OID, class... Opts>
    static void deserialize_class(
        Deserializer & , CLASS_CHOICE<OID,Opts...> & );

    // Deserialize sequence encoded with definite length form
    template<class Deserializer, class... Elems>
    static void deserialize_seq_def(Deserializer &ds, SEQUENCE<Elems...> &seq)
    {
        seq_def_deserializer<Deserializer>{ds}.deserialize(seq.elements());
    }

    // Deserialize sequence encoded with indefinite length form
    template<class Deserializer, class... Elems>
    static void deserialize_seq_indef(Deserializer &ds, SEQUENCE<Elems...> &seq)
    {
        seq_indef_deserializer<Deserializer>{ds}.deserialize(seq.elements());
    }
private:
    //------------------------------------------------------------------------
    // Deserialize OPTIONAL CHOICE or non-CHOICE element
    template<class Deserializer, class T>
    static bool deserialize_optional_(Deserializer &ds, const type_field_t &t,
        OPTIONAL<T> &opt, void * ) // It isn't a CHOICE
    {
        if(T::tag() != t.tag()) return false;
        ds.deserialize_lv(opt.set_default(), t.p_c());
        return true;
    }
    template<class Deserializer, class Explicit, tag_number_t Tag, class T, tag_class_t Cls>
    static bool deserialize_optional_(Deserializer &ds, const type_field_t &t,
        OPTIONAL<Explicit> &opt, EXPLICIT<Tag,T,Cls> * ) // It's an EXPLICIT
    {
        // EXPLICIT can be derived from CHOICE but generic case must be used
        return deserialize_optional_(ds, t, opt, (void *) 0);
    }
    template<class Deserializer, class Choice, class... Opts>
    static bool deserialize_optional_(Deserializer &ds, const type_field_t &t,
        OPTIONAL<Choice> &opt, CHOICE<Opts...> * ) // It's a CHOICE
    {
        Choice ch;
        if(!deserialize_choice_option(ds, ch, t)) return false;
        opt.set(std::move(ch));
        return true;
    }
    template<class Deserializer, class T>
    static bool deserialize_optional(
        Deserializer &ds, OPTIONAL<T> &opt, const type_field_t &t)
    {
        return deserialize_optional_(ds, t, opt, (T *) 0); // Is T a CHOICE?
    }
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    template<class Deserializer>
    struct seq_def_deserializer
    {
        Deserializer &ds;

        template<class Head, class... Tail>
        void choose_optional(
            sequence_elements<Head,Tail...> &se, const type_field_t &t)
        {
            check_and_deserialize(ds, se.head(), t);
            deserialize(se.tail());
        }
        template<class Head, class... Tail>
        void choose_optional(
            sequence_elements<OPTIONAL<Head>,Tail...> &se, const type_field_t &t)
        {
            if(deserialize_optional(ds, se.head(), t))
                deserialize(se.tail());
            else // Head doesn't match, try the next SEQUENCE element
                choose_optional(se.tail(), t);
        }
        void choose_optional(sequence_elements<> , const type_field_t &t)
        {
            throw_unexpected_type(t.tag());
        }
    public:
        explicit seq_def_deserializer(Deserializer &ds) : ds(ds) {}

        template<class Head, class... Tail>
        void deserialize(sequence_elements<Head,Tail...> &se)
        {
            ds.deserialize(se.head());
            deserialize(se.tail());
        }
        template<class Head, class... Tail>
        void deserialize(sequence_elements<OPTIONAL<Head>,Tail...> &se)
        {
            if(ds.have_more_bytes()) choose_optional(se, ds.read_type());
            else if(have_non_optional<Tail...>()) throw_required_field_absent();
        }
        void deserialize(sequence_elements<> ) {}
    };
    //------------------------------------------------------------------------
    template<class Deserializer>
    class seq_indef_deserializer
    {
        Deserializer &ds;

        template<class Head, class... Tail>
        void choose_optional(
            sequence_elements<Head,Tail...> &se, const type_field_t &t)
        {
            check_and_deserialize(ds, se.head(), t);
            deserialize(se.tail());
        }
        template<class Head, class... Tail>
        void choose_optional(
            sequence_elements<OPTIONAL<Head>,Tail...> &se, const type_field_t &t)
        {
            if(deserialize_optional(ds, se.head(), t))
                deserialize(se.tail());
            else // Head doesn't match, try the next SEQUENCE element
                choose_optional(se.tail(), t);
        }
        void choose_optional(sequence_elements<> , const type_field_t &t)
        {
            throw_unexpected_type(t.tag());
        }
    public:
        explicit seq_indef_deserializer(Deserializer &ds) : ds(ds) {}

        template<class Head, class... Tail>
        void deserialize(sequence_elements<Head,Tail...> &se)
        {
            ds.deserialize(se.head());
            deserialize(se.tail());
        }
        template<class Head, class... Tail>
        void deserialize(sequence_elements<OPTIONAL<Head>,Tail...> &se)
        {
            type_field_t t;
            if(ds.read_type_or_eoc(t)) choose_optional(se, t);
            else if(have_non_optional<Tail...>()) throw_required_field_absent();
        }
        void deserialize(sequence_elements<> ) { ds.skip_eoc_tlv(); }
    };
    //------------------------------------------------------------------------

    template<class Deserializer, class... Opts>
    static bool deserialize_choice_option(
        Deserializer & , CHOICE<Opts...> & , const type_field_t & );
    template<class Deserializer, class OID, class... Opts>
    static void deserialize_class_option(
        Deserializer & , CLASS_CHOICE<OID,Opts...> & , OID && );

    template<class Deserializer>
    struct choice_option_deserializer // "generic lambda"
    {
        Deserializer &ds;
        pc_t p_c;

        template<class T>
        void operator()(T &opt) const { ds.deserialize_lv(opt, p_c); }
    };
    template<class Deserializer>
    struct class_option_deserializer // "generic lambda"
    {
        Deserializer &ds;

        template<class T>
        void operator()(T &opt) const { ds.deserialize(opt); }
    };
};
//////////////////////////////////////////////////////////////////////////////
template<class StreamReader>
class BasicDeserializer : public DeserializerBase
{
    BER::Decoder<StreamReader> rd;
protected:
    template<class... Args>
    explicit BasicDeserializer(Args&&... args)
        : rd(std::forward<Args>(args)...) {}

    type_field_t read_type();
    pc_t read_type(type_tag_t );
    bool read_length(size_t &len) { return rd.read_length(len); }
    size_t read_definite_length() { return rd.read_definite_length(); }

    template<class Str> void read_value_bytes(Str & , size_t );
    template<class Str> void append_value_bytes(Str & , size_t );
    uint8_t read_boolean_value();
    template<class Int> Int read_integer(size_t len)
        { return rd.template read_integer<Int>(len); }

    void push_limit(size_t n) { rd.push_limit(n); }
    void pop_limit() { rd.pop_limit(); }
    bool have_more_bytes() const { return rd.have_more_bytes(); }
    bool read_type_or_eoc(type_field_t &t) { return rd.read_type_or_eoc(t); }
    void skip_eoc_tlv();

    template<class Raw>
    void deserialize_raw_lv(Raw & , pc_t );

    using DeserializerBase::deserialize_lv;
    template<class Int>
    void deserialize_lv(integer<Int> & , pc_t );
    template<class Enum>
    void deserialize_lv(ENUMERATED<Enum> & , pc_t );
    void deserialize_lv(NULL_ & , pc_t );
    void deserialize_lv(integer<raw> &v, pc_t p_c)
        { deserialize_raw_lv(v.as_raw(), p_c); }
    void deserialize_lv(REAL<raw> &v, pc_t p_c)
        { deserialize_raw_lv(v.as_raw(), p_c); }

    template<class Int>
    void deserialize_int(integer<Int> & );

    // Clear the state modulo state of StreamReader
    void partial_reset() { rd.clear_limits_stack(); }
public:
    typedef StreamReader stream_reader_type;
    stream_reader_type &get_stream_reader() { return rd.get_stream_reader(); }
    const stream_reader_type &get_stream_reader() const
        { return rd.get_stream_reader(); }

    void deserialize(NULL_ & );

    template<class Int>
    void deserialize(integer<Int> &v) { deserialize_int(v); }

    template<class Int, Int L, Int H>
    void deserialize(INTEGER<Int,L,H> &v) { deserialize_int(v); }

    template<class Enum>
    void deserialize(ENUMERATED<Enum> & );

    template<class... Opts>
    void deserialize(CHOICE<Opts...> & );

    void deserialize(integer<raw> & );
    void deserialize(REAL<raw> & );
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void DeserializerBase::check_type(type_tag_t t, type_tag_t expected)
{
    if(t != expected) throw_unexpected_type(t, expected);
}
//----------------------------------------------------------------------------
inline void DeserializerBase::check_constructness(pc_t p_c, pc_t expected)
{
    if(p_c != expected)
        throw bad_format("Invalid type \"constructness\" is used");
}
//----------------------------------------------------------------------------
inline void DeserializerBase::check_primitive(pc_t p_c)
{
    //check_constructness(p_c, BER::primitive);
    if(p_c != BER::primitive)
        throw bad_format("Only primitive encoding permitted");
}
//----------------------------------------------------------------------------
inline void DeserializerBase::check_constructed(pc_t p_c)
{
    //check_constructness(p_c, BER::constructed);
    if(p_c != BER::constructed)
        throw bad_format("Only constructed encoding permitted");
}
//----------------------------------------------------------------------------
template<class SR>
auto BasicDeserializer<SR>::read_type() -> type_field_t
{
    type_field_t t;
    if(!rd.read_type(t)) throw bad_format("No type-field found (EOF)");
    return t;
}
//----------------------------------------------------------------------------
template<class SR>
auto BasicDeserializer<SR>::read_type(type_tag_t expected) -> pc_t
{
    auto t = read_type();
    check_type(t.tag(), expected);
    return t.p_c();
}
//----------------------------------------------------------------------------
template<class SR>
template<class Str>
inline void BasicDeserializer<SR>::read_value_bytes(Str &v, size_t len)
{
    v.clear();
    append_value_bytes(v, len);
}
//----------------------------------------------------------------------------
template<class SR>
template<class Str>
void BasicDeserializer<SR>::append_value_bytes(Str &v, size_t len)
{
    if(len == 0) return;
    char buf[256];
    if(v.capacity() < v.length() + len) v.reserve(v.length() + len);
    for(;;)
    {
        size_t n = rd.read_max(buf, std::min(len, sizeof buf));
        if(n == 0) throw bad_format("Not enough bytes of value");
        v.append(buf, n);
        if(n == len) break;
        len -= n;
    }
}
//----------------------------------------------------------------------------
template<class SR>
uint8_t BasicDeserializer<SR>::read_boolean_value()
{
    if(read_definite_length() != 1) throw bad_format("Invalid value length");
    return rd.read_value_byte();
}
//----------------------------------------------------------------------------
template<class SR>
void BasicDeserializer<SR>::skip_eoc_tlv()
{
    if(!rd.is_eoc(read_type())) throw bad_format("EOC expected");
    rd.read_eoc_length();
}
//----------------------------------------------------------------------------
template<class SR>
template<class Raw>
void BasicDeserializer<SR>::deserialize_raw_lv(Raw &v, pc_t p_c)
{
    check_primitive(p_c);
    read_value_bytes(v, read_definite_length());
}
//----------------------------------------------------------------------------
template<class SR>
template<class Int>
void BasicDeserializer<SR>::deserialize_lv(integer<Int> &v, pc_t p_c)
{
    check_primitive(p_c);
    v = read_integer<Int>(read_definite_length());
}
//----------------------------------------------------------------------------
template<class SR>
template<class Enum>
void BasicDeserializer<SR>::deserialize_lv(ENUMERATED<Enum> &v, pc_t p_c)
{
    check_primitive(p_c);
    v.assign(read_integer<decltype(v.as_int())>(read_definite_length()));
}
//----------------------------------------------------------------------------
template<class SR>
void BasicDeserializer<SR>::deserialize_lv(NULL_ & , pc_t p_c)
{
    check_primitive(p_c);
    if(read_definite_length() != 0) throw bad_format("Invalid value length");
}
//----------------------------------------------------------------------------
template<class SR>
template<class Int>
void BasicDeserializer<SR>::deserialize_int(integer<Int> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
void BasicDeserializer<SR>::deserialize(NULL_ &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class Enum>
void BasicDeserializer<SR>::deserialize(ENUMERATED<Enum> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
void BasicDeserializer<SR>::deserialize(integer<raw> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
void BasicDeserializer<SR>::deserialize(REAL<raw> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class Deserializer, class... Opts>
inline bool DeserializerBase::deserialize_choice_option(
    Deserializer &ds, CHOICE<Opts...> &ch, const type_field_t &t)
{
    try {
        ch.set_default_and_apply(t.tag(),
            choice_option_deserializer<Deserializer>{ds, t.p_c()});
        return true;
    } catch(invalid_choice_tag) {
        return false;
    }
}
//----------------------------------------------------------------------------
template<class Deserializer, class... Opts>
inline void DeserializerBase::deserialize_choice_lv(
    Deserializer &ds, CHOICE<Opts...> &ch, const type_field_t &t)
{
    if(!deserialize_choice_option(ds, ch, t))
        throw bad_format(jan::msg(64) <<
            t.tag() << " is not valid CHOICE option");
}
//----------------------------------------------------------------------------
template<class Deserializer, class OID, class... Opts>
inline void DeserializerBase::deserialize_class_option(
    Deserializer &ds, CLASS_CHOICE<OID,Opts...> &ch, OID &&oid)
{
    try {
        ch.set_default_and_apply(oid.c_str(),
            class_option_deserializer<Deserializer>{ds});
    } catch(invalid_choice_tag) {
        throw bad_format(jan::msg(64) <<
            "Invalid CLASS discriminant: \"" << oid << '"');
    }
}
//----------------------------------------------------------------------------
template<class Deserializer, class OID, class... Opts>
inline void DeserializerBase::deserialize_class_lv(
    Deserializer &ds, CLASS_CHOICE<OID,Opts...> &ch, pc_t pc)
{
    OID oid;
    ds.deserialize_lv(oid, pc);
    deserialize_class_option(ds, ch, std::move(oid));
}
//----------------------------------------------------------------------------
template<class Deserializer, class OID, class... Opts>
inline void DeserializerBase::deserialize_class(
    Deserializer &ds, CLASS_CHOICE<OID,Opts...> &ch)
{
    OID oid;
    ds.deserialize(oid);
    deserialize_class_option(ds, ch, std::move(oid));
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
