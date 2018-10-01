//
// $Id$
//

#include<__vic/asn1/types.h>
#include<cstring>
#include<cctype>

namespace __vic { namespace ASN1 {

//----------------------------------------------------------------------------
inline bool find_in(char ch, const char *palette)
{
    return std::strchr(palette, ch);
}
//----------------------------------------------------------------------------
inline bool is_printable_char(char ch)
{
    return std::isalnum(ch) || find_in(ch, " '()+,-./:=?");
}
//----------------------------------------------------------------------------
inline bool is_graphic_char(unsigned char ch)
{
    // On http://www.itscj.ipsj.or.jp/ISO-IR
    // http://www.itscj.ipsj.or.jp/ISO-IR/practice/B6-CodeTable8BitGCharSetNotConf2022.doc
    // + SPACE
    return ch >= '\x20' && ch != '\x7F';
}
//----------------------------------------------------------------------------
inline bool is_visible_char(unsigned char ch)
{
    // No.6 on http://www.itscj.ipsj.or.jp/ISO-IR/overview.htm
    // Direct link: http://www.itscj.ipsj.or.jp/ISO-IR/006.pdf
    // + SPACE
    return '\x20' <= ch && ch < '\x7F';
}
//----------------------------------------------------------------------------
inline bool is_numeric_char(char ch)
{
    return std::isdigit(ch) || ch == ' ';
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool PrintableString::is_valid() const
{
    for(char ch : *this)
        if(!is_printable_char(ch)) return false;
    return true;
}
//----------------------------------------------------------------------------
bool GraphicString::is_valid() const
{
    for(char ch : *this)
        if(!is_graphic_char(ch)) return false;
    return true;
}
//----------------------------------------------------------------------------
bool VisibleString::is_valid() const
{
    for(char ch : *this)
        if(!is_visible_char(ch)) return false;
    return true;
}
//----------------------------------------------------------------------------
bool NumericString::is_valid() const
{
    for(char ch : *this)
        if(!is_numeric_char(ch)) return false;
    return true;
}
//----------------------------------------------------------------------------

}} // namespace
