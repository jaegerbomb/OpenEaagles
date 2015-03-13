#include "openeaagles/basic/Stack.h"
#include <stdlib.h>

namespace Eaagles {
namespace Basic {

IMPLEMENT_EMPTY_SLOTTABLE_SUBCLASS(Stack,"Stack")


//------------------------------------------------------------------------------
// copyData(), deleteData() -- copy (delete) member data
//------------------------------------------------------------------------------
EMPTY_COPYDATA(Stack)
EMPTY_DELETEDATA(Stack)


//------------------------------------------------------------------------------
// serialize() -- print the value of this object to the output stream sout.
//------------------------------------------------------------------------------
std::ostream& Stack::serialize(std::ostream& sout, const int, const bool) const
{
    return sout;
}

} // End Basic namespace
} // End Eaagles namespace
