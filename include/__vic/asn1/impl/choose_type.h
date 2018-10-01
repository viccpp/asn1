// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_CHOOSE_TYPE_H
#define __VIC_ASN1_IMPL_CHOOSE_TYPE_H

namespace __vic { namespace ASN1 {

template<unsigned , class... > struct choose_type_ {}; // SFINAE-friendly
template<unsigned , class > struct choose_type_tuple_ {}; // SFINAE-friendly
//////////////////////////////////////////////////////////////////////////////
template<class Head, class... Tail>
struct choose_type_<0U, Head, Tail...> { using type = Head; };
//////////////////////////////////////////////////////////////////////////////
template<unsigned I, class Head, class... Tail>
struct choose_type_<I, Head, Tail...> : choose_type_<I - 1U, Tail...> {};
//////////////////////////////////////////////////////////////////////////////
template<unsigned I, template<class...> class Tuple, class... Types>
struct choose_type_tuple_<I, Tuple<Types...>> : choose_type_<I, Types...> {};
//////////////////////////////////////////////////////////////////////////////
template<unsigned I, class... Types>
using choose_type = typename choose_type_<I,Types...>::type;

template<unsigned I, class Tuple>
using choose_type_tuple = typename choose_type_tuple_<I,Tuple>::type;
//////////////////////////////////////////////////////////////////////////////

}} // namespace

#endif // header guard
