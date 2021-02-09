//
// $Id$
//

#include<__vic/asn1/types.h>
#include<__vic/utf8/reader.h>
#include<__vic/readers/string.h>
#include<__vic/readers/cstring.h>

namespace __vic { namespace asn1 {

//----------------------------------------------------------------------------
size_t UTF8String::length_chars() const
{
    __vic::utf8::reader<__vic::string_reader> r(*this);
    size_t len = 0;
    __vic::unicode_t cp;
    while(r.read(cp)) len++;
    return len; // in code points
}
//----------------------------------------------------------------------------
bool UTF8String::is_valid() const
{
    __vic::utf8::reader<__vic::string_reader> r(*this);
    __vic::unicode_t cp;
    for(;;)
        switch(r.parse(cp))
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
    __vic::utf8::reader<__vic::cstring_reader> r(v);
    __vic::unicode_t cp;
    while(max_chars && r.read(cp)) max_chars--;
    s.assign(v, r.get_byte_reader().position() - v);
}
//----------------------------------------------------------------------------

}} // namespace
