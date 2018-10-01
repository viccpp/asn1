//
// $Id$
//

#include<__vic/asn1/ber_decoder.h>
#include<__vic/string_buffer.h>

namespace __vic { namespace ASN1 { namespace BER {

//----------------------------------------------------------------------------
DecoderBase::format_error::format_error(const char *msg)
    : error(__vic::msg(128) << "BER format error: " << msg)
{
}
//----------------------------------------------------------------------------
DecoderBase::decoding_error::decoding_error(const char *msg)
    : error(__vic::msg(128) << "BER decoding error: " << msg)
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

}}} // namespace
