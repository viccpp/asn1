//
// $Id$
//

#include<__vic/defs.h>
#include<__vic/asn1/defs.h>
#include<cstdio> // for std::snprintf()

namespace __vic { namespace asn1 {

namespace {
const char * const universal_types_names[] =
{
    "EOC",              // 0
    "BOOLEAN",          // 1
    "INTEGER",          // 2
    "BIT STRING",       // 3
    "OCTET STRING",     // 4
    "NULL",             // 5
    "OBJECT IDENTIFIER",// 6
    "ObjectDescriptor", // 7
    "EXTERNAL",         // 8
    "REAL",             // 9
    "ENUMERATED",       // 10
    "EMBEDDED PDV",     // 11
    "UTF8String",       // 12
    "RELATIVE-OID",     // 13
    "[UNIVERSAL 14]",   // 14
    "[UNIVERSAL	15]",   // 15
    "SEQUENCE (OF)",    // 16
    "SET (OF)",         // 17
    "NumericString",    // 18
    "PrintableString",  // 19
    "TeletexString",    // 20 (aka T61String)
    "VideotexString",   // 21
    "IA5String",        // 22
    "UTCTime",          // 23
    "GeneralizedTime",  // 24
    "GraphicString",    // 25
    "VisibleString",    // 26 (aka ISO646String)
    "GeneralString",    // 27
    "UniversalString",  // 28
    "CHARACTER STRING", // 29
    "BMPString"         // 30
};
} // namespace

//----------------------------------------------------------------------------
void to_text(type_tag_t t, std::string &res)
{
    bool is_context_spec = true;
    switch(t.class_())
    {
        case universal:
            if(t.number() < __vic::array_size(universal_types_names))
            {
                res += universal_types_names[t.number()];
                return;
            }
            // fall in
        case application:
        case private_:
            ((res += '[') += as_text(t.class_())) += ' ';
            is_context_spec = false;
            // fall in
        case context_specific:
        {
            if(is_context_spec) res += '[';
            // TODO: make more accurate & safe number to text conversion
            char buf[32];
            std::snprintf(buf, sizeof buf, "%u",
                            static_cast<unsigned>(t.number()));
            res += buf;
            res += ']';
        }
    }
}
//----------------------------------------------------------------------------

}} // namespace
