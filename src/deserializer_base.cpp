//
// $Id$
//

#include<__vic/asn1/impl/basic_deserializer.h>

namespace __vic { namespace ASN1 {

//----------------------------------------------------------------------------
void DeserializerBase::throw_unexpected_type(type_tag_t t, type_tag_t expected)
{
    throw bad_format(__vic::msg(128) <<
        "Expected type " << expected << " but found " << t);
}
//----------------------------------------------------------------------------
void DeserializerBase::throw_unexpected_type(type_tag_t t)
{
    throw bad_format(__vic::msg(128) << "Unexpected type " << t);
}
//----------------------------------------------------------------------------
void DeserializerBase::throw_cannot_read(type_tag_t t, const std::exception &ex)
{
    throw bad_format(__vic::msg(128) <<
        "Can't read " << t << ": " << ex.what());
}
//----------------------------------------------------------------------------
void DeserializerBase::throw_cannot_read(const char *t, const std::exception &ex)
{
    throw bad_format(__vic::msg(128) <<
        "Can't read " << t << ": " << ex.what());
}
//----------------------------------------------------------------------------
void DeserializerBase::throw_required_field_absent()
{
    throw bad_format("Required field not found");
}
//----------------------------------------------------------------------------

}} // namespace
