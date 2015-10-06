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
      checkDetonationManually(pdu->location);
   }
}

void NetIO::checkDetonationManually(WorldCoordinates wc)
{
   osg::Vec3d worldPos(wc.X_coord, wc.Y_coord, wc.Z_coord);
   Simulation::Station* stn = getStation();
   if (stn != 0) {
      Simulation::Simulation* sim = stn->getSimulation();
      if (sim != 0) {
         // right now, let's just say the max burst range is manual at 500ft
         LCreal maxRng = 10.0f * 500.0f;
         Basic::PairStream* plist = sim->getPlayers();
         if (plist != 0) {
            Basic::List::Item* item = plist->getFirstItem();
            // Process the detonation for all local, in-range players
            bool finished = false;
            while (item != 0 && !finished) {
               Basic::Pair* pair = static_cast<Basic::Pair*>(item->getValue());
               Simulation::Player* p = static_cast<Simulation::Player*>(pair->object()); 
               finished = p->isNetworkedPlayer();  // local only
               if (!finished) {
                  const osg::Vec3d dPos = p->getGeocPosition() - worldPos;
                  LCreal rng = dPos.length();
                  if ( (rng <= maxRng) ) {
                      // make a fake weapon that has the correct parameters - the correct way is to look
                      // it up by DIS enumeration
                      Simulation::Missile* wpn = new Simulation::Missile();
                      wpn->setMaxBurstRng(500);
                      wpn->setLethalRange(500);

                      p->processDetonation(rng, wpn);
                      wpn->unref();
                  }
               }
               item = item->getNext();
            }
         }
         // cleanup
         plist->unref();
         plist = 0;
      }
   }
}

} // End Dis namespace
} // End Network namespace
} // End Eaagles namespace
