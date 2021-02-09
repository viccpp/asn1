// ASN.1-types
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_TYPES_H
#define __VIC_ASN1_TYPES_H

#if __cplusplus < 201103L
#error ISO C++11 compiler required
#endif

#include<__vic/asn1/defs.h>
#include<__vic/asn1/impl/choose_type.h>
#include<__vic/error.h>
#include<__vic/type_traits.h>
#include<initializer_list>
#include<type_traits>
#include<cstddef>
#include<utility>
#include<cstdint>
#include<cstring>
#include<cassert>
#include<limits>
#include<string>
#include<memory>
#include<vector>

namespace __vic { namespace ASN1 {

using std::size_t;

// For tagged types
//constexpr auto UNIVERSAL = universal; // not useful
constexpr auto APPLICATION = application;
constexpr auto PRIVATE = private_;

#define FORWARD_BASE_OPS(T, base) \
    T() = default; \
    T(const T & ) = default; \
    T(T && ) = default; \
    template<class... Args> \
    T(Args&&... args) : base(std::forward<Args>(args)...) {} \
    T &operator=(const T & ) = default; \
    T &operator=(T && ) = default; \
    template<class U> \
    T &operator=(U &&v) { base::operator=(std::forward<U>(v)); return *this; }

//////////////////////////////////////////////////////////////////////////////
// Wrapper class for unparsed binary data (e.g. piece of BER-stream)
class raw
{
    std::string v;
public:
    using value_type = uint8_t;
    using const_iterator = const uint8_t *;

    raw() = default;
    raw(const void *data, size_t len)
        : v(static_cast<const char *>(data), len) {}

    const uint8_t *bytes() const
        { return reinterpret_cast<const uint8_t *>(v.data()); }
    size_t length() const { return v.length(); }

