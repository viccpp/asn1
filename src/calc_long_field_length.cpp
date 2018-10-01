//
// $Id$
//

#include<__vic/asn1/ber.h>

namespace __vic { namespace ASN1 { namespace BER {

//----------------------------------------------------------------------------
size_t impl::calc_long_type_field_length(tag_number_t tag)
{
    size_t res = 1; // one byte for "use high-tag-number form" mark
    do res++; while(tag >>= 7);
    return res;
}
//----------------------------------------------------------------------------
size_t impl::calc_long_length_field_length(size_t len)
{
    size_t res = 1; // one byte for "length of the length field"
    do res++; while(len >>= 8);
    return res;
}
//----------------------------------------------------------------------------

}}} // namespace
