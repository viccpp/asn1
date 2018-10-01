//
// $Id: decoder_limits.cpp 833 2013-03-21 13:23:51Z vdyachenko $
//

#include<mfisoft/january/asn1/ber_decoder.h>

namespace mfisoft { namespace january { namespace ASN1 { namespace BER {

//----------------------------------------------------------------------------
void DecoderBase::push_limit(size_t n)
{
    if(is_limited() && n > bytes_avail())
        throw format_error("nested length is too big");
    limits_push(n);
}
//----------------------------------------------------------------------------
void DecoderBase::pop_limit()
{
    assert(!limits.empty());
    limit lim = limits_top();
    if(lim.avail()) throw truncated_stream_error(); // or programmer error...
    limits_drop();
    if(!limits.empty()) limits_top() -= lim.total();
}
//----------------------------------------------------------------------------

}}}} // namespace
