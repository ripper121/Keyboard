/*
This is the part of the TrinketHidCombo code that is exposed to the user
See the header file for more comments on the functions

Copyright (c) 2013 Adafruit Industries
All rights reserved.

TrinketHidCombo is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

TrinketHidCombo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with TrinketHidCombo. If not, see
<http://www.gnu.org/licenses/>.
*/

#include "cmdline_defs.h"
#include "TrinketHidCombo.h"
#include "TrinketHidComboC.h"

#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// create an instance that the user can use
Trinket_Hid_Combo TrinketHidCombo;

// empty constructor
Trinket_Hid_Combo::Trinket_Hid_Combo()
{
	// nothing to do
}

// starts the USB driver, causes re-enumeration
void Trinket_Hid_Combo::begin()
{
	usbBegin();
}

// this must be called at least once every 10ms
void Trinket_Hid_Combo::poll()
{
	usbPollWrapper();
}

void Trinket_Hid_Combo::pressKey(uint8_t modifiers, uint8_t keycode1)
{
		// construct the report, follow the standard format as described
	// this format is compatible with "boot protocol"
	report_buffer[1] = modifiers;
	report_buffer[2] = 0; // reserved
	report_buffer[3] = keycode1;
	report_buffer[4] = 0;
	report_buffer[5] = 0;
	report_buffer[6] = 0;
	report_buffer[7] = 0;
	report_buffer[0] = REPID_KEYBOARD;
	usbReportSend(REPSIZE_KEYBOARD);
}

size_t Trinket_Hid_Combo::write(uint8_t ascii)
{
	//typeChar(ascii);
	return 1;
}

void Trinket_Hid_Combo::pressMultimediaKey(uint8_t key)
{
	report_buffer[0] = REPID_MMKEY;
	report_buffer[1] = key;
	report_buffer[2] = 0;
	usbReportSend(REPSIZE_MMKEY);
	// immediate release
	report_buffer[0] = REPID_MMKEY;
	report_buffer[1] = 0;
	report_buffer[2] = 0;
	usbReportSend(REPSIZE_MMKEY);
}

// checks if USB is connected, 0 if not connected
char Trinket_Hid_Combo::isConnected()
{
	return usb_hasCommed;
}