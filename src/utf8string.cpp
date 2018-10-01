//
// $Id: utf8string.cpp 1630 2014-12-23 08:25:47Z vdyachenko $
//

#include<mfisoft/january/asn1/types.h>
#include<mfisoft/january/letters/utf8.h>

namespace mfisoft { namespace january { namespace ASN1 {

//----------------------------------------------------------------------------
size_t UTF8String::length_chars() const
{
    return jan::utf8::length_chars(*this);
}
//----------------------------------------------------------------------------
bool UTF8String::is_valid() const
{
    return jan::utf8::is_valid(*this);
}
//----------------------------------------------------------------------------
template<>
void assign_trunc_(UTF8String &s, size_t max, const char *v)
{
    size_t bytes = 0;
    for(auto p = v; *p && max; max--)
    {
        auto ch_len = jan::utf8::length_of_char(p);
        bytes += ch_len;
        p += ch_len;
    }
    s.assign(v, bytes);
}
//----------------------------------------------------------------------------

}}} // namespace
