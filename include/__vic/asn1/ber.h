// Common primitives for ASN.1 BER
//
// Platform: ISO C++ 98 / 11
// $Id$

#ifndef __VIC_ASN1_BER_H
#define __VIC_ASN1_BER_H

#include<__vic/defs.h>
#include<__vic/stdint.h>
#include<cstddef>
#include<cassert>
#include<string>

namespace __vic { namespace ASN1 { namespace BER {

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
    tag_class_t cls_;
    tag_number_t num_;
public:
    type_tag_t() __VIC_DEFAULT_CTR
    __VIC_CONSTEXPR_FUNC type_tag_t(tag_number_t t, tag_class_t cls)
        : cls_(cls) , num_(t) {}
    __VIC_CONSTEXPR_FUNC type_tag_t(tag_class_t cls, tag_number_t t)
        : cls_(cls), num_(t) {}

    __VIC_CONSTEXPR_FUNC tag_number_t number() const { return num_; }
    __VIC_CONSTEXPR_FUNC tag_class_t class_() const { return cls_; }

    tag_number_t &number_ref() { return num_; }
    tag_class_t &class_ref() { return cls_; }
};
//////////////////////////////////////////////////////////////////////////////
class type_field_t
{
    type_tag_t tag_;
    primitive_constructed p_c_;
public:
    type_field_t() __VIC_DEFAULT_CTR
    __VIC_CONSTEXPR_FUNC type_field_t(type_tag_t t, primitive_constructed pc)
        : tag_(t), p_c_(pc) {}
    __VIC_CONSTEXPR_FUNC type_field_t(
            tag_number_t t, tag_class_t cls, primitive_constructed pc)
        : tag_(t, cls), p_c_(pc) {}
    __VIC_CONSTEXPR_FUNC type_field_t(
            tag_class_t cls, tag_number_t t, primitive_constructed pc)
        : tag_(t, cls), p_c_(pc) {}

    __VIC_CONSTEXPR_FUNC type_tag_t tag() const { return tag_; }
    __VIC_CONSTEXPR_FUNC tag_number_t tag_number() const { return tag_.number(); }
    __VIC_CONSTEXPR_FUNC tag_class_t tag_class() const { return tag_.class_(); }

    __VIC_CONSTEXPR_FUNC primitive_constructed p_c() const { return p_c_; }
    __VIC_CONSTEXPR_FUNC bool is_primitive() const { return p_c_ == primitive; }
    __VIC_CONSTEXPR_FUNC bool is_constructed() const { return p_c_ == constructed; }

    tag_number_t &tag_number_ref() { return tag_.number_ref(); }
    tag_class_t &tag_class_ref() { return tag_.class_ref(); }
    primitive_constructed &p_c_ref() { return p_c_; }
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
__VIC_CONSTEXPR_FUNC bool operator==(type_tag_t t1, type_tag_t t2)
{
    return t1.class_() == t2.class_() && t1.number() == t2.number();
}
//----------------------------------------------------------------------------
__VIC_CONSTEXPR_FUNC bool operator!=(type_tag_t t1, type_tag_t t2)
{
    return !(t1 == t2);
}
//----------------------------------------------------------------------------
// Canonical order as defined by ITU-T X.680, 8.6
__VIC_CONSTEXPR_FUNC bool operator<(type_tag_t t1, type_tag_t t2)
{
    return t1.class_() < t2.class_() ||
        (t1.class_() == t2.class_() && t1.number() < t2.number());
}
__VIC_CONSTEXPR_FUNC bool operator>(type_tag_t t1, type_tag_t t2)
{
    return t2 < t1;
}
__VIC_CONSTEXPR_FUNC bool operator>=(type_tag_t t1, type_tag_t t2)
{
    return !(t1 < t2);
}
__VIC_CONSTEXPR_FUNC bool operator<=(type_tag_t t1, type_tag_t t2)
{
    return !(t2 < t1);
}
//----------------------------------------------------------------------------
__VIC_CONSTEXPR_FUNC bool operator==(type_field_t t1, type_field_t t2)
{
    return t1.tag() == t2.tag() && t1.p_c() == t2.p_c();
}
__VIC_CONSTEXPR_FUNC bool operator!=(type_field_t t1, type_field_t t2)
{
    return !(t1 == t2);
}
//----------------------------------------------------------------------------
__VIC_CONSTEXPR_FUNC bool is_primitive(primitive_constructed p_c)
{
    return p_c == primitive;
}
//----------------------------------------------------------------------------
__VIC_CONSTEXPR_FUNC bool is_constructed(primitive_constructed p_c)
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

}}} // namespace

#endif // header guard
