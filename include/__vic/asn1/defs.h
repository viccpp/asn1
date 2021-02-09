// Common definitions for ASN.1
//
// Platform: ISO C++ 98 / 11
// $Id$

#ifndef __VIC_ASN1_DEFS_H
#define __VIC_ASN1_DEFS_H

#include<__vic/defs.h>
#include<__vic/string_buffer.h>
#include<string>

namespace __vic { namespace asn1 {

typedef unsigned tag_number_t;
//////////////////////////////////////////////////////////////////////////////
enum tag_class_t
#if __cplusplus >= 201103L // C++11
    : unsigned char
#endif
{
    universal        = 0, // 0 0
    application      = 1, // 0 1
    context_specific = 2, // 1 0
    private_         = 3  // 1 1
};
enum primitive_constructed
#if __cplusplus >= 201103L // C++11
    : unsigned char
#endif
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
    type_tag_t() __VIC_DEFAULT_CTR
    __VIC_CONSTEXPR_FUNC type_tag_t(tag_number_t t, tag_class_t cls)
        : num_(t), cls_(cls) {}
    __VIC_CONSTEXPR_FUNC type_tag_t(tag_class_t cls, tag_number_t t)
        : num_(t), cls_(cls) {}

    __VIC_CONSTEXPR_FUNC tag_number_t number() const { return num_; }
    __VIC_CONSTEXPR_FUNC tag_class_t class_() const { return cls_; }

    tag_number_t &number_ref() { return num_; }
    tag_class_t &class_ref() { return cls_; }
};
//////////////////////////////////////////////////////////////////////////////
class type_field_t
{
    tag_number_t num_;
    tag_class_t cls_;
    primitive_constructed p_c_;
public:
    type_field_t() __VIC_DEFAULT_CTR
    __VIC_CONSTEXPR_FUNC type_field_t(type_tag_t t, primitive_constructed pc)
        : num_(t.number()), cls_(t.class_()), p_c_(pc) {}
    __VIC_CONSTEXPR_FUNC type_field_t(
            tag_number_t t, tag_class_t cls, primitive_constructed pc)
        : num_(t), cls_(cls), p_c_(pc) {}
    __VIC_CONSTEXPR_FUNC type_field_t(
            tag_class_t cls, tag_number_t t, primitive_constructed pc)
        : num_(t), cls_(cls), p_c_(pc) {}

    __VIC_CONSTEXPR_FUNC type_tag_t tag() const { return type_tag_t(num_, cls_); }
    __VIC_CONSTEXPR_FUNC tag_number_t tag_number() const { return num_; }
    __VIC_CONSTEXPR_FUNC tag_class_t tag_class() const { return cls_; }

    __VIC_CONSTEXPR_FUNC primitive_constructed p_c() const { return p_c_; }
    __VIC_CONSTEXPR_FUNC bool is_primitive() const { return p_c_ == primitive; }
    __VIC_CONSTEXPR_FUNC bool is_constructed() const { return p_c_ == constructed; }

    tag_number_t &tag_number_ref() { return num_; }
    tag_class_t &tag_class_ref() { return cls_; }
    primitive_constructed &p_c_ref() { return p_c_; }
};
//////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 201103L // C++11
constexpr type_tag_t EOC{universal, 0};
#endif

//----------------------------------------------------------------------------
__VIC_CONSTEXPR_FUNC bool operator==(type_tag_t t1, type_tag_t t2)
{
    return t1.number() == t2.number() && t1.class_() == t2.class_();
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
    extern const char * const tag_class_names[4];
    return tag_class_names[cls];
}
//----------------------------------------------------------------------------
void to_text(type_tag_t , std::string & );

inline std::string to_text(type_tag_t t)
{ std::string res; to_text(t, res); return res; }

inline __vic::string_buffer &operator<<(__vic::string_buffer &s, type_tag_t t)
{ to_text(t, s); return s; }
#if __cpp_rvalue_references
inline __vic::string_buffer &&operator<<(__vic::string_buffer &&s, type_tag_t t)
{ to_text(t, s); return std::move(s); }
#endif

}} // namespace

#endif // header guard
