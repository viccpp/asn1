// ASN.1 DER deserializer
//
// Platform: ISO C++ 11
// $Id: der_deserializer.h 1944 2015-12-22 09:53:20Z vdyachenko $

#ifndef __MFISOFT_JANUARY_ASN1_DER_DESERIALIZER_H
#define __MFISOFT_JANUARY_ASN1_DER_DESERIALIZER_H

#include<mfisoft/january/asn1/impl/basic_deserializer.h>
#include<type_traits>

namespace mfisoft { namespace january { namespace ASN1 {

//////////////////////////////////////////////////////////////////////////////
template<class StreamReader>
class DERDeserializer : public BasicDeserializer<StreamReader>
{
    typedef BasicDeserializer<StreamReader> base;
    typedef BER::primitive_constructed pc_t;
    friend DeserializerBase; // for DeserializerBase::choose_option(), etc.

    using base::read_type;
    using base::read_definite_length;
    using base::have_more_bytes;
    using base::throw_cannot_read;
    using base::check_primitive;
    using base::check_constructed;
    using base::push_limit;
    using base::pop_limit;
    using base::deserialize_raw_lv;

    // Deserialize length'n'value
    using base::deserialize_lv;
    void deserialize_lv(OCTET_STRING &v, pc_t p_c)
        { deserialize_raw_lv(v.as_raw(), p_c); }
    void deserialize_lv(CHARACTER_STRING &v, pc_t p_c)
        { deserialize_raw_lv(v, p_c); }
    void deserialize_lv(BOOLEAN & , pc_t );

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize_lv(IMPLICIT<Tag,T,Cls> &v, pc_t p_c)
        { deserialize_lv(v.unwrap(), p_c); }
    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize_lv(EXPLICIT<Tag,T,Cls> &v, pc_t p_c)
    {
        check_constructed(p_c);
        push_limit(read_definite_length());
        deserialize(v.unwrap());
        pop_limit();
        //!! deserialize_constructed([&]{ deserialize(v.get()); });
    }

    template<class... Elems>
    void deserialize_lv(SEQUENCE<Elems...> &seq, pc_t p_c)
    {
        check_constructed(p_c);
        push_limit(read_definite_length());
        this->deserialize_seq_def(*this, seq);
        pop_limit();
    }
    template<class T, template<class,class> class SeqCont>
    void deserialize_lv(SEQUENCE_OF<T,SeqCont> &seq, pc_t p_c)
    {
        check_constructed(p_c);
        push_limit(read_definite_length());
        seq.clear();
        while(have_more_bytes()) deserialize(seq.push_default());
        pop_limit();
    }

    template<class OID, class... Opts>
    void deserialize_lv(CLASS_CHOICE<OID,Opts...> &ch, pc_t pc)
        { this->deserialize_class_lv(*this, ch, pc); }

    void deserialize_str(CHARACTER_STRING & , type_tag_t );
public:
    typedef typename base::bad_format bad_format;

    template<class... Args>
    explicit DERDeserializer(Args&&... args)
        : base(std::forward<Args>(args)...) {}

    using typename base::stream_reader_type;
    using base::get_stream_reader;

    using base::deserialize;

    void deserialize(OCTET_STRING & );
    void deserialize(CHARACTER_STRING &v) { deserialize_str(v, v.tag()); }
    void deserialize(PrintableString &v) { deserialize_str(v, v.tag()); }
    void deserialize(GraphicString &v) { deserialize_str(v, v.tag()); }
    void deserialize(VisibleString &v) { deserialize_str(v, v.tag()); }
    void deserialize(NumericString &v) { deserialize_str(v, v.tag()); }
    void deserialize(UTF8String &v) { deserialize_str(v, v.tag()); }
    void deserialize(ObjectDescriptor &v) { deserialize_str(v, v.tag()); }
    void deserialize(UTCTime &v) { deserialize_str(v, v.tag()); }
    void deserialize(GeneralizedTime &v) { deserialize_str(v, v.tag()); }

    template<class T, size_t L, size_t H>
    void deserialize(SIZE<T,L,H> &v) { deserialize(v.unwrap()); }

    void deserialize(BOOLEAN & );

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize(IMPLICIT<Tag,T,Cls> & );

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize(EXPLICIT<Tag,T,Cls> & );

    template<class... Elems>
    void deserialize(SEQUENCE<Elems...> & );

    template<class T, template<class,class> class SeqCont>
    void deserialize(SEQUENCE_OF<T,SeqCont> & );

    template<class... Opts>
    void deserialize(CHOICE<Opts...> & );

    template<class OID, class... Opts>
    void deserialize(CLASS_CHOICE<OID,Opts...> & );
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class SR>
void DERDeserializer<SR>::deserialize_lv(BOOLEAN &v, pc_t p_c)
{
    check_primitive(p_c);
    uint8_t b = this->read_boolean_value();
    switch(b)
    {
        case 0x00: v = false; break;
        case 0xFF: v = true;  break;
        default: throw bad_format(jan::msg(32) << "Invalid value: 0x" <<
            jan::hi_nibble_to_hex(b) << jan::lo_nibble_to_hex(b));
    }
}
//----------------------------------------------------------------------------
template<class SR>
void DERDeserializer<SR>::deserialize_str(CHARACTER_STRING &v, type_tag_t tag)
{
    try
    {
        deserialize_lv(v, read_type(tag));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(tag, ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
void DERDeserializer<SR>::deserialize(OCTET_STRING &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
void DERDeserializer<SR>::deserialize(BOOLEAN &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<tag_number_t Tag, class T, tag_class_t Cls>
void DERDeserializer<SR>::deserialize(IMPLICIT<Tag,T,Cls> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<tag_number_t Tag, class T, tag_class_t Cls>
void DERDeserializer<SR>::deserialize(EXPLICIT<Tag,T,Cls> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class... Elems>
void DERDeserializer<SR>::deserialize(SEQUENCE<Elems...> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read("SEQUENCE", ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class T, template<class,class> class SeqCont>
void DERDeserializer<SR>::deserialize(SEQUENCE_OF<T,SeqCont> &v)
{
    try
    {
        deserialize_lv(v, read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read("SEQUENCE OF", ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class... Opts>
void DERDeserializer<SR>::deserialize(CHOICE<Opts...> &ch)
{
    try
    {
        this->deserialize_choice_lv(*this, ch, read_type());
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read("CHOICE", ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class OID, class... Opts>
void DERDeserializer<SR>::deserialize(CLASS_CHOICE<OID,Opts...> &c)
{
    try
    {
        this->deserialize_class(*this, c);
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read("CLASS", ex);
    }
}
//----------------------------------------------------------------------------

}}} // namespace

#endif // header guard
