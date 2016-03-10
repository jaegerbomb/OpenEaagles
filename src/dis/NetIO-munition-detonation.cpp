//------------------------------------------------------------------------------
// Class: NetIO
// Description: Portions of class defined to support munition detonation PDUs
//------------------------------------------------------------------------------

#include "openeaagles/dis/NetIO.h"
#include "openeaagles/dis/Nib.h"
#include "openeaagles/dis/pdu.h"

#include "openeaagles/simulation/AirVehicle.h"
#include "openeaagles/simulation/Player.h"
#include "openeaagles/simulation/Simulation.h"
#include "openeaagles/simulation/Weapon.h"
#include "openeaagles/basic/Nav.h"
#include "openeaagles/basic/NetHandler.h"
#include "openeaagles/basic/Pair.h"
#include "openeaagles/basic/PairStream.h"

#include "openeaagles\simulation\Station.h"
#include "openeaagles\simulation\Simulation.h"
#include "openeaagles\simulation\Missile.h"

#include "openeaagles\dis\Ntm.h"
#include "openeaagles\dis\structs.h"
#include "openeaagles\simulation\Weapon.h"

namespace Eaagles {
namespace Network {
namespace Dis {

//------------------------------------------------------------------------------
// processDetonationPDU() callback --
//------------------------------------------------------------------------------
void NetIO::processDetonationPDU(const DetonationPDU* const pdu)
{
   // check everything manually
   checkDetonationManually(pdu);
}

void NetIO::checkDetonationManually(const DetonationPDU* const pdu) const
{  
   if (pdu == 0) return;
   osg::Vec3d worldPos(pdu->location.X_coord, pdu->location.Y_coord, pdu->location.Z_coord);
   const Simulation::Station* stn = getStation();
   if (stn != 0) {
      const Simulation::Simulation* sim = stn->getSimulation();
      if (sim != 0) {
         Simulation::Weapon* cWpn = 0;
         // We are going to kluge this for speed... we need to tell the player that this is a 
         // manual detonation with no weapon attached - but it always has a weapon to look at
         // (in processDetonation).  So, we will make the dummy flag true to tell the derived
         // player that we are indeed a made up weapon, and the weapon's position will be in world
         // coordinates instead of relative position.
         // default weapon parameters (not very deadly)
         LCreal maxBurstRange = 25.0f;
         LCreal maxLethalRange = 0.5;
         // find the missile based on the enumerated type that came in!
         const Dis::Ntm* ntm = findNtmByTypeCodes(pdu->burst.munition.kind,
            pdu->burst.munition.domain, pdu->burst.munition.country, 
            pdu->burst.munition.category, pdu->burst.munition.subcategory, 
            pdu->burst.munition.specific, pdu->burst.munition.extra);

         std::cout << "NetIO::checkDetonationManually - weapon parameters:" << std::endl;
         std::cout << "<Kind, Domain, Country, Category, SubCategory, Specific, Extra>:" << std::endl;
         std::cout << static_cast<unsigned>(pdu->burst.munition.kind) << ", " << static_cast<unsigned>(pdu->burst.munition.domain) << ", " 
            << pdu->burst.munition.country << ", " << static_cast<unsigned>(pdu->burst.munition.category) << ", " 
            << static_cast<unsigned>(pdu->burst.munition.subcategory) << ", " << static_cast<unsigned>(pdu->burst.munition.specific) << ", " 
            << static_cast<unsigned>(pdu->burst.munition.extra) << std::endl;
         if (ntm != 0) {
            Simulation::Player* ply = (Simulation::Player*)ntm->getTemplatePlayer();
            if (ply != 0) {
               cWpn = dynamic_cast<Simulation::Weapon*>(ply);
               if (cWpn != 0) {
                  cWpn->ref();
                  maxBurstRange = cWpn->getMaxBurstRng();
                  cWpn->setDummy(true);
                  cWpn->container((Basic::Component*)sim);
                  cWpn->setGeocPosition(worldPos, true);
                  std::cout << "Weapon type = " << *cWpn->getType() << std::endl;
                  std::cout << "Max Burst Range = " << maxBurstRange << std::endl;
                  std::cout << "Lethal Range = " << cWpn->getLethalRange() << std::endl;
               }
               // not a weapon, but instead a player - just make up numbers!
               else {
                  std::cout << "Detontation is not from a weapon, default weapon parameters" << std::endl;
               }
            }
         }
         // no incoming weapon was found - print out 
         else {
            std::cout << "No weapon of this type found - default weapon parameters!" << std::endl;
         }

         // extend burst range out far enough
         LCreal maxRng = 10.0f * maxBurstRange;
         const Basic::PairStream* plist = sim->getPlayers();
         if (plist != 0) {
            const Basic::List::Item* item = plist->getFirstItem();
            // Process the detonation for all local, in-range players
            bool finished = false;
            while (item != 0 && !finished) {
               const Basic::Pair* pair = static_cast<const Basic::Pair*>(item->getValue());
               Simulation::Player* p = (Simulation::Player*)pair->object(); 
               finished = p->isNetworkedPlayer();  // local only
               if (!finished) {
                  p->preProcessDetonation(worldPos);
                  const osg::Vec3d dPos = p->getGeocPosition() - worldPos;
                  LCreal rng = dPos.length();
                  if ( (rng <= maxRng) ) {
                     // default, make our own weapon
                     if (cWpn == 0) {
                        cWpn = new Simulation::Missile();
                        cWpn->setMaxBurstRng(maxBurstRange);
                        cWpn->setLethalRange(maxLethalRange);
                        cWpn->setDummy(true);
                        cWpn->container((Basic::Component*)sim);
                        cWpn->setGeocPosition(worldPos, true);
                        p->processDetonation(rng, cWpn);
                     }
                     else {
                        p->processDetonation(rng, cWpn);
                     }
                  }
               }
               item = item->getNext();
            }
         }
         plist->unref();
         plist = 0;
         if (cWpn != 0) {
            cWpn->container(0);
            cWpn->unref();
            cWpn = 0;
         }
      }
   }
}


} // End Dis namespace
} // End Network namespace
} // End Eaagles namespace
