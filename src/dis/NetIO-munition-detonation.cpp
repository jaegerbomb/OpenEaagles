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

   // Get the Firing Player's ID
   unsigned short fPlayerId = pdu->firingEntityID.ID;
   unsigned short fSiteId = pdu->firingEntityID.simulationID.siteIdentification;
   unsigned short fApplicationId = pdu->firingEntityID.simulationID.applicationIdentification;

   // Ignore our own PDUs
   if (fSiteId == getSiteID() && fApplicationId == getApplicationID()) return;

   // Get the Munition Player's ID
   unsigned short mPlayerId = pdu->munitionID.ID;
   unsigned short mSiteId = pdu->munitionID.simulationID.siteIdentification;
   unsigned short mApplicationId = pdu->munitionID.simulationID.applicationIdentification;

   // Get the Target Player's ID
   unsigned short tPlayerId = pdu->targetEntityID.ID;
   unsigned short tSiteId = pdu->targetEntityID.simulationID.siteIdentification;
   unsigned short tApplicationId = pdu->targetEntityID.simulationID.applicationIdentification;

   // ---
   // 1) Find the target player
   // ---
   Simulation::Player* tPlayer = 0;
   if (tPlayerId != 0 && tSiteId != 0 && tApplicationId != 0) {
      Simulation::Nib* tNib = findDisNib(tPlayerId, tSiteId, tApplicationId, OUTPUT_NIB);
      if (tNib != 0) {
         tPlayer = tNib->getPlayer();
      }
   }
   //std::cout << "Net kill(2) tPlayer = " << tPlayer << std::endl;

   // ---
   // 2) Find the firing player and munitions (networked) IPlayers
   // ---
   Simulation::Player* fPlayer = 0;
   if (fPlayerId != 0 && fSiteId != 0 && fApplicationId != 0) {
      Simulation::Nib* fNib = findDisNib(fPlayerId, fSiteId, fApplicationId, INPUT_NIB);
      if (fNib != 0) {
         fPlayer = fNib->getPlayer();
      }
      else {
         SPtr<Basic::PairStream> players( getSimulation()->getPlayers() );
         fPlayer = getSimulation()->findPlayer(fPlayerId);
      }
   }

   Simulation::Nib* mNib = 0;
   if (mPlayerId != 0 && mSiteId != 0 && mApplicationId != 0) {
      mNib = findDisNib(mPlayerId, mSiteId, mApplicationId, INPUT_NIB);
   }

    //std::cout << "Net kill(3) fNib = " << fNib << ", mNib = " << mNib << std::endl;

   // ---
   // 3) Update the data of the munition's NIB and player
   // ---
   Simulation::Weapon* mPlayer = 0;
   if (mNib != 0) {

      // ---
      // a) Set the munition's NIB to the location of the detonation
      // ---

      // Get the geocentric position, velocity and acceleration from the PDU
      osg::Vec3d geocPos;
      geocPos[Basic::Nav::IX] = pdu->location.X_coord;
      geocPos[Basic::Nav::IY] = pdu->location.Y_coord;
      geocPos[Basic::Nav::IZ] = pdu->location.Z_coord;

      osg::Vec3d geocVel;
      geocVel[Basic::Nav::IX] = pdu->velocity.component[0];
      geocVel[Basic::Nav::IY] = pdu->velocity.component[1];
      geocVel[Basic::Nav::IZ] = pdu->velocity.component[2];

      osg::Vec3d geocAcc(0,0,0);
      osg::Vec3d geocAngles(0,0,0);
      osg::Vec3d arates(0,0,0);

      // (re)initialize the dead reckoning function
      mNib->resetDeadReckoning(
         Simulation::Nib::STATIC_DRM,
         geocPos,
         geocVel,
         geocAcc,
         geocAngles,
         arates);

      // Set the NIB's mode to DETONATED
      mNib->setMode(Simulation::Player::DETONATED);

      // Find the munition player and set its mode, location and target position
      mPlayer = dynamic_cast<Simulation::Weapon*>(mNib->getPlayer());
      if (mPlayer != 0) {

         // Munition's mode
         mPlayer->setMode(Simulation::Player::DETONATED);

         // munition's position, velocity and acceleration at the time of the detonation
         mPlayer->setGeocPosition(geocPos);
         mPlayer->setGeocVelocity(geocVel);
         mPlayer->setGeocAcceleration(geocAcc);

         // detonation results
         mPlayer->setDetonationResults(Simulation::Weapon::Detonation(pdu->detonationResult));

         // Munition's target player and the location of detonation relative to target
         mPlayer->setTargetPlayer(tPlayer,false);
         LCreal x = pdu->locationInEntityCoordinates.component[0];
         LCreal y = pdu->locationInEntityCoordinates.component[1];
         LCreal z = pdu->locationInEntityCoordinates.component[2];
         osg::Vec3 loc(x,y,z);
         mPlayer->setDetonationLocation(loc);

         // Munition's launcher
         if (mPlayer->getLaunchVehicle() == 0 && fPlayer != 0) {
            mPlayer->setLaunchVehicle(fPlayer);
         }
      }
   }

   // ---
   // 4) Check all local players for the effects of the detonation
   // ---
   if (mPlayer != 0) {
      mPlayer->checkDetonationEffect();
   }
   // NO player was found, let's try to use world coordinates!
   else {
      checkDetonationManually(pdu);
   }
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
                  // take out the altitude component of it for now
                  const osg::Vec2d flatPos(worldPos.x(), worldPos.y());
                  const osg::Vec2d flatPlayerPos(p->getGeocPosition().x(),  p->getGeocPosition().y());
                  const osg::Vec2d dPos = flatPlayerPos - flatPos;
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
