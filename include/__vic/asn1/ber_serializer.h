// ASN.1 BER serializer
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_BER_SERIALIZER_H
#define __VIC_ASN1_BER_SERIALIZER_H

#include<__vic/asn1/impl/ber_encoded_length.h>
#include<__vic/asn1/impl/basic_serializer.h>
#include<__vic/asn1/impl/der_pc_traits.h>
#include<type_traits>

namespace __vic { namespace ASN1 {

//////////////////////////////////////////////////////////////////////////////
template<class StreamWriter>
class BERSerializer : private BasicSerializer<StreamWriter>
{
    typedef BasicSerializer<StreamWriter> base;

    template<class T>
    static size_t value_length(const T &v)
        { return impl::ber_encoded_length(v); }

    using base::write_type;
    using base::write_primitive_type;
    using base::write_constructed_type;
    using base::write_constructed_type_short;
    using base::write_length;
    using base::write_indefined_length;
    using base::write_eoc_tlv;

    using base::serialize_value;
    template<tag_number_t Tag, class T, tag_class_t Cls>
    void serialize_value(const IMPLICIT<Tag,T,Cls> &v)
        { serialize_value(v.unwrap()); }
    template<tag_number_t Tag, class T, tag_class_t Cls>
    void serialize_value(const EXPLICIT<Tag,T,Cls> &v)
        { serialize(v.unwrap()); }
    template<class... Elems>
    void serialize_value(const SEQUENCE<Elems...> &seq)
        { this->serialize_seq_value(*this, seq); }
    template<class T, template<class,class> class SeqCont>
    void serialize_value(const SEQUENCE_OF<T,SeqCont> &seq)
        { for(const auto &el : seq) serialize(el); }

    template<class T>
    void serialize_constructed_lv(const T & );

    bool m_use_definite = false;
public:
    template<class... Args>
    explicit BERSerializer(Args&&... args)
        : base(std::forward<Args>(args)...) {}

    using typename base::stream_writer_type;
    using base::get_stream_writer;

    using base::serialize;

    template<tag_number_t Tag, class T, tag_class_t Cls>
    typename std::enable_if<impl::is_der_primitive<T>()>::type
        serialize(const IMPLICIT<Tag,T,Cls> & );

    template<tag_number_t Tag, class T, tag_class_t Cls>
    typename std::enable_if<impl::is_der_constructed<T>()>::type
        serialize(const IMPLICIT<Tag,T,Cls> & );

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void serialize(const EXPLICIT<Tag,T,Cls> & );

    template<class... Elems>
    void serialize(const SEQUENCE<Elems...> & );

    template<class T, template<class,class> class SeqCont>
    void serialize(const SEQUENCE_OF<T,SeqCont> & );

    template<class... Opts>
    void serialize(const CHOICE<Opts...> & );

    template<class OID, class... Opts>
    void serialize(const CLASS_CHOICE<OID,Opts...> & );

    void use_definite(bool yes = true) { m_use_definite = yes; }
    void use_indefinite(bool yes = true) { use_definite(!yes); }
    bool is_definite_used() const { return m_use_definite; }
    bool is_indefinite_used() const { return !is_definite_used(); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class SW>
template<class T>
inline void BERSerializer<SW>::serialize_constructed_lv(const T &v)
{
    if(m_use_definite)
    {
        write_length(value_length(v));
        serialize_value(v);
    }
    else
    {
        write_indefined_length();
        serialize_value(v);
        write_eoc_tlv();
    }
}
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
typename std::enable_if<impl::is_der_primitive<T>()>::type
    BERSerializer<SW>::serialize(const IMPLICIT<Tag,T,Cls> &v)
{
    write_primitive_type(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
typename std::enable_if<impl::is_der_constructed<T>()>::type
    BERSerializer<SW>::serialize(const IMPLICIT<Tag,T,Cls> &v)
{
    write_constructed_type(v.tag());
    serialize_constructed_lv(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
void BERSerializer<SW>::serialize(const EXPLICIT<Tag,T,Cls> &v)
{
    write_constructed_type(v.tag());
    serialize_constructed_lv(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Elems>
void BERSerializer<SW>::serialize(const SEQUENCE<Elems...> &v)
{
    write_constructed_type_short(v.tag());
    serialize_constructed_lv(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class T, template<class,class> class SeqCont>
void BERSerializer<SW>::serialize(const SEQUENCE_OF<T,SeqCont> &v)
{
    write_constructed_type_short(v.tag());
    serialize_constructed_lv(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Opts>
void BERSerializer<SW>::serialize(const CHOICE<Opts...> &ch)
{
    this->serialize_choice(*this, ch);
}
//----------------------------------------------------------------------------
template<class SW>
template<class OID, class... Opts>
void BERSerializer<SW>::serialize(const CLASS_CHOICE<OID,Opts...> &c)
{
    this->serialize_class(*this, c);
}
//----------------------------------------------------------------------------

}} // namespace

#endif // header guard
