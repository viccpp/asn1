// ASN.1 DER serializer
//
// Platform: ISO C++ 11
// $Id: der_serializer.h 1854 2015-09-30 10:48:44Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_DER_SERIALIZER_H
#define __MFISOFT_JANUARY_ASN1_DER_SERIALIZER_H

#include<mfisoft/january/asn1/impl/der_encoded_length.h>
#include<mfisoft/january/asn1/impl/basic_serializer.h>
#include<mfisoft/january/asn1/impl/der_pc_traits.h>

namespace mfisoft { namespace january { namespace ASN1 {

//////////////////////////////////////////////////////////////////////////////
template<class StreamWriter>
class DERSerializer : private BasicSerializer<StreamWriter>
{
    typedef BasicSerializer<StreamWriter> base;

    template<class T>
    static size_t value_length(const T &v)
        { return impl::der_encoded_length(v); }

    using base::write_type;
    using base::write_constructed_type;
    using base::write_constructed_type_short;
    using base::write_length;

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
public:
    template<class... Args>
    explicit DERSerializer(Args&&... args)
        : base(std::forward<Args>(args)...) {}

    using typename base::stream_writer_type;
    using base::get_stream_writer;

    using base::serialize;

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void serialize(const IMPLICIT<Tag,T,Cls> & );

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
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
void DERSerializer<SW>::serialize(const IMPLICIT<Tag,T,Cls> &v)
{
    write_type(v.tag(), impl::der_constructness<T>());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
void DERSerializer<SW>::serialize(const EXPLICIT<Tag,T,Cls> &v)
{
    write_constructed_type(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Elems>
void DERSerializer<SW>::serialize(const SEQUENCE<Elems...> &v)
{
    write_constructed_type_short(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class T, template<class,class> class SeqCont>
void DERSerializer<SW>::serialize(const SEQUENCE_OF<T,SeqCont> &v)
{
    write_constructed_type_short(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Opts>
void DERSerializer<SW>::serialize(const CHOICE<Opts...> &ch)
{
    this->serialize_choice(*this, ch);
}
//----------------------------------------------------------------------------
template<class SW>
template<class OID, class... Opts>
void DERSerializer<SW>::serialize(const CLASS_CHOICE<OID,Opts...> &c)
{
    this->serialize_class(*this, c);
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
