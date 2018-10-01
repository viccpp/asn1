// Common primitives for ASN.1 BER
//
// Platform: ISO C++ 98 / 11
// $Id: ber.h 1452 2014-05-16 09:30:44Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_BER_H
#define __MFISOFT_JANUARY_ASN1_BER_H

#include<mfisoft/january/defs.h>
#include<cstddef>
#include JAN_CSTDINT
#include<cassert>
#include<string>

namespace mfisoft { namespace january { namespace ASN1 { namespace BER {

using std::size_t;

typedef unsigned tag_number_t;
//////////////////////////////////////////////////////////////////////////////
enum tag_class_t
{
    universal        = 0, // 0 0
    application      = 1, // 0 1
    context_specific = 2, // 1 0
    private_         = 3  // 1 1
};
enum primitive_constructed
{
    primitive   = 0,
    constructed = 1
};
//////////////////////////////////////////////////////////////////////////////
class type_tag_t
{
    tag_number_t num_;
    tag_class_t cls_;
public:
    type_tag_t() JAN_DEFAULT_CTR
    JAN_CONSTEXPR_FUNC type_tag_t(tag_number_t t, tag_class_t cls)
        : num_(t), cls_(cls) {}
    JAN_CONSTEXPR_FUNC type_tag_t(tag_class_t cls, tag_number_t t)
        : num_(t), cls_(cls) {}

    JAN_CONSTEXPR_FUNC tag_number_t number() const { return num_; }
    JAN_CONSTEXPR_FUNC tag_class_t class_() const { return cls_; }

    tag_number_t &number_ref() { return num_; }
    tag_class_t &class_ref() { return cls_; }

    JAN_CONSTEXPR_FUNC bool operator==(type_tag_t o) const
        { return class_() == o.class_() && number() == o.number(); }
    JAN_CONSTEXPR_FUNC bool operator!=(type_tag_t o) const
        { return class_() != o.class_() || number() != o.number(); }
};
//////////////////////////////////////////////////////////////////////////////
class type_field_t
{
    type_tag_t tag_;
    primitive_constructed p_c_;
public:
    type_field_t() JAN_DEFAULT_CTR
    JAN_CONSTEXPR_FUNC type_field_t(type_tag_t t, primitive_constructed pc)
        : tag_(t), p_c_(pc) {}
    JAN_CONSTEXPR_FUNC type_field_t(
            tag_number_t t, tag_class_t cls, primitive_constructed pc)
        : tag_(t, cls), p_c_(pc) {}
    JAN_CONSTEXPR_FUNC type_field_t(
            tag_class_t cls, tag_number_t t, primitive_constructed pc)
        : tag_(t, cls), p_c_(pc) {}

    JAN_CONSTEXPR_FUNC type_tag_t tag() const { return tag_; }
    JAN_CONSTEXPR_FUNC tag_number_t tag_number() const { return tag_.number(); }
    JAN_CONSTEXPR_FUNC tag_class_t tag_class() const { return tag_.class_(); }

    JAN_CONSTEXPR_FUNC primitive_constructed p_c() const { return p_c_; }
    JAN_CONSTEXPR_FUNC bool is_primitive() const { return p_c_ == primitive; }
    JAN_CONSTEXPR_FUNC bool is_constructed() const { return p_c_ == constructed; }

    tag_number_t &tag_number_ref() { return tag_.number_ref(); }
    tag_class_t &tag_class_ref() { return tag_.class_ref(); }
    primitive_constructed &p_c_ref() { return p_c_; }

    JAN_CONSTEXPR_FUNC bool operator==(type_field_t o) const
        { return tag() == o.tag() && p_c() == o.p_c(); }
    JAN_CONSTEXPR_FUNC bool operator!=(type_field_t o) const
        { return tag() != o.tag() || p_c() != o.p_c(); }
};
//////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 201103L // C++11
constexpr type_tag_t EOC{universal, 0};
#endif

namespace impl {
size_t calc_long_type_field_length(tag_number_t );
size_t calc_long_length_field_length(size_t );
extern const char * const tag_class_names[4];
} // namespace

//----------------------------------------------------------------------------
JAN_CONSTEXPR_FUNC bool is_primitive(primitive_constructed p_c)
{
    return p_c == primitive;
}
//----------------------------------------------------------------------------
JAN_CONSTEXPR_FUNC bool is_constructed(primitive_constructed p_c)
{
    return p_c == constructed;
}
//----------------------------------------------------------------------------
inline const char *as_text(tag_class_t cls)
{
    return impl::tag_class_names[cls];
}
//----------------------------------------------------------------------------
inline size_t type_field_length(tag_number_t tag)
{
    return tag < 0x1FU ? 1 : impl::calc_long_type_field_length(tag);
}
//----------------------------------------------------------------------------
inline size_t type_field_length(type_tag_t t)
{
    return type_field_length(t.number());
}
//----------------------------------------------------------------------------
inline size_t length_field_length(size_t len)
{
    return len <= 0x7FU ? 1 : impl::calc_long_length_field_length(len);
}
//----------------------------------------------------------------------------
template<class TInt>
size_t integer_length(TInt n)
{
    size_t len = 1;
    n >>= 7; // BER-representation of INTEGER is always signed
    while(n != TInt(0) && ~n != TInt(0)) len++, n >>= 8;
    return len;
}
//----------------------------------------------------------------------------
void to_text(type_tag_t , std::string & );

inline std::string to_text(type_tag_t t)
{ std::string res; to_text(t, res); return res; }

}}}} // namespace

#endif // header guard
