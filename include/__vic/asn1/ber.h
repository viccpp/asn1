// Common primitives for ASN.1 BER
//
// Platform: ISO C++ 98 / 11
// $Id$

#ifndef __VIC_ASN1_BER_H
#define __VIC_ASN1_BER_H

#include<__vic/asn1/defs.h>
#include<__vic/stdint.h>
#include<cstddef>
#include<cassert>

namespace __vic { namespace ASN1 { namespace BER {

using std::size_t;

namespace impl {
size_t calc_long_type_field_length(tag_number_t );
size_t calc_long_length_field_length(size_t );
} // namespace

//----------------------------------------------------------------------------
inline size_t type_field_length(tag_number_t tag)
{
    return tag < 0x1FU ? 1 : impl::calc_long_type_field_length(tag);
}
//----------------------------------------------------------------------------
inline size_t type_field_length(type_tag_t t)
{
    return type_field_length(t.number());
}
//----------------------------------------------------------------------------
inline size_t length_field_length(size_t len)
{
    return len <= 0x7FU ? 1 : impl::calc_long_length_field_length(len);
}
//----------------------------------------------------------------------------
template<class TInt>
size_t integer_length(TInt n)
{
    size_t len = 1;
    n >>= 7; // BER-representation of INTEGER is always signed
    while(n != TInt(0) && ~n != TInt(0)) len++, n >>= 8;
    return len;
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
