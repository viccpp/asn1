// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_DER_PC_TRAITS_H
#define __VIC_ASN1_IMPL_DER_PC_TRAITS_H

#include<__vic/asn1/ber.h>
#include<__vic/asn1/types.h>
#include<type_traits>

namespace __vic { namespace ASN1 { namespace impl {

//----------------------------------------------------------------------------
template<class... Args>
constexpr BER::primitive_constructed der_constructness(const SEQUENCE<Args...> * )
{ return BER::constructed; }
//----------------------------------------------------------------------------
template<class T>
constexpr BER::primitive_constructed der_constructness(const SEQUENCE_OF<T> * )
{ return BER::constructed; }
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
constexpr BER::primitive_constructed der_constructness(const EXPLICIT<Tag,T,Cls> * )
{ return BER::constructed; }
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
constexpr BER::primitive_constructed der_constructness(const IMPLICIT<Tag,T,Cls> * )
{ return der_constructness(static_cast<const T *>(nullptr)); }
//----------------------------------------------------------------------------
constexpr BER::primitive_constructed der_constructness(const void * )
{ return BER::primitive; }
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T> constexpr BER::primitive_constructed der_constructness()
{
    return der_constructness(static_cast<const T *>(nullptr));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T> constexpr bool is_der_primitive()
{ return BER::is_primitive(der_constructness<T>()); }
//----------------------------------------------------------------------------
template<class T> constexpr bool is_der_constructed()
{ return BER::is_constructed(der_constructness<T>()); }
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
