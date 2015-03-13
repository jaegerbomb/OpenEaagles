#include "openeaagles/instruments/buttons/Button.h"
#include "openeaagles/basic/Number.h"
#include "openeaagles/basicGL/Display.h"

namespace Eaagles {
namespace Instruments {

IMPLEMENT_SUBCLASS(Button,"Button")
EMPTY_SERIALIZER(Button)

//------------------------------------------------------------------------------
// Slot table for this form type
//------------------------------------------------------------------------------
BEGIN_SLOTTABLE(Button)
    "eventId",        // Which event we will send for each button (A, B, C events)
END_SLOTTABLE(Button)

//------------------------------------------------------------------------------
//  Map slot table to handles 
//------------------------------------------------------------------------------
BEGIN_SLOT_MAP(Button)
    ON_SLOT(1, setSlotEventId, Basic::Number)
END_SLOT_MAP()

//------------------------------------------------------------------------------
// Event Table
//------------------------------------------------------------------------------
BEGIN_EVENT_HANDLER(Button)
    ON_EVENT(ON_SINGLE_CLICK, onSingleClick)
    ON_EVENT(ON_CANCEL, onCancel)
END_EVENT_HANDLER()

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
Button::Button()
{
    STANDARD_CONSTRUCTOR()
    eventId = -1;
}

//------------------------------------------------------------------------------
// copyData() -- copy this object's data
//------------------------------------------------------------------------------
void Button::copyData(const Button& org, const bool)
{
    BaseClass::copyData(org);
    eventId = org.eventId;
}

//------------------------------------------------------------------------------
// deleteData() -- delete this object's data
//------------------------------------------------------------------------------
EMPTY_DELETEDATA(Button)

//------------------------------------------------------------------------------
// setSlotEventId() - sets our slot event Id
//------------------------------------------------------------------------------
bool Button::setSlotEventId(const Basic::Number* const newEvent)
{
    bool ok = false;
    if (newEvent != 0) {
        int a = newEvent->getInt();
        ok = setEventId(a);
    }
    return ok;
}

//------------------------------------------------------------------------------
// onSingleClick() - tells us we have been clicked, and we can override this
// to make it do whatever we want.
//------------------------------------------------------------------------------
bool Button::onSingleClick()
{
    // when I am clicked, I will send an event to my container, we find out what
    // event Id we have, and send that eventId
    bool ok = false;
    BasicGL::Display* myDisplay = (BasicGL::Display*)findContainerByType(typeid(BasicGL::Display));
    
    if (myDisplay != 0) {
        myDisplay->buttonEvent(getEventId());
        ok = true;
    }

    return ok;
}

//------------------------------------------------------------------------------
// onCancel() - this is where we cancel button pushes
//------------------------------------------------------------------------------
bool Button::onCancel()
{
    // do nothing here, but our pushbuttons and switches will!
    return true;
}

//------------------------------------------------------------------------------
// getSlotByIndex() for Button
//------------------------------------------------------------------------------
Basic::Object* Button::getSlotByIndex(const int si)
{
    return BaseClass::getSlotByIndex(si);
}

}  // end Instruments namespace
}  // end Eaagles namespace
