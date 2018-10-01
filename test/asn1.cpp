#include<__vic/asn1/ber.h>
#include<__vic/asn1/ber_coder.h>
#include<__vic/asn1/ber_decoder.h>
#if __cplusplus >= 201103L
#include<__vic/asn1/types.h>
#include<__vic/asn1/ber_serializer.h>
#include<__vic/asn1/der_serializer.h>
#include<__vic/asn1/ber_deserializer.h>
#include<__vic/asn1/der_deserializer.h>
#endif
#include<__vic/fs.h>
#include<iostream>
#include<fstream>
#include<cassert>

namespace ASN1 = __vic::ASN1;
using namespace ASN1;

//////////////////////////////////////////////////////////////////////////////
class ostream_writer
{
    std::ostream *os;
public:
    explicit ostream_writer(std::ostream &s) : os(&s)
        { os->exceptions(std::ios::badbit); }
    // only to avoid compilation errors on explicit instantiation
    ostream_writer() : os(nullptr) {}

    void write(uint8_t byte) { os->put(byte); }
    void write(const void *bytes, std::size_t n)
        { os->write(reinterpret_cast<const char *>(bytes), n); }
};
//////////////////////////////////////////////////////////////////////////////
class istream_reader
{
    std::istream *is;
public:
    explicit istream_reader(std::istream &s) : is(&s)
        { is->exceptions(std::ios::badbit); }
    // only to avoid compilation errors on explicit instantiation
    istream_reader() : is(nullptr) {}

    bool read(uint8_t &byte)
        { return static_cast<bool>(is->get(reinterpret_cast<char&>(byte))); }
    size_t read_max(void *bytes, size_t n)
    {
        if(!*is) return 0;
        is->read(static_cast<char*>(bytes), n);
        return is->gcount();
    }
    size_t skip_max(size_t n)
    {
        if(!*is) return 0;
        is->ignore(n);
        return is->gcount();
    }
};
//////////////////////////////////////////////////////////////////////////////

template class BER::Coder<ostream_writer>;
template class BER::Decoder<istream_reader>;

//----------------------------------------------------------------------------
void encoding_test(const char *fname = "asn1.ber")
{
    std::ofstream file(fname, std::ios::binary);
    assert(file.is_open());
    BER::Coder<ostream_writer> out((ostream_writer(file)));
    out.write_type(BER::universal, 2, BER::primitive); // INTEGER
    out.write_integer_with_length(666);
}
//----------------------------------------------------------------------------
void decoding_test(const char *fname = "asn1.ber")
{
    std::ifstream file(fname, std::ios::binary);
    assert(file.is_open());
    BER::Decoder<istream_reader> in((istream_reader(file)));

    BER::type_field_t t;
    assert(in.read_type(t));
    assert(t.tag_class() == BER::universal && t.tag_number() == 2);
    assert(t.p_c() == BER::primitive);

    std::size_t len;
    assert(in.read_length(len));

    assert(in.read_integer<int>(len) == 666);
}
//----------------------------------------------------------------------------

#if __cplusplus >= 201103L
//----------------------------------------------------------------------------
template<class T>
void serializeBER(const T &v, const char *fname = "res.ber")
{
    std::ofstream file(fname, std::ios::binary);
    assert(file.is_open());
    ASN1::BERSerializer<ostream_writer> s{ostream_writer{file}};
    //s.use_definite(true);
    s.serialize(v);
}
//----------------------------------------------------------------------------
template<class T>
void serializeDER(const T &v, const char *fname = "res.der")
{
    std::ofstream file(fname, std::ios::binary);
    assert(file.is_open());
    ASN1::DERSerializer<ostream_writer> s{ostream_writer{file}};
    s.serialize(v);
}
//----------------------------------------------------------------------------
template<class T>
void deserializeBER(T &v, const char *fname = "res.ber")
{
    std::ifstream file(fname, std::ios::binary);
    assert(file.is_open());
    ASN1::BERDeserializer<istream_reader> s{istream_reader{file}};
    s.deserialize(v);
}
//----------------------------------------------------------------------------
template<class T>
void deserializeDER(T &v, const char *fname = "res.der")
{
    std::ifstream file(fname, std::ios::binary);
    assert(file.is_open());
    ASN1::DERDeserializer<istream_reader> s{istream_reader{file}};
    s.deserialize(v);
}
//----------------------------------------------------------------------------

