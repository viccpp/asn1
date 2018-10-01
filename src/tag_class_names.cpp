//
// $Id: tag_class_names.cpp 581 2012-09-21 11:24:54Z vdyachenko $
//

#include<mfisoft/january/asn1/ber.h>

namespace mfisoft { namespace january { namespace ASN1 { namespace BER {

const char * const impl::tag_class_names[4] =
    { "UNIVERSAL", "APPLICATION", "CONTEXT-SPECIFIC", "PRIVATE" };

}}}} // namespace
