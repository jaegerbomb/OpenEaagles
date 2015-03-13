#include "openeaagles/simulation/Ntm.h"

namespace Eaagles {
namespace Simulation {

//==============================================================================
// Ntm class
//==============================================================================
IMPLEMENT_ABSTRACT_SUBCLASS(Ntm,"Ntm")

//------------------------------------------------------------------------------
// slot table for this class type
//------------------------------------------------------------------------------
BEGIN_SLOTTABLE(Ntm)
   "template",       // 1) Template player
END_SLOTTABLE(Ntm)

// Map slot table to handles 
BEGIN_SLOT_MAP(Ntm)
    ON_SLOT(1, setSlotTemplatePlayer, Player)
END_SLOT_MAP()

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
Ntm::Ntm()
{
   STANDARD_CONSTRUCTOR()

   tPlayer = 0;
}

//------------------------------------------------------------------------------
// copyData() -- copy member data
//------------------------------------------------------------------------------
void Ntm::copyData(const Ntm& org, const bool cc)
{
   BaseClass::copyData(org);

   if (cc) {
      tPlayer = 0;
   }

   setSlotTemplatePlayer( org.tPlayer );
}

//------------------------------------------------------------------------------
// deleteData() -- delete member data
//------------------------------------------------------------------------------
void Ntm::deleteData()
{
   setSlotTemplatePlayer( 0 );
}

//------------------------------------------------------------------------------
// Set Slot functions
//------------------------------------------------------------------------------

// Sets the template player
bool Ntm::setSlotTemplatePlayer(const Player* const msg)
{
   tPlayer = msg;
   return true;
}

//------------------------------------------------------------------------------
// getSlotByIndex()
//------------------------------------------------------------------------------
Basic::Object* Ntm::getSlotByIndex(const int si)
{
    return BaseClass::getSlotByIndex(si);
         }

//------------------------------------------------------------------------------
// serialize
//------------------------------------------------------------------------------
std::ostream& Ntm::serialize(std::ostream& sout, const int i, const bool slotsOnly) const
{
   int j = 0;
   if ( !slotsOnly ) {
      indent(sout,i);
      sout << "( " << getFactoryName() << std::endl;
      j = 4;
      }

   if (tPlayer != 0) {
      indent(sout,i+j);
      //sout << "template:" << std::endl;
      //tPlayer->serialize(sout,(i+j+4));
      sout << "template: ( " << tPlayer->getFactoryName();
      const Basic::String* tt = tPlayer->getType();
      if (tt != 0) {
         sout << " type: " << *tt;
   }
      sout << " )";
      sout << std::endl;
}

   BaseClass::serialize(sout,i+j,true);

   if ( !slotsOnly ) {
      indent(sout,i);
      sout << ")" << std::endl;
   }

   return sout;
}

} // End Simulation namespace
} // End Eaagles namespace
