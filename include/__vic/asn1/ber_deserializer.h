// ASN.1 BER deserializer
//
// Platform: ISO C++ 11
// $Id$

#ifndef __VIC_ASN1_BER_DESERIALIZER_H
#define __VIC_ASN1_BER_DESERIALIZER_H

#include<__vic/asn1/impl/basic_deserializer.h>
#include<type_traits>

namespace __vic { namespace asn1 { namespace ber {

//////////////////////////////////////////////////////////////////////////////
template<class StreamReader>
class deserializer : public ber::basic_deserializer<StreamReader>
{
    typedef ber::basic_deserializer<StreamReader> base;
    typedef primitive_constructed pc_t;
    friend ber::deserializer_base; // for deserializer_base::choose_option(), etc.

    using base::read_type;
    using base::read_length;
    using base::read_definite_length;
    using base::append_value_bytes;
    using base::read_type_or_eoc;
    using base::skip_eoc_tlv;
    using base::have_more_bytes;
    using base::throw_cannot_read;
    using base::check_type;
    using base::check_primitive;
    using base::check_constructed;
    using base::push_limit;
    using base::pop_limit;

    // Deserialize length'n'value
    using base::deserialize_lv;
    void deserialize_lv(OCTET_STRING &v, pc_t p_c)
        { v.clear(); deserialize_segments(v, p_c); }
    void deserialize_lv(CHARACTER_STRING &v, pc_t p_c)
        { v.clear(); deserialize_segments(v, p_c); }

    void deserialize_lv(BOOLEAN &v, pc_t p_c)
    {
        check_primitive(p_c);
        v = this->read_boolean_value() != 0x00;
    }

    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize_lv(IMPLICIT<Tag,T,Cls> &v, pc_t p_c)
        { deserialize_lv(v.unwrap(), p_c); }
    template<tag_number_t Tag, class T, tag_class_t Cls>
    void deserialize_lv(EXPLICIT<Tag,T,Cls> & , pc_t );

    template<class... Elems>
    void deserialize_lv(SEQUENCE<Elems...> &seq, pc_t p_c)
    {
        check_constructed(p_c);
        size_t len;
        if(read_length(len)) // definite
        {
            push_limit(len);
            this->deserialize_seq_def(*this, seq);
            pop_limit();
        }
        else // indefinite
            this->deserialize_seq_indef(*this, seq);
    }
    template<class T, template<class,class> class SeqCont>
    void deserialize_lv(SEQUENCE_OF<T,SeqCont> &seq, pc_t p_c)
    {
        check_constructed(p_c);
        seq.clear();
        size_t len;
        if(read_length(len)) // definite
        {
            push_limit(len);
            while(have_more_bytes()) deserialize(seq.push_default());
            pop_limit();
        }
        else // indefinite
        {
            type_field_t t;
            while(read_type_or_eoc(t))
                deserialize_lv_or_choice(seq.push_default(), t);
        }
    }
    template<class T>
    void deserialize_lv_or_choice(T &v, const type_field_t &t)
    {
        // the last argument is used only for
        // choosing an appropriate function
        deserialize_lv_or_choice_helper(v, t, &v);
    }
    template<class T>
    void deserialize_lv_or_choice_helper( // It isn't a CHOICE
        T &v, const type_field_t &t, void * )
    {
        this->check_and_deserialize(*this, v, t);
    }
    template<class T, class... Opts>
    void deserialize_lv_or_choice_helper( // It's a CHOICE
        T &ch, const type_field_t &t, CHOICE<Opts...> * )
    {
        this->deserialize_choice_lv(*this, ch, t);
    }

    template<class OID, class... Opts>
    void deserialize_lv(CLASS_CHOICE<OID,Opts...> &ch, pc_t pc)
        { this->deserialize_class_lv(*this, ch, pc); }

    template<class Str> void deserialize_segments(Str & , pc_t );
    template<class Str> void deserialize_segments_definite(Str & , size_t );
    template<class Str> void deserialize_segments_indefinite(Str & );

    void deserialize_str(CHARACTER_STRING & , type_tag_t );
public:
    template<class... Args>
    explicit deserializer(Args&&... args)
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
template<class Str>
void deserializer<SR>::deserialize_segments(Str &v, pc_t p_c)
{
    if(is_primitive(p_c)) // straightforward case - single segment
        append_value_bytes(v, read_definite_length());
    else // many segments
    {
        size_t len;
        if(read_length(len))
            deserialize_segments_definite(v, len);
        else
            deserialize_segments_indefinite(v);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<class Str>
void deserializer<SR>::deserialize_segments_definite(Str &v, size_t len_all)
{
    push_limit(len_all);
    // which tag must we use in case of IMPLICIT tag??
    while(have_more_bytes())
        deserialize_segments(v, read_type(v.tag()));
    pop_limit();
}
//----------------------------------------------------------------------------
template<class SR>
template<class Str>
void deserializer<SR>::deserialize_segments_indefinite(Str &v)
{
    type_field_t t;
    while(read_type_or_eoc(t))
    {
        check_type(t.tag(), v.tag()); // which tag if IMPLICIT??
        deserialize_segments(v, t.p_c());
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<tag_number_t Tag, class T, tag_class_t Cls>
void deserializer<SR>::deserialize_lv(EXPLICIT<Tag,T,Cls> &v, pc_t p_c)
{
    check_constructed(p_c);
    size_t len;
    if(read_length(len)) // definite
    {
        push_limit(len);
        deserialize(v.unwrap());
        pop_limit();
    }
    else // indefinite
    {
        deserialize(v.unwrap());
        skip_eoc_tlv();
    }
}
//----------------------------------------------------------------------------
template<class SR>
void deserializer<SR>::deserialize_str(CHARACTER_STRING &v, type_tag_t tag)
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
void deserializer<SR>::deserialize(OCTET_STRING &v)
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
void deserializer<SR>::deserialize(BOOLEAN &v)
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
void deserializer<SR>::deserialize(IMPLICIT<Tag,T,Cls> &v)
{
    try
    {
        deserialize_lv(v.unwrap(), read_type(v.tag()));
    }
    catch(const std::exception &ex)
    {
        throw_cannot_read(v.tag(), ex);
    }
}
//----------------------------------------------------------------------------
template<class SR>
template<tag_number_t Tag, class T, tag_class_t Cls>
void deserializer<SR>::deserialize(EXPLICIT<Tag,T,Cls> &v)
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
void deserializer<SR>::deserialize(SEQUENCE<Elems...> &v)
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
void deserializer<SR>::deserialize(SEQUENCE_OF<T,SeqCont> &v)
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
void deserializer<SR>::deserialize(CHOICE<Opts...> &ch)
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
void deserializer<SR>::deserialize(CLASS_CHOICE<OID,Opts...> &c)
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
