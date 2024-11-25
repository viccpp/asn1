//
// $Id$
//

#include<__vic/asn1/types.h>
#include<__vic/utf8/reader.h>
#include<__vic/sreaders/string.h>
#include<__vic/sreaders/cstring.h>

namespace __vic { namespace asn1 {

//----------------------------------------------------------------------------
size_t UTF8String::length_chars() const
{
    __vic::utf8::reader<__vic::string_sreader> rd(*this);
    size_t len = 0;
    while(rd()) len++;
    return len; // in code points
}
//----------------------------------------------------------------------------
bool UTF8String::is_valid() const
{
    __vic::utf8::reader<__vic::string_sreader> rd(*this);
    for(;;)
        switch(rd.parse().status())
        {
            case __vic::utf8::status::ok: break;
            case __vic::utf8::status::eof: return true;
            default: return false;
        }
}
//----------------------------------------------------------------------------
template<>
void assign_trunc_(UTF8String &s, size_t max_chars, const char *v)
{
    __vic::utf8::reader<__vic::cstring_sreader> rd(v);
    while(max_chars && rd()) max_chars--;
    s.assign(v, rd.get_byte_reader().position() - v);
}
//----------------------------------------------------------------------------

}} // namespace