enum Status { stOk = 1, stError = 2 };

namespace __vic { namespace ASN1 {
//---------------------------------------------------------------------------
// Validator for a deserializer
template<>
inline bool is_enum_value<Status>(std::underlying_type<Status>::type int_val)
{
    return int_val == stOk || int_val == stError;
}
//---------------------------------------------------------------------------
}} // namespace

extern const char oid1[] = "oid1", oid2[] = "oid2", oid3[] = "oid3";

//////////////////////////////////////////////////////////////////////////////
struct ChoiceType :
    CHOICE<
        IMPLICIT<0, CHARACTER_STRING>,
        EXPLICIT<1, OCTET_STRING>,
        EXPLICIT<2, INTEGER<>>,
        EXPLICIT<3, REAL<raw>>
    >
{
    CHOICE_FIELD(0, string)
    CHOICE_FIELD(1, bytes)
    CHOICE_FIELD(2, integer)
    CHOICE_FIELD(3, real)
};
//////////////////////////////////////////////////////////////////////////////
struct ClassSeqType :
    SEQUENCE<
        INTEGER<>
    >
{
    ClassSeqType() { integer() = 0; }
    SEQ_FIELD(0, integer)
};
//////////////////////////////////////////////////////////////////////////////
struct ClassType :
    CLASS_CHOICE<ObjectDescriptor,
        CLASS_OPTION<oid1, CHARACTER_STRING>,
        CLASS_OPTION<oid2, IMPLICIT<0, ClassSeqType>>,
        CLASS_OPTION<oid3, integer<raw>>
    >
{
    CHOICE_FIELD(0, string)
    CHOICE_FIELD(1, seq)
    CHOICE_FIELD(2, integer)
};
//////////////////////////////////////////////////////////////////////////////
struct StructType :
    SEQUENCE<
        NumericString,
        OPTIONAL<PrintableString>,
        INTEGER<int>,
        IMPLICIT<0, INTEGER<int>>,
        EXPLICIT<1, INTEGER<int>>,
        OPTIONAL<EXPLICIT<2, PrintableString>>,
        SEQUENCE_OF<PrintableString>,
        ENUMERATED<Status>,
        ChoiceType,
        INTEGER<int, -10, 10>,
        SIZE<PrintableString, 2, 10>,
        OPTIONAL<INTEGER<>>,
        ClassType,
        OPTIONAL<PrintableString>
    >
{
    SEQ_FIELD(0, num_string)
    SEQ_FIELD(1, print_string)
    SEQ_FIELD(2, number)
    SEQ_FIELD(3, number_impl)
    SEQ_FIELD(4, number_expl)
    SEQ_FIELD(5, print_string_opt)
    SEQ_FIELD(6, seq_of)
    SEQ_FIELD(7, status)
    SEQ_FIELD(8, choice)
    SEQ_FIELD(9, constrained_int)
    SEQ_FIELD(10, constrained_str)
    SEQ_FIELD(11, opt)
    SEQ_FIELD(12, class_)
    SEQ_FIELD(13, opt2)
};
//////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
void assign_trunc_test()
{
    static const char test_str[] = "0123456789";
    SIZE<PrintableString, 0, 8> s;
    s.assign_trunc(test_str);
    std::cout << s << '\n';
    assert(s == "01234567");

    s.clear();
    assert(s.empty());
    s.assign_trunc(std::string{test_str});
    std::cout << s << '\n';
    assert(s == "01234567");

    s.assign(test_str);
    std::cout << s << '\n';
    assert(s == test_str);

    SIZE<UTF8String, 0, 3> utf8;
    utf8.assign_trunc("\xD0\x90\xD0\x91\xD0\x92\xD0\x93"); // 4 cyr letters
    assert(utf8.length_chars() == 3);
    assert(utf8.length() == 6);
}
//---------------------------------------------------------------------------
void set_default_and_apply_choice_test()
{
    ChoiceType ch;
    struct generic_lambda
    {
        void operator()(CHARACTER_STRING &s) const { s = "abc"; }
        void operator()(OCTET_STRING &s) const { s.assign("A", 1); }
        void operator()(INTEGER<> &n) const { n = 321; }
        void operator()(REAL<raw> &n) const { n.assign("12.3", 4); }
    };
    ch.set_default_and_apply(0, generic_lambda{});
    serializeBER(ch);
}
//---------------------------------------------------------------------------
void set_default_and_apply_class_test()
{
    ClassType c;
    struct generic_lambda
    {
        void operator()(CHARACTER_STRING &s) const { s = "class"; }
        void operator()(ClassSeqType &s) const { /* Do nothing */ }
        void operator()(integer<raw> &n) const { n.assign("\x01", 1); }
    };
    c.set_default_and_apply(oid1, generic_lambda{});
    serializeBER(c);
}
//---------------------------------------------------------------------------
struct OptionalChoice :
    SEQUENCE<
        OPTIONAL<ChoiceType>,
        INTEGER<>,
        OPTIONAL<ChoiceType>
    >
{
};
void optional_choice_test()
{
    OptionalChoice s;
    s.get<0>().set_default().set_integer() = 1;
    serializeBER(s);
    deserializeBER(s);
}
//---------------------------------------------------------------------------
struct Print : private __vic::non_copyable
{
    template<class T>
    void operator()(const T &v) const { std::cout << v << ' '; }
};
void for_each_test()
{
    struct Seq :
        SEQUENCE<
            INTEGER<>,
            BOOLEAN,
            PrintableString
        >
    {} s;

    s.get<0>() = 2;
    s.get<1>() = true;
    s.get<2>() = "abc";

    std::cout << std::boolalpha;
    const_cast<const Seq &>(s).for_each(Print{});
    std::cout << '\n';

    struct Modify : private __vic::non_copyable
    {
        void operator()(INTEGER<> &v) const { v = v + 1; }
        void operator()(BOOLEAN &v) const { v = !v; }
        void operator()(PrintableString &v) const { v += 'z'; }
    };
    s.for_each(Modify{});

    s.for_each(Print{});
    std::cout << '\n';
}
//---------------------------------------------------------------------------
void asn1_types_test()
{
    StructType seq;
    seq.num_string() = "123";
    seq.print_string() = "abc";
    seq.number() = -666;
    seq.number_impl() = seq.number();
    seq.number_expl() = seq.number();
    seq.print_string_opt() = "xyz";
    seq.seq_of() = { "seq1", "seq2", "seq3" };
    seq.status() = stOk;

    seq.constrained_int() = -20;
    if(!seq.constrained_int().is_valid())
        std::cout << "Constraint violation " <<
            seq.constrained_int().as_int() << '\n';
    seq.constrained_int() = 1;
    if(!seq.constrained_int().is_valid())
        std::cout << "Invalid " << seq.constrained_int().as_int() << '\n';

    seq.constrained_str() = "qqq";
    if(!seq.constrained_str().is_valid())
        std::cout << "Invalid constrained_str\n";

    seq.choice().set_string() = "choice-option";
    std::cout << seq.choice().string() << '\n';
    try {
        OCTET_STRING _ = seq.choice().bytes();
    } catch(const invalid_choice_index &ex) {
        // OK, ignore
        //std::clog << "Element 1: " << ex.what() << '\n';
    }
    seq.choice().set_integer() = 555;

    seq.opt() = 0;
    seq.class_().set_default<0>() = "class.str";

    serializeDER(seq);
    serializeBER(seq);

    StructType seq1;
    deserializeDER(seq1);
    serializeDER(seq1, "res1.der");

    StructType seq2;
    deserializeBER(seq2);
    serializeBER(seq2, "res1.ber");
}
//---------------------------------------------------------------------------
#endif // C++11

int main()
{
    try
    {
        std::cout << BER::to_text(BER::type_tag_t(2, BER::universal)) << std::endl;
        std::cout << BER::to_text(BER::type_tag_t(0, BER::context_specific)) << std::endl;
        std::cout << BER::to_text(BER::type_tag_t(3, BER::application)) << std::endl;

        encoding_test();
        decoding_test();
        __vic::remove_file_if_exists("asn1.ber");

#if __cplusplus >= 201103L
        asn1_types_test();
        assign_trunc_test();
        optional_choice_test();
        for_each_test();
#endif

        return 0;
    }
    catch(const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
    }
    return 1;
}
