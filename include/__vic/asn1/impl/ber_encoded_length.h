// Internal implementation header
//
// Platform: ISO C++ 11
// $Id: ber_encoded_length.h 1041 2013-08-29 14:10:27Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_IMPL_BER_ENCODED_LENGTH_H
#define __MFISOFT_JANUARY_ASN1_IMPL_BER_ENCODED_LENGTH_H

#include<mfisoft/january/asn1/impl/der_encoded_length.h>

namespace mfisoft { namespace january { namespace ASN1 { namespace impl {

//----------------------------------------------------------------------------
// Using DER-functions
template<class T>
inline size_t ber_encoded_length(const T &v)
{
    return der_encoded_length(v);
}
//----------------------------------------------------------------------------

}}}} // namespace

#endif // header guard