    const_iterator begin() const { return bytes(); }
    const_iterator end() const   { return bytes() + length(); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const   { return end(); }

    raw &assign(const void *data, size_t n)
        { v.assign(static_cast<const char *>(data), n); return *this; }
    raw &append(const void *data, size_t n)
        { v.append(static_cast<const char *>(data), n); return *this; }
    raw &assign(size_t n, uint8_t b) { v.assign(n, b); return *this; }
    raw &append(size_t n, uint8_t b) { v.append(n, b); return *this; }
    raw &assign(uint8_t b) { v  = static_cast<char>(b); return *this; }
    raw &append(uint8_t b) { v += static_cast<char>(b); return *this; }

    bool empty() const { return v.empty(); }
    void reserve(size_t n) { v.reserve(n); }
    size_t capacity() const { return v.capacity(); }
    void clear() { v.clear(); }
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
struct OCTET_STRING : public raw
{
    static constexpr type_tag_t tag() { return {universal, 4}; }

    OCTET_STRING() = default;
    OCTET_STRING(const void *data, size_t len) : raw(data, len) {}

    OCTET_STRING &assign(const void *data, size_t n)
        { raw::assign(data, n); return *this; }
    OCTET_STRING &append(const void *data, size_t n)
        { raw::append(data, n); return *this; }
    OCTET_STRING &assign(size_t n, uint8_t b)
        { raw::assign(n, b); return *this; }
    OCTET_STRING &append(size_t n, uint8_t b)
        { raw::append(n, b); return *this; }
    OCTET_STRING &assign(uint8_t b)
        { raw::assign(b); return *this; }
    OCTET_STRING &append(uint8_t b)
        { raw::append(b); return *this; }

    raw &as_raw() { return *this; }
    const raw &as_raw() const { return *this; }
};
//////////////////////////////////////////////////////////////////////////////
class CHARACTER_STRING : public std::string
{
public:
    static constexpr type_tag_t tag() { return {universal, 29}; }
    FORWARD_BASE_OPS(CHARACTER_STRING, std::string)
    const char *bytes() const { return this->data(); }
    // inherited or redefined by descendants
    size_t length_chars() const { return this->length(); }
};
//////////////////////////////////////////////////////////////////////////////
// Internal implementation class
class RestrictedCharacterStringType : public CHARACTER_STRING
{
    using CHARACTER_STRING::tag; // Hide. Must be overriden
public:
    FORWARD_BASE_OPS(RestrictedCharacterStringType, CHARACTER_STRING)
};
//////////////////////////////////////////////////////////////////////////////
struct PrintableString : public RestrictedCharacterStringType
{
    static constexpr type_tag_t tag() { return {universal, 19}; }
    FORWARD_BASE_OPS(PrintableString, RestrictedCharacterStringType)
    bool is_valid() const;
};
//////////////////////////////////////////////////////////////////////////////
struct GraphicString : public RestrictedCharacterStringType
{
    static constexpr type_tag_t tag() { return {universal, 25}; }
    FORWARD_BASE_OPS(GraphicString, RestrictedCharacterStringType)
    bool is_valid() const;
};
//////////////////////////////////////////////////////////////////////////////
struct VisibleString : public RestrictedCharacterStringType
{
    static constexpr type_tag_t tag() { return {universal, 26}; }
    FORWARD_BASE_OPS(VisibleString, RestrictedCharacterStringType)
    bool is_valid() const;
};
using ISO646String = VisibleString;
//////////////////////////////////////////////////////////////////////////////
struct NumericString : public RestrictedCharacterStringType
{
    static constexpr type_tag_t tag() { return {universal, 18}; }
    FORWARD_BASE_OPS(NumericString, RestrictedCharacterStringType)
    bool is_valid() const;
};
//////////////////////////////////////////////////////////////////////////////
struct UTF8String : public RestrictedCharacterStringType
{
    static constexpr type_tag_t tag() { return {universal, 12}; }
    FORWARD_BASE_OPS(UTF8String, RestrictedCharacterStringType)
    bool is_valid() const;
    size_t length_chars() const;
};
//////////////////////////////////////////////////////////////////////////////
struct ObjectDescriptor : public GraphicString
{
    static constexpr type_tag_t tag() { return {universal, 7}; }
    FORWARD_BASE_OPS(ObjectDescriptor, GraphicString)
};
//////////////////////////////////////////////////////////////////////////////
struct UTCTime : public VisibleString
{
    static constexpr type_tag_t tag() { return {universal, 23}; }
    FORWARD_BASE_OPS(UTCTime, VisibleString)
};
//////////////////////////////////////////////////////////////////////////////
struct GeneralizedTime : public VisibleString
{
    static constexpr type_tag_t tag() { return {universal, 24}; }
    FORWARD_BASE_OPS(GeneralizedTime, VisibleString)
};
//////////////////////////////////////////////////////////////////////////////
template<class Int = int>
class integer
{
    Int val;
public:
    using int_type = Int;
    static constexpr type_tag_t tag() { return {universal, 2}; }

    integer() = default;
    constexpr integer(Int n) : val{n} {}
    template<class Int2>
    constexpr integer(integer<Int2> n) : val{n.as_int()} {}

    integer &operator=(Int n) { val = n; return *this; }
    template<class Int2>
    integer &operator=(integer<Int2> n)
        { val = Int{n.as_int()}; return *this; }

    constexpr Int as_int() const { return val; }
    constexpr operator Int() const { return as_int(); }
};
//////////////////////////////////////////////////////////////////////////////
template<class Int = int,
    Int LowPoint = std::numeric_limits<Int>::min(),
    Int HiPoint  = std::numeric_limits<Int>::max()>
class INTEGER : public integer<Int>
{
    static_assert(LowPoint <= HiPoint, "Invalid range constraint");
    using base = integer<Int>;
public:
    static constexpr Int min() { return LowPoint; }
    static constexpr Int max() { return HiPoint; }

    INTEGER() = default;
    constexpr INTEGER(Int n) : base(n) {}
    template<class Int2, Int2 L, Int2 H>
    constexpr INTEGER(INTEGER<Int2,L,H> n) : base(n) {}

    INTEGER &operator=(Int n) { base::operator=(n); return *this; }
    template<class Int2, Int2 L, Int2 H>
    INTEGER &operator=(INTEGER<Int2,L,H> n)
        { base::operator=(n); return *this; }

    constexpr bool is_valid() const
        { return min() <= *this && *this <= max(); }
};
//////////////////////////////////////////////////////////////////////////////
template<class Enum>
class ENUMERATED
{
    static_assert(std::is_enum<Enum>(), "enum-type required for ENUMERATED");
    Enum val;
public:
    using enum_type = Enum;
    using int_type = typename std::underlying_type<Enum>::type;

    static constexpr type_tag_t tag() { return {universal, 10}; }

    ENUMERATED() = default;
    constexpr ENUMERATED(Enum v) : val{v} {}

    ENUMERATED &operator=(Enum v) { val = v; return *this; }
    ENUMERATED &assign(int_type );

    constexpr Enum as_enum() const { return val; }
    constexpr operator Enum() const { return as_enum(); }
    constexpr int_type as_int() const { return static_cast<int_type>(val); }
};
//////////////////////////////////////////////////////////////////////////////
class BOOLEAN
{
    bool val;
public:
    static constexpr type_tag_t tag() { return {universal, 1}; }

    BOOLEAN() = default;
    constexpr BOOLEAN(bool v) : val{v} {}

    BOOLEAN &operator=(bool v) { val = v; return *this; }
    constexpr bool as_bool() const { return val; }
    constexpr operator bool() const { return as_bool(); }
};
//////////////////////////////////////////////////////////////////////////////
template<class Real = double>
class REAL
{
    Real val;
public:
    using real_type = Real;
    static constexpr type_tag_t tag() { return {universal, 9}; }

    REAL() = default;
    constexpr REAL(Real n) : val{n} {}
    template<class Real2>
    constexpr REAL(REAL<Real2> n) : val{n.as_real()} {}

    REAL &operator=(Real n) { val = n; return *this; }
    template<class Real2>
    REAL &operator=(REAL<Real2> n)
        { val = Real{n.as_real()}; return *this; }
    constexpr Real as_real() const { return val; }
    constexpr operator Real() const { return as_real(); }
};
//////////////////////////////////////////////////////////////////////////////
struct NULL_
{
    static constexpr type_tag_t tag() { return {universal, 5}; }
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Unparsed types with internal BER-representation
//////////////////////////////////////////////////////////////////////////////
template<>
struct integer<raw> : public raw
{
    static constexpr type_tag_t tag() { return {universal, 2}; }
    raw &as_raw() { return *this; }
    const raw &as_raw() const { return *this; }
};
//////////////////////////////////////////////////////////////////////////////
template<>
struct REAL<raw> : public raw
{
    static constexpr type_tag_t tag() { return {universal, 9}; }
    raw &as_raw() { return *this; }
    const raw &as_raw() const { return *this; }
};
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// This function template can be specialized to enable checking of
// correspondence of arbitrary integer value to any allowed enum value.
// I.e. function returns true if integer value can be safely casted to enum.
// No checking is performed if no specialization done
template<class Enum>
inline bool is_enum_value(typename std::underlying_type<Enum>::type )
{ return true; }
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
struct invalid_enum_value : public std::exception
{
    const char *what() const noexcept { return "Invalid ENUMERATED value"; }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class Enum>
inline ENUMERATED<Enum> &ENUMERATED<Enum>::assign(int_type v)
{
    if(!is_enum_value<Enum>(v)) throw invalid_enum_value{};
    return *this = static_cast<Enum>(v);
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Implicitly tagged type e.g. [APPLICATION 1] IMPLICIT
template<tag_number_t Tag, class T, tag_class_t Cls = context_specific>
class IMPLICIT : public T
{
public:
    using value_type = T;
    static constexpr type_tag_t tag() { return {Cls, Tag}; }

    FORWARD_BASE_OPS(IMPLICIT, T)

    const T &unwrap() const { return *this; }
    T &unwrap() { return *this; }
};
//////////////////////////////////////////////////////////////////////////////
// Explicitly tagged type e.g. [APPLICATION 1] EXPLICIT
template<tag_number_t Tag, class T, tag_class_t Cls = context_specific>
class EXPLICIT : public T
{
public:
    using value_type = T;
    static constexpr type_tag_t tag() { return {Cls, Tag}; }

    FORWARD_BASE_OPS(EXPLICIT, T)

    const T &unwrap() const { return *this; }
    T &unwrap() { return *this; }
};
//////////////////////////////////////////////////////////////////////////////
template<class T>
class OPTIONAL
{
    std::unique_ptr<T> p;
public:
    using value_type = T;
    static constexpr type_tag_t tag() { return T::tag(); }

    OPTIONAL() = default;
    OPTIONAL(OPTIONAL && ) = default;
    OPTIONAL(const OPTIONAL &v) : p(v.is_set() ? new T{v.get()} : nullptr) {}
    template<class... Args>
    explicit OPTIONAL(Args&&... args) : p(new T{std::forward<Args>(args)...}) {}

    T &get() { assert(is_set()); return *p; }
    const T &get() const { assert(is_set()); return *p; }
    template<class U> void set(U && ); // U is type, assignable to T
    template<class U> void set(const OPTIONAL<U> & ) = delete;
    T &set_default() { if(!p) p.reset(new T); return *p; }

    OPTIONAL &operator=(OPTIONAL && ) = default;
    OPTIONAL &operator=(const OPTIONAL & );
    template<class U>
    OPTIONAL &operator=(U &&v) { set(std::forward<U>(v)); return *this; }

    bool is_set() const { return static_cast<bool>(p); }
    void clear() { p.reset(); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class T>
template<class U>
inline void OPTIONAL<T>::set(U &&v)
{
    if(p) *p = std::forward<U>(v); else p.reset(new T{std::forward<U>(v)});
}
//----------------------------------------------------------------------------
template<class T>
OPTIONAL<T> &OPTIONAL<T>::operator=(const OPTIONAL<T> &v)
{
    if(v.is_set()) set(v.get()); else clear();
    return *this;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
template<class... >
class sequence_elements // no elements case
{
public:
    template<class F> void for_each(F && ) {}
    template<class F> void for_each(F && ) const {}
};
//////////////////////////////////////////////////////////////////////////////
template<class Head>
class sequence_elements<Head>
{
    Head head_;
public:
    Head &head() { return head_; }
    const Head &head() const { return head_; }
    sequence_elements<> tail() const { return sequence_elements<>{}; }

    template<unsigned I>
    typename std::enable_if<I == 0U, Head &>::type get() { return head(); }
    template<unsigned I>
    typename std::enable_if<I == 0U, const Head &>::type get() const
        { return head(); }

    template<class F> void for_each(F &&f) { std::forward<F>(f)(head()); }
    template<class F> void for_each(F &&f) const { std::forward<F>(f)(head()); }
};
//////////////////////////////////////////////////////////////////////////////
template<class Head, class... Tail>
class sequence_elements<Head, Tail...>
{
    Head head_;
    sequence_elements<Tail...> tail_;
public:
    Head &head() { return head_; }
    sequence_elements<Tail...> &tail() { return tail_; }
    const Head &head() const { return head_; }
    const sequence_elements<Tail...> &tail() const { return tail_; }

    template<unsigned I>
    typename std::enable_if<I == 0U, Head &>::type get() { return head(); }
    template<unsigned I>
    typename std::enable_if<I == 0U, const Head &>::type get() const
        { return head(); }

    template<unsigned I>
    typename std::enable_if<(I > 0U && I <= sizeof...(Tail) + 1U),
        choose_type<I, Head, Tail...>
    >::type &get() { return tail().template get<I - 1U>(); }
    template<unsigned I>
    typename std::enable_if<(I > 0U && I <= sizeof...(Tail) + 1U),
        choose_type<I, Head, Tail...>
    >::type const &get() const { return tail().template get<I - 1U>(); }

    template<class F> void for_each(F &&f)
        { std::forward<F>(f)(head()); tail().for_each(std::forward<F>(f)); }
    template<class F> void for_each(F &&f) const
        { std::forward<F>(f)(head()); tail().for_each(std::forward<F>(f)); }
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
template<class... Elems>
class SEQUENCE
{
    static_assert(sizeof...(Elems) > 0, "Empty SEQUENCE is disallowed");
    sequence_elements<Elems...> elems;
protected:
    using self_type = SEQUENCE<Elems...>;
public:
    using elements_type = sequence_elements<Elems...>;
    static constexpr type_tag_t tag() { return {universal, 16}; }
    static constexpr unsigned size() { return sizeof...(Elems); }

    SEQUENCE();
    SEQUENCE(SEQUENCE && ) noexcept(
        std::is_nothrow_move_constructible<elements_type>());
    SEQUENCE(const SEQUENCE & );
    ~SEQUENCE();

    SEQUENCE &operator=(SEQUENCE && ) noexcept(
        std::is_nothrow_move_assignable<elements_type>());
    SEQUENCE &operator=(const SEQUENCE & );

    template<unsigned I>
    choose_type<I, Elems...> &get()
        { return elems.template get<I>(); }
    template<unsigned I>
    const choose_type<I, Elems...> &get() const
        { return elems.template get<I>(); }

    elements_type &elements() { return elems; }
    const elements_type &elements() const { return elems; }

    template<class F>
    void for_each(F &&f) { elements().for_each(std::forward<F>(f)); }
    template<class F>
    void for_each(F &&f) const { elements().for_each(std::forward<F>(f)); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define SEQ_FIELD(index,name) \
    ::__vic::ASN1::choose_type_tuple<index,self_type> \
        &name() { return this->get<index>(); } \
    ::__vic::ASN1::choose_type_tuple<index,self_type> \
        const &name() const { return this->get<index>(); }
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...>::SEQUENCE() = default;
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...>::SEQUENCE(SEQUENCE<Elems...> && )
    noexcept(std::is_nothrow_move_constructible<elements_type>()) = default;
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...>::SEQUENCE(const SEQUENCE<Elems...> & ) = default;
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...>::~SEQUENCE() = default;
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...> &SEQUENCE<Elems...>::operator=(SEQUENCE<Elems...> && )
    noexcept(std::is_nothrow_move_assignable<elements_type>()) = default;
//----------------------------------------------------------------------------
template<class... Elems>
SEQUENCE<Elems...> &SEQUENCE<Elems...>::operator=(const SEQUENCE<Elems...> & )
    = default;
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
template<
    class T,
    template<class , class = std::allocator<T>> class SeqCont = std::vector
>
class SEQUENCE_OF : public SeqCont<T>
{
public:
    using container_type = SeqCont<T>;
    static constexpr type_tag_t tag() { return {universal, 16}; }

    //using container_type::container_type;
    SEQUENCE_OF() = default;
    SEQUENCE_OF(SEQUENCE_OF && ) = default;
    SEQUENCE_OF(const SEQUENCE_OF & ) = default;
    SEQUENCE_OF(std::initializer_list<T> lst) : container_type(lst) {}
    template<class InputIterator>
    SEQUENCE_OF(InputIterator first, InputIterator last)
        : container_type(first, last) {}

    using container_type::operator=;
    SEQUENCE_OF &operator=(SEQUENCE_OF && ) = default;
    SEQUENCE_OF &operator=(const SEQUENCE_OF & ) = default;

    T &push_default() { this->emplace_back(); return this->back(); }
};
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Functions for SIZE::assign_trunc() implementation
//----------------------------------------------------------------------------
template<class StrT>
void assign_trunc_(StrT &s, size_t max, const std::string &v)
{ s.assign(v, 0, max); }
//----------------------------------------------------------------------------
template<class StrT>
void assign_trunc_(StrT &s, size_t max, const char *v)
{ while(*v && max--) s += *v++; }
//----------------------------------------------------------------------------
template<>
void assign_trunc_(UTF8String & , size_t , const char * );
//----------------------------------------------------------------------------
template<>
inline void assign_trunc_(UTF8String &s, size_t max, const std::string &v)
{ assign_trunc_(s, max, v.c_str()); }
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Common version used for restricted character string types
template<class StrT, size_t MinLength, size_t MaxLength = MinLength>
struct SIZE : public StrT
{
    static_assert(std::is_base_of<RestrictedCharacterStringType, StrT>(),
        "Size constraint unapplicable for this type");
    static_assert(MinLength <= MaxLength, "Invalid size constraint");

    static constexpr size_t min_length() { return MinLength; }
    static constexpr size_t max_length() { return MaxLength; }

    FORWARD_BASE_OPS(SIZE, StrT)

    SIZE &assign_trunc(const std::string &st)
        { assign_trunc_(unwrap(), max_length(), st); return *this; }
    SIZE &assign_trunc(const char *st)
        { assign_trunc_(unwrap(), max_length(), st); return *this; }
    SIZE &pad_min(char ch = ' ')
    {
        auto len = this->length_chars();
        if(len < min_length()) this->append(min_length() - len, ch);
        return *this;
    }

    StrT &unwrap() { return *this; }
    const StrT &unwrap() const { return *this; }

    bool is_valid() const
    {
        return StrT::is_valid() &&
            min_length() <= this->length_chars() &&
                            this->length_chars() <= max_length();
    }
};
//////////////////////////////////////////////////////////////////////////////
template<size_t MinLength, size_t MaxLength>
struct SIZE<OCTET_STRING,MinLength,MaxLength> : public OCTET_STRING
{
    static_assert(MinLength <= MaxLength, "Invalid size constraint");

    static constexpr size_t min_length() { return MinLength; }
    static constexpr size_t max_length() { return MaxLength; }

    // using OCTET_STRING::OCTET_STRING;
    template<class... Args>
    SIZE(Args&&... args) : OCTET_STRING(std::forward<Args>(args)...) {}

    SIZE &pad_min(uint8_t b = 0)
    {
        auto len = this->length();
        if(len < min_length()) this->append(min_length() - len, b);
        return *this;
    }

    OCTET_STRING &unwrap() { return *this; }
    const OCTET_STRING &unwrap() const { return *this; }

    bool is_valid() const
        { return min_length() <= length() && length() <= max_length(); }
};
//////////////////////////////////////////////////////////////////////////////
template<size_t MinLength, size_t MaxLength>
struct SIZE<CHARACTER_STRING,MinLength,MaxLength> : public CHARACTER_STRING
{
    static_assert(MinLength <= MaxLength, "Invalid size constraint");

    static constexpr size_t min_length() { return MinLength; }
    static constexpr size_t max_length() { return MaxLength; }

    FORWARD_BASE_OPS(SIZE, CHARACTER_STRING)

    SIZE &assign_trunc(const std::string &st)
        { this->assign(st, 0, max_length()); return *this; }
    SIZE &assign_trunc(const char *st)
    {
        auto n = max_length();
        while(*st && n--) *this += *st++;
        return *this;
    }
    SIZE &pad_min(char ch = ' ')
    {
        auto len = this->length_chars();
        if(len < min_length()) this->append(min_length() - len, ch);
        return *this;
    }

    CHARACTER_STRING &unwrap() { return *this; }
    const CHARACTER_STRING &unwrap() const { return *this; }

    bool is_valid() const
        { return min_length() <= length_chars() &&
                                 length_chars() <= max_length(); }
};
//////////////////////////////////////////////////////////////////////////////

namespace impl {
//////////////////////////////////////////////////////////////////////////////
struct choice_option_base
{
    virtual ~choice_option_base() = default;
    virtual type_tag_t tag() const = 0;
    virtual choice_option_base *clone() const = 0;
};
//////////////////////////////////////////////////////////////////////////////
template<class T>
class choice_option : public choice_option_base
{
    T val;
public:
    using value_type = T;

    choice_option() = default;
    choice_option(const T &v) : val(v) {}
    choice_option(T &&v) : val(std::move(v)) {}

    choice_option &operator=(const T &v) { val = v; return *this; }
    choice_option &operator=(T &&v) { val = std::move(v); return *this; }

    T &unwrap() { return val; }
    const T &unwrap() const { return val; }

    type_tag_t tag() const override { return val.tag(); }
    choice_option *clone() const override { return new choice_option<T>{val}; }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<unsigned I, class Choice, class Func>
auto choice_apply_thunk(Choice &ch, Func &&f)
    -> decltype(std::forward<Func>(f)(ch.template get_unchecked<I>()))
{
    return std::forward<Func>(f)(ch.template get_unchecked<I>());
}
//----------------------------------------------------------------------------
template<unsigned I, class Choice, class Func>
auto choice_set_default_and_apply_thunk(Choice &ch, Func &&f)
    -> decltype(std::forward<Func>(f)(ch.template set_default<I>()))
{
    return std::forward<Func>(f)(ch.template set_default<I>());
}
//----------------------------------------------------------------------------
template<class Choice, class Func, size_t... I>
inline auto choice_apply_impl(Choice &ch, Func &&f, __vic::index_sequence<I...> )
    -> decltype(choice_apply_thunk<0>(ch, std::forward<Func>(f)))
{
    static const decltype(&choice_apply_thunk<0,Choice,Func>)
        func[] = { &choice_apply_thunk<I>... };
    // Hand-made virtual call
    return (*func[ch.index()])(ch, std::forward<Func>(f));
}
//----------------------------------------------------------------------------
template<class Choice, class Func, size_t... I>
inline auto choice_set_default_and_apply_impl(
        Choice &ch, unsigned idx, Func &&f, __vic::index_sequence<I...> )
    -> decltype(choice_set_default_and_apply_thunk<0>(ch, std::forward<Func>(f)))
{
    static const decltype(&choice_set_default_and_apply_thunk<0,Choice,Func>)
        func[] = { &choice_set_default_and_apply_thunk<I>... };
    // Hand-made virtual call
    return (*func[idx])(ch, std::forward<Func>(f));
}
//----------------------------------------------------------------------------
template<class Choice, class Func>
inline auto choice_apply(Choice &ch, Func &&f)
    -> decltype(choice_apply_thunk<0>(ch, std::forward<Func>(f)))
{
    return choice_apply_impl(ch, std::forward<Func>(f),
        __vic::make_index_sequence<Choice::size()>{});
}
//----------------------------------------------------------------------------
template<class Choice, class Func>
inline auto choice_set_default_and_apply(Choice &ch, unsigned idx, Func &&f)
    -> decltype(choice_set_default_and_apply_thunk<0>(ch, std::forward<Func>(f)))
{
    return choice_set_default_and_apply_impl(ch, idx, std::forward<Func>(f),
        __vic::make_index_sequence<Choice::size()>{});
}
//----------------------------------------------------------------------------
} // namespace

//////////////////////////////////////////////////////////////////////////////
struct invalid_choice_index : public std::exception
{
    const char *what() const noexcept { return "Invalid CHOICE index"; }
};
//////////////////////////////////////////////////////////////////////////////
struct invalid_choice_tag : public std::exception
{
    const char *what() const noexcept
        { return "CHOICE doesn't have option with specified tag"; }
};
//////////////////////////////////////////////////////////////////////////////
struct choice_is_not_set : public std::exception
{
    const char *what() const noexcept { return "CHOICE isn't set"; }
};
//////////////////////////////////////////////////////////////////////////////
template<class... Opts>
class CHOICE
{
public:
    static constexpr unsigned size() { return sizeof...(Opts); }
    static_assert(size() != 0, "CHOICE must have at least one option");
    template<unsigned I> using option_type = choose_type<I,Opts...>;
    template<unsigned I> option_type<I> &set_default();
private:
    int curr = -1;
    std::unique_ptr<impl::choice_option_base> p;

    template<unsigned I>
    using wrapped_option_type = impl::choice_option<option_type<I>>;

    template<unsigned I, class Func>
    auto choose_and_apply(type_tag_t tag, Func &&f) ->
        typename std::enable_if< (I == size()-1U),
        decltype(std::forward<Func>(f)(this->set_default<I>()))>::type
    {
        if(option_type<I>::tag() == tag)
            return std::forward<Func>(f)(set_default<I>());
        else throw invalid_choice_tag{};
    }
    template<unsigned I, class Func>
    auto choose_and_apply(type_tag_t tag, Func &&f) ->
        typename std::enable_if< (I < size()-1U),
        decltype(std::forward<Func>(f)(this->set_default<I>()))>::type
    {
        if(option_type<I>::tag() == tag)
            return std::forward<Func>(f)(set_default<I>());
        else return choose_and_apply<I+1>(tag, std::forward<Func>(f));
    }
protected:
    using self_type = CHOICE<Opts...>;
public:
    CHOICE() = default;
    CHOICE(const CHOICE &o)
        : curr(o.curr), p(o.is_set() ? o.p->clone() : nullptr) {}
    CHOICE(CHOICE &&o) noexcept
        : curr(o.curr), p(std::move(o.p)) { o.curr = -1; }

    CHOICE &operator=(const CHOICE &o) { CHOICE(o).swap(*this); return *this; }
    CHOICE &operator=(CHOICE &&o) noexcept { swap(o); return *this; }

    template<unsigned I> option_type<I> &get()
    {
        if(I != curr) throw invalid_choice_index{};
        return get_unchecked<I>();
    }
    template<unsigned I> const option_type<I> &get() const
    {
        if(I != curr) throw invalid_choice_index{};
        return get_unchecked<I>();
    }

    template<unsigned I> option_type<I> &get_unchecked()
    {
        //static_assert(I < size(), "Invalid index");
        assert(is_set());
        return static_cast<wrapped_option_type<I> &>(*p).unwrap();
    }
    template<unsigned I> const option_type<I> &get_unchecked() const
    {
        assert(is_set());
        return static_cast<const wrapped_option_type<I> &>(*p).unwrap();
    }

    template<unsigned I> void set(const option_type<I> &v)
    {
        p.reset(new wrapped_option_type<I>{v});
        curr = I;
    }
    template<unsigned I> void set(option_type<I> &&v)
    {
        p.reset(new wrapped_option_type<I>{std::move(v)});
        curr = I;
    }

    // Applies generic function to the current option
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply_unchecked(Func &&f)
    {
        return impl::choice_apply(*this, std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply_unchecked(Func &&f) const
    {
        return impl::choice_apply(*this, std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply(Func &&f)
    {
        validate();
        return apply_unchecked(std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply(Func &&f) const
    {
        validate();
        return apply_unchecked(std::forward<Func>(f));
    }

    // Calls set_default<idx>() and applies generic function to the created option
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    set_default_and_apply(unsigned idx, Func &&f)
    {
        if(idx >= size()) throw invalid_choice_index{};
        return impl::choice_set_default_and_apply(
                        *this, idx, std::forward<Func>(f));
    }
    // Finds option with specified tag, sets default value and applies generic function to it
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    set_default_and_apply(type_tag_t tag, Func &&f)
    {
        return choose_and_apply<0>(tag, std::forward<Func>(f));
    }

    type_tag_t tag() const { assert(is_set()); return p->tag(); }
    void validate() const { if(!is_set()) throw choice_is_not_set{}; }

    int index() const { return curr; }
    bool is_set() const { return index() != -1; }
    void clear() { p.reset(); curr = -1; }
    void swap(CHOICE &o) noexcept { std::swap(curr, o.curr); p.swap(o.p); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class... Opts>
template<unsigned I>
auto CHOICE<Opts...>::set_default() -> option_type<I> &
{
    if(static_cast<int>(I) == curr) return get_unchecked<I>();
    auto obj_ptr = new wrapped_option_type<I>;
    p.reset(obj_ptr);
    curr = I;
    return obj_ptr->unwrap();
}
//----------------------------------------------------------------------------
#define CHOICE_FIELD(index,name) \
    self_type::option_type<index> \
        &name() { return this->get<index>(); } \
    self_type::option_type<index> \
        const &name() const { return this->get<index>(); } \
    self_type::option_type<index> \
        &set_##name() { return this->set_default<index>(); }
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
template<const char *Id, class T>
class CLASS_OPTION : public impl::choice_option<T>
{
public:
    static constexpr const char *id() { return Id; }
    FORWARD_BASE_OPS(CLASS_OPTION, impl::choice_option<T>)
    CLASS_OPTION *clone() const override
        { return new CLASS_OPTION<Id,T>{this->unwrap()}; }
};
//////////////////////////////////////////////////////////////////////////////
// Assert: OID is one of String classes!!
// Assert: Opts are CLASS_OPTION<...>!!
template<class OID, class... Opts>
class CLASS_CHOICE // CLASS
{
    template<unsigned I>
    using wrapped_option_type = choose_type<I,Opts...>;
public:
    static constexpr type_tag_t tag() { return OID::tag(); }
    static constexpr unsigned size() { return sizeof...(Opts); }
    using oid_type = OID;
    template<unsigned I>
    using option_type = typename wrapped_option_type<I>::value_type;
    template<unsigned I> option_type<I> &set_default();
private:
    static_assert(size() != 0,
        "CLASS_CHOICE must have at least one option");
    OID d; // discriminant
    int curr = -1;
    std::unique_ptr<impl::choice_option_base> p; // CLASS_OPTION

    static bool eq(const char *s1, const char *s2)
        { return std::strcmp(s1, s2) == 0; }

    template<unsigned I, class Func>
    auto choose_and_apply(const char *d, Func &&f) ->
        typename std::enable_if<(I == size()-1U),
        decltype(std::forward<Func>(f)(this->set_default<I>()))>::type
    {
        if(eq(d, wrapped_option_type<I>::id()))
            return std::forward<Func>(f)(set_default<I>());
        else throw invalid_choice_tag{};
    }
    template<unsigned I, class Func>
    auto choose_and_apply(const char *d, Func &&f) ->
        typename std::enable_if<(I < size()-1U),
        decltype(std::forward<Func>(f)(this->set_default<I>()))>::type
    {
        if(eq(d, wrapped_option_type<I>::id()))
            return std::forward<Func>(f)(set_default<I>());
        else return choose_and_apply<I+1>(d, std::forward<Func>(f));
    }
protected:
    using self_type = CLASS_CHOICE<OID,Opts...>;
public:
    CLASS_CHOICE() = default;
    CLASS_CHOICE(const CLASS_CHOICE &o)
        : d(o.d), curr(o.curr), p(o.is_set() ? o.p->clone() : nullptr) {}
    CLASS_CHOICE(CLASS_CHOICE &&o) noexcept
        : d(std::move(o.d)), curr(o.curr), p(std::move(o.p)) { o.curr = -1; }

    CLASS_CHOICE &operator=(const CLASS_CHOICE &o)
        { CLASS_CHOICE(o).swap(*this); return *this; }
    CLASS_CHOICE &operator=(CLASS_CHOICE &&o) noexcept
        { swap(o); return *this; }

    template<unsigned I> option_type<I> &get()
    {
        if(I != curr) throw invalid_choice_index{};
        return get_unchecked<I>();
    }
    template<unsigned I> const option_type<I> &get() const
    {
        if(I != curr) throw invalid_choice_index{};
        return get_unchecked<I>();
    }

    template<unsigned I> option_type<I> &get_unchecked()
    {
        assert(is_set());
        return static_cast<wrapped_option_type<I> &>(*p).unwrap();
    }
    template<unsigned I> const option_type<I> &get_unchecked() const
    {
        assert(is_set());
        return static_cast<const wrapped_option_type<I> &>(*p).unwrap();
    }

    // Applies generic function to current option
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply_unchecked(Func &&f)
    {
        return impl::choice_apply(*this, std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(const option_type<0> &)>::type
    apply_unchecked(Func &&f) const
    {
        return impl::choice_apply(*this, std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    apply(Func &&f)
    {
        validate();
        return apply_unchecked(std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(const option_type<0> &)>::type
    apply(Func &&f) const
    {
        validate();
        return apply_unchecked(std::forward<Func>(f));
    }

    // Sets new option and applies generic function to it
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    set_default_and_apply(const char *oid, Func &&f)
    {
        return choose_and_apply<0>(oid, std::forward<Func>(f));
    }
    template<class Func>
    typename std::result_of<Func(option_type<0> &)>::type
    set_default_and_apply(const std::string &oid, Func &&f)
    {
        return set_default_and_apply(oid.c_str(), std::forward<Func>(f));
    }

    const oid_type &oid() const { return d; }
    type_tag_t option_tag() const { assert(is_set()); return p->tag(); }
    void validate() const { if(!is_set()) throw choice_is_not_set{}; }

    int index() const { return curr; }
    bool is_set() const { return index() != -1; }
    void clear() { p.reset(); curr = -1; }
    void swap(CLASS_CHOICE &o) noexcept
    {
        d.swap(o.d);
        p.swap(o.p);
        std::swap(curr, o.curr);
    }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class OID, class... Opts>
template<unsigned I>
auto CLASS_CHOICE<OID,Opts...>::set_default() -> option_type<I> &
{
    if(static_cast<int>(I) == curr) return get_unchecked<I>();
    auto obj_ptr = new wrapped_option_type<I>;
    p.reset(obj_ptr);
    curr = I;
    d = wrapped_option_type<I>::id();
    return obj_ptr->unwrap();
}
//----------------------------------------------------------------------------

#undef FORWARD_BASE_OPS

}} // namespace

#endif // header guard
