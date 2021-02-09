// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_DER_ENCODED_LENGTH_H
#define __VIC_ASN1_IMPL_DER_ENCODED_LENGTH_H

#include<__vic/asn1/ber.h>
#include<__vic/asn1/types.h>
#include<cassert>

namespace __vic { namespace asn1 { namespace der {

//----------------------------------------------------------------------------
constexpr size_t encoded_length(BOOLEAN ) { return 1; }
//----------------------------------------------------------------------------
constexpr size_t encoded_length(NULL_ ) { return 0; }
//----------------------------------------------------------------------------
inline size_t encoded_length(const OCTET_STRING &v)
{
    return v.length();
}
//----------------------------------------------------------------------------
inline size_t encoded_length(const CHARACTER_STRING &v)
{
    return v.length();
}
//----------------------------------------------------------------------------
template<class TInt>
inline size_t encoded_length(integer<TInt> v)
{
    return ber::integer_length(v.as_int());
}
//----------------------------------------------------------------------------
inline size_t encoded_length(integer<raw> v)
{
    return v.length();
}
//----------------------------------------------------------------------------
template<class Enum>
inline size_t encoded_length(ENUMERATED<Enum> v)
{
    return ber::integer_length(v.as_int());
}
//----------------------------------------------------------------------------
inline size_t encoded_length(REAL<raw> v)
{
    return v.length();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t encoded_length(const IMPLICIT<Tag,T,Cls> & );
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t encoded_length(const EXPLICIT<Tag,T,Cls> & );
template<class... Elems>
size_t encoded_length(const SEQUENCE<Elems...> & );
template<class T, template<class,class> class SeqCont>
size_t encoded_length(const SEQUENCE_OF<T,SeqCont> & );
template<class... Opts>
size_t encoded_length(const CHOICE<Opts...> & );
//----------------------------------------------------------------------------
struct choice_option_encoded_length // "generic lambda"
{
    template<class T>
    size_t operator()(const T &opt) const { return encoded_length(opt); }
};
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T>
inline size_t tlv_length(const T &v)
{
    size_t len = encoded_length(v);
    return ber::type_field_length(v.tag()) +
           ber::length_field_length(len) +
           len;
}
//----------------------------------------------------------------------------
template<class T>
inline size_t tlv_length(const OPTIONAL<T> &v)
{
    return v.is_set() ? tlv_length(v.get()) : 0;
}
//----------------------------------------------------------------------------
template<class T, class OID, class... Opts> // T is CLASS_CHOICE<>
inline size_t tlv_length_or_class(
    const T &ch, const CLASS_CHOICE<OID,Opts...>  * )
{
    size_t opt_len = ch.apply(choice_option_encoded_length{});
    return tlv_length(ch.oid()) +
           ber::type_field_length(ch.option_tag()) +
           ber::length_field_length(opt_len) +
           opt_len;
}
//----------------------------------------------------------------------------
template<class T> // T isn't CLASS_CHOICE<>
inline size_t tlv_length_or_class(const T &v, const void * )
{
    return tlv_length(v);
}
//----------------------------------------------------------------------------
template<class T>
inline size_t seq_element_length(const T &v)
{
    return tlv_length_or_class(v, &v);
}
//----------------------------------------------------------------------------
inline size_t encoded_length(sequence_elements<> )
{
    return 0;
}
//----------------------------------------------------------------------------
template<class Head, class... Tail>
inline size_t encoded_length(const sequence_elements<Head,Tail...> &se)
{
    return seq_element_length(se.head()) + encoded_length(se.tail());
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
inline size_t encoded_length(const IMPLICIT<Tag,T,Cls> &v)
{
    return encoded_length(v.unwrap());
}
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t encoded_length(const EXPLICIT<Tag,T,Cls> &v)
{
    return tlv_length(v.unwrap());
}
//----------------------------------------------------------------------------
template<class... Elems>
size_t encoded_length(const SEQUENCE<Elems...> &seq)
{
    return encoded_length(seq.elements());
}
//----------------------------------------------------------------------------
template<class T, template<class,class> class SeqCont>
size_t encoded_length(const SEQUENCE_OF<T,SeqCont> &seq)
{
    size_t len = 0;
    for(const auto &el : seq) len += tlv_length(el);
    return len;
}
//----------------------------------------------------------------------------
template<class... Opts>
inline size_t encoded_length(const CHOICE<Opts...> &ch)
{
    return ch.apply(choice_option_encoded_length{});
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
