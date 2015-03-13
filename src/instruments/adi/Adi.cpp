#include "openeaagles/instruments/adi/Adi.h"

namespace Eaagles {
namespace Instruments {

IMPLEMENT_SUBCLASS(Adi, "Adi")
EMPTY_SERIALIZER(Adi)

//------------------------------------------------------------------------------
// Slot table for this form type
//------------------------------------------------------------------------------
BEGIN_SLOTTABLE(Adi)
    "maxRate",      // rate at which we drive towards pitch and roll 
END_SLOTTABLE(Adi)

//------------------------------------------------------------------------------
//  Map slot table to handles 
//------------------------------------------------------------------------------
BEGIN_SLOT_MAP(Adi)
    ON_SLOT(1, setSlotMaxRate, Basic::Angle)     // we can be sent an angle (degrees or radians) / per second
    ON_SLOT(1, setSlotMaxRate, Basic::Number)    // or a number as degrees per second
END_SLOT_MAP()


//------------------------------------------------------------------------------
// Event handler
//------------------------------------------------------------------------------
BEGIN_EVENT_HANDLER(Adi)
    // override the instrument event, since we need to use it ourself
    ON_EVENT_OBJ(UPDATE_INSTRUMENTS, onUpdatePitchAdi, Basic::Number)
    ON_EVENT_OBJ(UPDATE_VALUE, onUpdateRollDegAdi, Basic::Number)
    ON_EVENT_OBJ(UPDATE_VALUE2, onUpdateRollRadAdi, Basic::Number)
    ON_EVENT_OBJ(UPDATE_VALUE3, onUpdateMaxRateAdi, Basic::Number)
END_EVENT_HANDLER()

//------------------------------------------------------------------------------
// Constructor(s)
//------------------------------------------------------------------------------
Adi::Adi()
{
    STANDARD_CONSTRUCTOR()
    scaledPitch = 0;
    roll = 0;
    pitch = 0;
    curTheta = 0;
    curPhi = 0;
    maxRate = 500;  // default to extremely high degrees/second (for instantaneous movement)
}

//------------------------------------------------------------------------------
// copyData() -- copy member data
//------------------------------------------------------------------------------
void Adi::copyData(const Adi& org, const bool)
{
    BaseClass::copyData(org);
    scaledPitch = org.scaledPitch;
    pitch = org.pitch;
    roll = org.roll;
    curTheta = org.curTheta;
    curPhi = org.curPhi;
    maxRate = org.maxRate;
}

//------------------------------------------------------------------------------
// deleteData() -- delete member data
//------------------------------------------------------------------------------
EMPTY_DELETEDATA(Adi)

//------------------------------------------------------------------------------
// draw() - 
//------------------------------------------------------------------------------
void Adi::draw()
{
    lcSaveMatrix();
        lcRotate(curPhi);
        lcTranslate(0, scaledPitch);
        BaseClass::draw();   
    lcRestoreMatrix();
}

//------------------------------------------------------------------------------
// updateData() - 
//------------------------------------------------------------------------------
void Adi::updateData(const LCreal dt)
{
    // update our base class first
    BaseClass::updateData(dt);
    
    // drive our adi toward the actual pitch, from our current pitch, no faster
    // than our MAX_RATE (this allows for greater fidelity, simulates an analog adi)
    LCreal delta = 0;
    delta = alim (lcAepcDeg(pitch - curTheta), maxRate * dt);    
    curTheta = lcAepcDeg(curTheta + delta);
    
    // now do the same thing for roll
    delta = alim (lcAepcRad(roll - curPhi), maxRate * dt);
    curPhi = lcAepcRad(curPhi + delta);
    
    // get our table, and do the linear interpolation ourself
    setInstVal(curTheta);
    scaledPitch = getInstValue();
}

// SLOT functions
//------------------------------------------------------------------------------
// setSlotMaxRate(Basic::Angle)
//------------------------------------------------------------------------------
bool Adi::setSlotMaxRate(const Basic::Angle* const newMR)
{
    bool ok = false;
    if (newMR != 0) ok = setMaxRate( static_cast<LCreal>(Basic::Degrees::convertStatic(*newMR)) );
    return ok;
}
//------------------------------------------------------------------------------
// setSlotMaxRate() - 
//------------------------------------------------------------------------------
bool Adi::setSlotMaxRate(const Basic::Number* const newMR)
{
    bool ok = false;
    if (newMR != 0) ok = setMaxRate(newMR->getReal());
    return ok;
}

// Event functions
//------------------------------------------------------------------------------
// onUpdateRollDegAdi() - update roll by degrees
//------------------------------------------------------------------------------
bool Adi::onUpdateRollDegAdi(const Basic::Number* const newR) 
{
    bool ok = false;
    if (newR != 0) ok = setRollDeg(newR->getReal());
    return ok;
}
//------------------------------------------------------------------------------
// onUpdateRollRadAdi() - update roll by radians
//------------------------------------------------------------------------------
bool Adi::onUpdateRollRadAdi(const Basic::Number* const newR) 
{
    bool ok = false;
    if (newR != 0) ok = setRollRad(newR->getReal());
    return ok;
}
//------------------------------------------------------------------------------
// onUpdatePitchAdi() - update pitch (degrees)
//------------------------------------------------------------------------------
bool Adi::onUpdatePitchAdi(const Basic::Number* const newP)
{
    bool ok = false;
    if (newP != 0) ok = setPitch(newP->getReal());
    return ok;
}
//------------------------------------------------------------------------------
// onUpdateMaxRateAdi() - set our max rate for the ADI
//------------------------------------------------------------------------------
bool Adi::onUpdateMaxRateAdi(const Basic::Number* const newMR)
{
    bool ok = false;
    if (newMR != 0) ok = setMaxRate(newMR->getReal());
    return ok;
}

// SET functions
//------------------------------------------------------------------------------
// setRollDeg() - set our amount of roll in degrees
//------------------------------------------------------------------------------
bool Adi::setRollDeg(const LCreal newR) 
{
    roll = newR * static_cast<LCreal>(Basic::Angle::D2RCC);
    return true;
}
//------------------------------------------------------------------------------
// setRollRad() - set roll in radians
//------------------------------------------------------------------------------
bool Adi::setRollRad(const LCreal newR) 
{
    roll = newR;
    return true;
}
//------------------------------------------------------------------------------
// setPitch() - set our pitch value (degrees)
//------------------------------------------------------------------------------
bool Adi::setPitch(const LCreal newP)
{
    pitch = newP;
    return true;
}
//------------------------------------------------------------------------------
// setMaxRate() - set our max rate 
//------------------------------------------------------------------------------
bool Adi::setMaxRate(const LCreal newMR)
{
    maxRate = newMR;
    return true;
}

//------------------------------------------------------------------------------
// getSlotByIndex() for Adi
//------------------------------------------------------------------------------
Basic::Object* Adi::getSlotByIndex(const int si)
{
    return BaseClass::getSlotByIndex(si);
}

}; // end of Instruments namespace
}; // end of Eaagles namespace
