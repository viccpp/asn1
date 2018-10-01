//
// $Id: ber_decoder_error.cpp 815 2013-03-05 14:39:54Z vdyachenko $
//

#include<mfisoft/january/asn1/ber_decoder.h>
#include<mfisoft/january/string_buffer.h>

namespace mfisoft { namespace january { namespace ASN1 { namespace BER {

//----------------------------------------------------------------------------
DecoderBase::format_error::format_error(const char *msg)
    : error(jan::msg(128) << "BER format error: " << msg)
{
}
//----------------------------------------------------------------------------
DecoderBase::decoding_error::decoding_error(const char *msg)
    : error(jan::msg(128) << "BER decoding error: " << msg)
{
}
//----------------------------------------------------------------------------
DecoderBase::constructed_eoc_error::constructed_eoc_error()
    : format_error("only primitive encoding is allowed for EOC")
{
}
//----------------------------------------------------------------------------
DecoderBase::truncated_stream_error::truncated_stream_error()
    : format_error("truncated stream")
{
}
//----------------------------------------------------------------------------

}}}} // namespace
