//------------------------------------------------------------------------------
// Float
//------------------------------------------------------------------------------
#include "openeaagles/basic/Float.h"

namespace Eaagles {
namespace Basic {

IMPLEMENT_EMPTY_SLOTTABLE_SUBCLASS(Float,"float")


//------------------------------------------------------------------------------
// copyData(), deleteData() -- copy (delete) member data
//------------------------------------------------------------------------------
EMPTY_COPYDATA(Float)
EMPTY_DELETEDATA(Float)


//------------------------------------------------------------------------------
// serialize() -- print the value of this object to the output stream sout.
//------------------------------------------------------------------------------
std::ostream& Float::serialize(std::ostream& sout, const int, const bool) const
{
   sout << val;
   return sout;
}

} // End Basic namespace
} // End Eaagles namespace
