//
// $Id$
//

#include<__vic/asn1/ber/decoder.h>
#include<__vic/string_buffer.h>

namespace __vic { namespace asn1 { namespace ber {

//----------------------------------------------------------------------------
decoder_base::format_error::format_error(const char *msg)
    : error(__vic::msg(128) << "BER format error: " << msg)
{
}
//----------------------------------------------------------------------------
decoder_base::decoding_error::decoding_error(const char *msg)
    : error(__vic::msg(128) << "BER decoding error: " << msg)
{
}
//----------------------------------------------------------------------------
decoder_base::constructed_eoc_error::constructed_eoc_error()
    : format_error("only primitive encoding is allowed for EOC")
{
}
//----------------------------------------------------------------------------
decoder_base::truncated_stream_error::truncated_stream_error()
    : format_error("truncated stream")
{
}
//----------------------------------------------------------------------------

}}} // namespace
