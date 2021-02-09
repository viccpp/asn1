// ASN.1 DER serializer
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_DER_SERIALIZER_H
#define __VIC_ASN1_DER_SERIALIZER_H

#include<__vic/asn1/ber/impl/basic_serializer.h>
#include<__vic/asn1/der/impl/encoded_length.h>
#include<__vic/asn1/der/impl/pc_traits.h>

namespace __vic { namespace asn1 { namespace der {

//////////////////////////////////////////////////////////////////////////////
template<class StreamWriter>
class serializer : private ber::basic_serializer<StreamWriter>
{
    typedef ber::basic_serializer<StreamWriter> base;

    template<class T>
    static size_t value_length(const T &v) { return der::encoded_length(v); }

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
    explicit serializer(Args&&... args)
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
void serializer<SW>::serialize(const IMPLICIT<Tag,T,Cls> &v)
{
    write_type(v.tag(), der::constructness<T>());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<tag_number_t Tag, class T, tag_class_t Cls>
void serializer<SW>::serialize(const EXPLICIT<Tag,T,Cls> &v)
{
    write_constructed_type(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Elems>
void serializer<SW>::serialize(const SEQUENCE<Elems...> &v)
{
    write_constructed_type_short(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class T, template<class,class> class SeqCont>
void serializer<SW>::serialize(const SEQUENCE_OF<T,SeqCont> &v)
{
    write_constructed_type_short(v.tag());
    write_length(value_length(v));
    serialize_value(v);
}
//----------------------------------------------------------------------------
template<class SW>
template<class... Opts>
void serializer<SW>::serialize(const CHOICE<Opts...> &ch)
{
    this->serialize_choice(*this, ch);
}
//----------------------------------------------------------------------------
template<class SW>
template<class OID, class... Opts>
void serializer<SW>::serialize(const CLASS_CHOICE<OID,Opts...> &c)
{
    this->serialize_class(*this, c);
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
