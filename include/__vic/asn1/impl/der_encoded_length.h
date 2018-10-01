// Internal implementation header
//
// Platform: ISO C++ 11
// $Id: der_encoded_length.h 1757 2015-06-05 11:29:26Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_IMPL_DER_ENCODED_LENGTH_H
#define __MFISOFT_JANUARY_ASN1_IMPL_DER_ENCODED_LENGTH_H

#include<mfisoft/january/asn1/ber.h>
#include<mfisoft/january/asn1/types.h>
#include<cassert>

namespace mfisoft { namespace january { namespace ASN1 { namespace impl {

//----------------------------------------------------------------------------
constexpr size_t der_encoded_length(BOOLEAN ) { return 1; }
//----------------------------------------------------------------------------
constexpr size_t der_encoded_length(NULL_ ) { return 0; }
//----------------------------------------------------------------------------
inline size_t der_encoded_length(const OCTET_STRING &v)
{
    return v.length();
}
//----------------------------------------------------------------------------
inline size_t der_encoded_length(const CHARACTER_STRING &v)
{
    return v.length();
}
//----------------------------------------------------------------------------
template<class TInt>
inline size_t der_encoded_length(integer<TInt> v)
{
    return BER::integer_length(v.as_int());
}
//----------------------------------------------------------------------------
inline size_t der_encoded_length(integer<raw> v)
{
    return v.length();
}
//----------------------------------------------------------------------------
template<class Enum>
inline size_t der_encoded_length(ENUMERATED<Enum> v)
{
    return BER::integer_length(v.as_int());
}
//----------------------------------------------------------------------------
inline size_t der_encoded_length(REAL<raw> v)
{
    return v.length();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t der_encoded_length(const IMPLICIT<Tag,T,Cls> & );
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t der_encoded_length(const EXPLICIT<Tag,T,Cls> & );
template<class... Elems>
size_t der_encoded_length(const SEQUENCE<Elems...> & );
template<class T, template<class,class> class SeqCont>
size_t der_encoded_length(const SEQUENCE_OF<T,SeqCont> & );
template<class... Opts>
size_t der_encoded_length(const CHOICE<Opts...> & );
//----------------------------------------------------------------------------
struct choice_option_der_encoded_length // "generic lambda"
{
    template<class T>
    size_t operator()(const T &opt) const { return der_encoded_length(opt); }
};
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T>
inline size_t der_tlv_length(const T &v)
{
    size_t len = der_encoded_length(v);
    return BER::type_field_length(v.tag()) +
           BER::length_field_length(len) +
           len;
}
//----------------------------------------------------------------------------
template<class T>
inline size_t der_tlv_length(const OPTIONAL<T> &v)
{
    return v.is_set() ? der_tlv_length(v.get()) : 0;
}
//----------------------------------------------------------------------------
template<class T, class OID, class... Opts> // T is CLASS_CHOICE<>
inline size_t der_tlv_length_or_class(
    const T &ch, const CLASS_CHOICE<OID,Opts...>  * )
{
    size_t opt_len = ch.apply(choice_option_der_encoded_length{});
    return der_tlv_length(ch.oid()) +
           BER::type_field_length(ch.option_tag()) +
           BER::length_field_length(opt_len) +
           opt_len;
}
//----------------------------------------------------------------------------
template<class T> // T isn't CLASS_CHOICE<>
inline size_t der_tlv_length_or_class(const T &v, const void * )
{
    return der_tlv_length(v);
}
//----------------------------------------------------------------------------
template<class T>
inline size_t seq_element_length(const T &v)
{
    return der_tlv_length_or_class(v, &v);
}
//----------------------------------------------------------------------------
inline size_t der_encoded_length(sequence_elements<> )
{
    return 0;
}
//----------------------------------------------------------------------------
template<class Head, class... Tail>
inline size_t der_encoded_length(const sequence_elements<Head,Tail...> &se)
{
    return seq_element_length(se.head()) + der_encoded_length(se.tail());
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
inline size_t der_encoded_length(const IMPLICIT<Tag,T,Cls> &v)
{
    return der_encoded_length(v.unwrap());
}
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
size_t der_encoded_length(const EXPLICIT<Tag,T,Cls> &v)
{
    return der_tlv_length(v.unwrap());
}
//----------------------------------------------------------------------------
template<class... Elems>
size_t der_encoded_length(const SEQUENCE<Elems...> &seq)
{
    return der_encoded_length(seq.elements());
}
//----------------------------------------------------------------------------
template<class T, template<class,class> class SeqCont>
size_t der_encoded_length(const SEQUENCE_OF<T,SeqCont> &seq)
{
    size_t len = 0;
    for(const auto &el : seq) len += der_tlv_length(el);
    return len;
}
//----------------------------------------------------------------------------
template<class... Opts>
inline size_t der_encoded_length(const CHOICE<Opts...> &ch)
{
    return ch.apply(choice_option_der_encoded_length{});
}
//----------------------------------------------------------------------------

}}}} // namespace

#endif // header guard
