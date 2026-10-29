// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "Firmware/Application/Model/ModelParams.h"

namespace IgniteDesktop::Media
{
    // The eight CV destinations occupy the gap between DIN and USB MIDI IDs.
    inline bool isMidiPortId(int port)
    {
        return (port>=int(midiDataIOPorts::dinMidi1) && port<=int(midiDataIOPorts::dinMidi5)) ||
               (port>=int(midiDataIOPorts::usbDevMidi1) && port<=int(midiDataIOPorts::usbHostMidi5));
    }
}
