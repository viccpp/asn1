// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_IMPL_BER_ENCODED_LENGTH_H
#define __VIC_ASN1_IMPL_BER_ENCODED_LENGTH_H

#include<__vic/asn1/impl/der_encoded_length.h>

namespace __vic { namespace ASN1 { namespace impl {

//----------------------------------------------------------------------------
// Using DER-functions
template<class T>
inline size_t ber_encoded_length(const T &v)
{
    return der_encoded_length(v);
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
