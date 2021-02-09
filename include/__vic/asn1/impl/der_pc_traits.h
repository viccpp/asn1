// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_DER_PC_TRAITS_H
#define __VIC_ASN1_IMPL_DER_PC_TRAITS_H

#include<__vic/asn1/ber.h>
#include<__vic/asn1/types.h>
#include<type_traits>

namespace __vic { namespace asn1 { namespace der {

//----------------------------------------------------------------------------
template<class... Args>
constexpr primitive_constructed constructness(const SEQUENCE<Args...> * )
{ return constructed; }
//----------------------------------------------------------------------------
template<class T>
constexpr primitive_constructed constructness(const SEQUENCE_OF<T> * )
{ return constructed; }
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
constexpr primitive_constructed constructness(const EXPLICIT<Tag,T,Cls> * )
{ return constructed; }
//----------------------------------------------------------------------------
template<tag_number_t Tag, class T, tag_class_t Cls>
constexpr primitive_constructed constructness(const IMPLICIT<Tag,T,Cls> * )
{ return constructness(static_cast<const T *>(nullptr)); }
//----------------------------------------------------------------------------
constexpr primitive_constructed constructness(const void * )
{ return primitive; }
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T> constexpr primitive_constructed constructness()
{
    return constructness(static_cast<const T *>(nullptr));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template<class T> constexpr bool is_primitive()
{ return is_primitive(constructness<T>()); }
//----------------------------------------------------------------------------
template<class T> constexpr bool is_constructed()
{ return is_constructed(constructness<T>()); }
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
