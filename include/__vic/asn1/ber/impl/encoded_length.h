// Internal implementation header
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_BER_IMPL_ENCODED_LENGTH_H
#define __VIC_ASN1_BER_IMPL_ENCODED_LENGTH_H

#include<__vic/asn1/der/impl/encoded_length.h>

namespace __vic { namespace asn1 { namespace ber {

//----------------------------------------------------------------------------
// Using DER-functions
template<class T>
inline size_t encoded_length(const T &v)
{
    return der::encoded_length(v);
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
