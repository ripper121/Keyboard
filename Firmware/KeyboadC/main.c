/* Name: main.c
* Project: Pr0Keyb0ard
* Author: Ripper121 / aka Unknown23 @ www.pr0gramm.com
* Creation Date: 2014-08-27
* Copyright: (c) 2014 by Ripper121
* License: GNU GPL v2 (see License.txt)
* This Revision: $Id$
*/

#define F_CPU 12000000UL
#include "usbdrv/usbdrv.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>

uint8_t report_buffer[8];
char usb_hasCommed = 0;
uint8_t idle_rate = 500 / 4;  // see HID1_11.pdf sect 7.2.4
uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6
uint8_t led_state = 0; // caps/num/scroll lock LEDs

/***** Configure IO *****/
#define pinIN(bit)             {bit ## _DDR &= ~(1 << bit);}
#define pinOUT(bit)            {bit ## _DDR |= (1 << bit);}

#define pullUpOn(bit)                      {bit ## _PORT |= (1 << bit);}
#define pullUpOff(bit)                     {bit ## _PORT &= ~(1 << bit);}

#define disable_digital_input(channel)      {DIDR0 |= (1 << channel);}
#define enable_digital_input(channel)       {DIDR0 &= ~(1 << channel);}

/***** Manipulate Outputs *****/
#define set_high(bit)                       {bit ## _PORT |= (1 << bit);}
#define set_low(bit)                        {bit ## _PORT &= ~(1 << bit);}
#define toggle(bit)                         {bit ## _PIN |= (1 << bit);}

/***** Test Inputs *****/
#define is_high(bit)                        (bit ## _PIN & (1 << bit))
#define is_low(bit)                         (! (bit ## _PIN & (1 << bit)))

/* ----------------------- hardware I/O abstraction ------------------------ */

#define LED1                PC0
#define LED1_PORT           PORTC
#define LED1_DDR            DDRC
#define LED1_PIN            PINC

#define LED2                PC1
#define LED2_PORT           PORTC
#define LED2_DDR            DDRC
#define LED2_PIN            PINC

#define COL1                PC2
#define COL1_PORT           PORTC
#define COL1_DDR            DDRC
#define COL1_PIN            PINC

#define COL2                PC3
#define COL2_PORT           PORTC
#define COL2_DDR            DDRC
#define COL2_PIN            PINC

#define COL3                PC4
#define COL3_PORT           PORTC
#define COL3_DDR            DDRC
#define COL3_PIN            PINC

#define COL4                PC5
#define COL4_PORT           PORTC
#define COL4_DDR            DDRC
#define COL4_PIN            PINC

#define ROW1                PD0
#define ROW1_PORT           PORTD
#define ROW1_DDR            DDRD
#define ROW1_PIN            PIND

#define ROW2                PD1
#define ROW2_PORT           PORTD
#define ROW2_DDR            DDRD
#define ROW2_PIN            PIND

static void resetMatrix(void)
{
	//Key Matrix
	//COLS
	pinIN(COL1);//Input
	pullUpOff(COL1);//Low
	pinIN(COL2);//Input
	pullUpOff(COL2);//Low
	pinIN(COL3);//Input
	pullUpOff(COL3);//Low
	pinIN(COL4);//Input
	pullUpOff(COL4);//Low
	//ROWS
	pinIN(ROW1);//Input
	pullUpOff(ROW1);//Low
	pinIN(ROW2);//Input
	pullUpOff(ROW2);//Low
}

static void hardwareInit(void)
{
	TCCR0 = 5;      /* timer 0 prescaler: 1024 */
	
	//Led
	pinOUT(LED1);
	pinOUT(LED2);
	set_low(LED1);
	set_low(LED2);
	
	resetMatrix();
}

static uchar getKey(void)
{
	//Volume Keys
	resetMatrix();
	pinOUT(ROW1);
	set_high(ROW1);
	pinOUT(ROW2);
	set_low(ROW2);

	pinIN(COL1);
	pullUpOn(COL1);
	pinIN(COL3);
	pullUpOn(COL3);
	pinIN(COL4);
	pullUpOn(COL4);
	_delay_ms(1);
	
	if (is_low(COL1) && is_low(COL3)) {
		return 10;
	}
	if (is_low(COL1) && is_low(COL4)) {
		return 9;
	}
	
	//ROW1 & COLS
	resetMatrix();
	pinOUT(ROW1);
	set_low(ROW1);
	pinOUT(ROW2);
	set_high(ROW2);
	
	pinIN(COL1);
	pullUpOn(COL1);
	pinIN(COL2);
	pullUpOn(COL2);
	pinIN(COL3);
	pullUpOn(COL3);
	pinIN(COL4);
	pullUpOn(COL4);
	_delay_ms(1);
	
	if (is_low(COL1)) {
		return 6;
	}
	if (is_low(COL2)) {
		return 7;
	}
	if (is_low(COL3)) {
		return 1;
	}
	if (is_low(COL4)) {
		return 8;
	}
	
	//ROW2 & COLS
	resetMatrix();
	pinOUT(ROW1);
	set_high(ROW1);
	pinOUT(ROW2);
	set_low(ROW2);

	pinIN(COL1);
	pullUpOn(COL1);
	pinIN(COL2);
	pullUpOn(COL2);
	pinIN(COL3);
	pullUpOn(COL3);
	pinIN(COL4);
	pullUpOn(COL4);
	_delay_ms(1);
	
	if (is_low(COL1)) {
		return 2;
	}
	if (is_low(COL2)) {
		return 5;
	}
	if (is_low(COL3)) {
		return 4; //3
	}
	if (is_low(COL4)) {
		return 3; //4
	}
	
	resetMatrix();
	return 0;
}

void usbBegin()
{
	cli();
	// fake a disconnect to force the computer to re-enumerate
	PORTB &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT));
	usbDeviceDisconnect();
	_delay_ms(250);
	usbDeviceConnect();
	// start the USB driver
	usbInit();
	sei();
}

void usbReportSend(uint8_t sz)
{
	// perform usb background tasks until the report can be sent, then send it
	while (1)
	{
		usbPoll(); // this needs to be called at least once every 10 ms
		if (usbInterruptIsReady())
		{
			usbSetInterrupt((uint8_t*)report_buffer, sz); // send
			break;

			// see http://vusb.wikidot.com/driver-api
		}
	}
}

#define REPID_MOUSE         1
#define REPID_KEYBOARD      2
#define REPID_MMKEY         3
#define REPID_SYSCTRLKEY    4
#define REPSIZE_MOUSE       4
#define REPSIZE_KEYBOARD    8
#define REPSIZE_MMKEY       3
#define REPSIZE_SYSCTRLKEY  2

// use these masks with the "move" function
#define MOUSEBTN_LEFT_MASK		0x01
#define MOUSEBTN_RIGHT_MASK		0x02
#define MOUSEBTN_MIDDLE_MASK	0x04

// LED state masks
#define KB_LED_NUM			0x01
#define KB_LED_CAPS			0x02
#define KB_LED_SCROLL		0x04

// some convenience definitions for modifier keys
#define KEYCODE_MOD_LEFT_CONTROL	0x01
#define KEYCODE_MOD_LEFT_SHIFT		0x02
#define KEYCODE_MOD_LEFT_ALT		0x04
#define KEYCODE_MOD_LEFT_GUI		0x08
#define KEYCODE_MOD_RIGHT_CONTROL	0x10
#define KEYCODE_MOD_RIGHT_SHIFT		0x20
#define KEYCODE_MOD_RIGHT_ALT		0x40
#define KEYCODE_MOD_RIGHT_GUI		0x80

// some more keycodes
#define KEYCODE_LEFT_CONTROL	0xE0
#define KEYCODE_LEFT_SHIFT		0xE1
#define KEYCODE_LEFT_ALT		0xE2
#define KEYCODE_LEFT_GUI		0xE3
#define KEYCODE_RIGHT_CONTROL	0xE4
#define KEYCODE_RIGHT_SHIFT		0xE5
#define KEYCODE_RIGHT_ALT		0xE6
#define KEYCODE_RIGHT_GUI		0xE7
#define KEYCODE_1				0x1E
#define KEYCODE_2				0x1F
#define KEYCODE_3				0x20
#define KEYCODE_4				0x21
#define KEYCODE_5				0x22
#define KEYCODE_6				0x23
#define KEYCODE_7				0x24
#define KEYCODE_8				0x25
#define KEYCODE_9				0x26
#define KEYCODE_0				0x27
#define KEYCODE_A				0x04
#define KEYCODE_B				0x05
#define KEYCODE_C				0x06
#define KEYCODE_D				0x07
#define KEYCODE_E				0x08
#define KEYCODE_F				0x09
#define KEYCODE_G				0x0A
#define KEYCODE_H				0x0B
#define KEYCODE_I				0x0C
#define KEYCODE_J				0x0D
#define KEYCODE_K				0x0E
#define KEYCODE_L				0x0F
#define KEYCODE_M				0x10
#define KEYCODE_N				0x11
#define KEYCODE_O				0x12
#define KEYCODE_P				0x13
#define KEYCODE_Q				0x14
#define KEYCODE_R				0x15
#define KEYCODE_S				0x16
#define KEYCODE_T				0x17
#define KEYCODE_U				0x18
#define KEYCODE_V				0x19
#define KEYCODE_W				0x1A
#define KEYCODE_X				0x1B
#define KEYCODE_Y				0x1C
#define KEYCODE_Z				0x1D
#define KEYCODE_COMMA			0x36
#define KEYCODE_PERIOD			0x37
#define KEYCODE_MINUS			0x2D
#define KEYCODE_EQUAL			0x2E
#define KEYCODE_BACKSLASH		0x31
#define KEYCODE_SQBRAK_LEFT		0x2F
#define KEYCODE_SQBRAK_RIGHT	0x30
#define KEYCODE_SLASH			0x38
#define KEYCODE_F1				0x3A
#define KEYCODE_F2				0x3B
#define KEYCODE_F3				0x3C
#define KEYCODE_F4				0x3D
#define KEYCODE_F5				0x3E
#define KEYCODE_F6				0x3F
#define KEYCODE_F7				0x40
#define KEYCODE_F8				0x41
#define KEYCODE_F9				0x42
#define KEYCODE_F10				0x43
#define KEYCODE_F11				0x44
#define KEYCODE_F12				0x45
#define KEYCODE_APP				0x65
#define KEYCODE_ENTER			0x28
#define KEYCODE_BACKSPACE		0x2A
#define KEYCODE_ESC				0x29
#define KEYCODE_TAB				0x2B
#define KEYCODE_SPACE			0x2C
#define KEYCODE_INSERT			0x49
#define KEYCODE_HOME			0x4A
#define KEYCODE_PAGE_UP			0x4B
#define KEYCODE_DELETE			0x4C
#define KEYCODE_END				0x4D
#define KEYCODE_PAGE_DOWN		0x4E
#define KEYCODE_PRINTSCREEN		0x46
#define KEYCODE_ARROW_RIGHT		0x4F
#define KEYCODE_ARROW_LEFT		0x50
#define KEYCODE_ARROW_DOWN		0x51
#define KEYCODE_ARROW_UP		0x52
#define KEYCODE_KPminus			0x56
#define KEYCODE_KPplus			0x57

// multimedia keys
#define MMKEY_KB_VOL_UP			0x80 // do not use
#define MMKEY_KB_VOL_DOWN		0x81 // do not use
#define MMKEY_VOL_UP			0xE9
#define MMKEY_VOL_DOWN			0xEA
#define MMKEY_SCAN_NEXT_TRACK	0xB5
#define MMKEY_SCAN_PREV_TRACK	0xB6
#define MMKEY_STOP				0xB7
#define MMKEY_PLAYPAUSE			0xCD
#define MMKEY_MUTE				0xE2
#define MMKEY_BASSBOOST			0xE5
#define MMKEY_LOUDNESS			0xE7
#define MMKEY_KB_EXECUTE		0x74
#define MMKEY_KB_HELP			0x75
#define MMKEY_KB_MENU			0x76
#define MMKEY_KB_SELECT			0x77
#define MMKEY_KB_STOP			0x78
#define MMKEY_KB_AGAIN			0x79
#define MMKEY_KB_UNDO			0x7A
#define MMKEY_KB_CUT			0x7B
#define MMKEY_KB_COPY			0x7C
#define MMKEY_KB_PASTE			0x7D
#define MMKEY_KB_FIND			0x7E
#define MMKEY_KB_MUTE			0x7F // do not use

// system control keys
#define SYSCTRLKEY_POWER		0x01
#define SYSCTRLKEY_SLEEP		0x02
#define SYSCTRLKEY_WAKE			0x03

// USB HID report descriptor for boot protocol keyboard
// see HID1_11.pdf appendix B section 1
// USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH is defined in usbconfig (should be 173)
const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,           // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,           // USAGE (Mouse)
	0xa1, 0x01,           // COLLECTION (Application)
	0x09, 0x01,           //   USAGE (Pointer)
	0xA1, 0x00,           //   COLLECTION (Physical)
	0x85, REPID_MOUSE,    //     REPORT_ID
	0x05, 0x09,           //     USAGE_PAGE (Button)
	0x19, 0x01,           //     USAGE_MINIMUM
	0x29, 0x03,           //     USAGE_MAXIMUM
	0x15, 0x00,           //     LOGICAL_MINIMUM (0)
	0x25, 0x01,           //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,           //     REPORT_COUNT (3)
	0x75, 0x01,           //     REPORT_SIZE (1)
	0x81, 0x02,           //     INPUT (Data,Var,Abs)
	0x95, 0x01,           //     REPORT_COUNT (1)
	0x75, 0x05,           //     REPORT_SIZE (5)
	0x81, 0x03,           //     INPUT (Const,Var,Abs)
	0x05, 0x01,           //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,           //     USAGE (X)
	0x09, 0x31,           //     USAGE (Y)
	0x15, 0x81,           //     LOGICAL_MINIMUM (-127)
	0x25, 0x7F,           //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,           //     REPORT_SIZE (8)
	0x95, 0x02,           //     REPORT_COUNT (2)
	0x81, 0x06,           //     INPUT (Data,Var,Rel)
	0xC0,                 //   END_COLLECTION
	0xC0,                 // END COLLECTION

	0x05, 0x01,           // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,           // USAGE (Keyboard)
	0xA1, 0x01,           // COLLECTION (Application)
	0x85, REPID_KEYBOARD, // REPORT_ID
	0x75, 0x01,           //   REPORT_SIZE (1)
	0x95, 0x08,           //   REPORT_COUNT (8)
	0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xE0,           //   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xE7,           //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00,           //   LOGICAL_MINIMUM (0)
	0x25, 0x01,           //   LOGICAL_MAXIMUM (1)
	0x81, 0x02,           //   INPUT (Data,Var,Abs) ; Modifier byte
	0x95, 0x01,           //   REPORT_COUNT (1)
	0x75, 0x08,           //   REPORT_SIZE (8)
	0x81, 0x03,           //   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05,           //   REPORT_COUNT (5)
	0x75, 0x01,           //   REPORT_SIZE (1)
	0x05, 0x08,           //   USAGE_PAGE (LEDs)
	0x19, 0x01,           //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,           //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,           //   OUTPUT (Data,Var,Abs) ; LED report
	0x95, 0x01,           //   REPORT_COUNT (1)
	0x75, 0x03,           //   REPORT_SIZE (3)
	0x91, 0x03,           //   OUTPUT (Cnst,Var,Abs) ; LED report padding
	0x95, 0x05,           //   REPORT_COUNT (5)
	0x75, 0x08,           //   REPORT_SIZE (8)
	0x15, 0x00,           //   LOGICAL_MINIMUM (0)
	0x26, 0xA4, 0x00,     //   LOGICAL_MAXIMUM (164)
	0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00,           //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x2A, 0xA4, 0x00,     //   USAGE_MAXIMUM (Keyboard Application)(164)
	0x81, 0x00,           //   INPUT (Data,Ary,Abs)
	0xC0,                 // END_COLLECTION

	// this second multimedia key report is what handles the multimedia keys
	0x05, 0x0C,           // USAGE_PAGE (Consumer Devices)
	0x09, 0x01,           // USAGE (Consumer Control)
	0xA1, 0x01,           // COLLECTION (Application)
	0x85, REPID_MMKEY,    //   REPORT_ID
	0x19, 0x00,           //   USAGE_MINIMUM (Unassigned)
	0x2A, 0x3C, 0x02,     //   USAGE_MAXIMUM
	0x15, 0x00,           //   LOGICAL_MINIMUM (0)
	0x26, 0x3C, 0x02,     //   LOGICAL_MAXIMUM
	0x95, 0x01,           //   REPORT_COUNT (1)
	0x75, 0x10,           //   REPORT_SIZE (16)
	0x81, 0x00,           //   INPUT (Data,Ary,Abs)
	0xC0,                 // END_COLLECTION

	// system controls, like power, needs a 3rd different report and report descriptor
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x80,             // USAGE (System Control)
	0xA1, 0x01,             // COLLECTION (Application)
	0x85, REPID_SYSCTRLKEY, //   REPORT_ID
	0x95, 0x01,             //   REPORT_COUNT (1)
	0x75, 0x02,             //   REPORT_SIZE (2)
	0x15, 0x01,             //   LOGICAL_MINIMUM (1)
	0x25, 0x03,             //   LOGICAL_MAXIMUM (3)
	0x09, 0x82,             //   USAGE (System Sleep)
	0x09, 0x81,             //   USAGE (System Power)
	0x09, 0x83,             //   USAGE (System Wakeup)
	0x81, 0x60,             //   INPUT
	0x75, 0x06,             //   REPORT_SIZE (6)
	0x81, 0x03,             //   INPUT (Cnst,Var,Abs)
	0xC0,                   // END_COLLECTION
};


// see http://vusb.wikidot.com/driver-api
// constants are found in usbdrv.h
usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	usb_hasCommed = 1;

	// see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
	return 0; // ignore request if it's not a class specific request

	// see HID1_11.pdf sect 7.2
	switch (rq->bRequest)
	{
		case USBRQ_HID_GET_IDLE:
		usbMsgPtr = &idle_rate; // send data starting from this byte
		return 1; // send 1 byte
		case USBRQ_HID_SET_IDLE:
		idle_rate = rq->wValue.bytes[1]; // read in idle rate
		return 0; // send nothing
		case USBRQ_HID_GET_PROTOCOL:
		usbMsgPtr = &protocol_version; // send data starting from this byte
		return 1; // send 1 byte
		case USBRQ_HID_SET_PROTOCOL:
		protocol_version = rq->wValue.bytes[1];
		return 0; // send nothing
		case USBRQ_HID_GET_REPORT:
		usbMsgPtr = (uint8_t*)&report_buffer; // send the report data
		report_buffer[0] = rq->wValue.bytes[0];
		report_buffer[1] = report_buffer[2] = report_buffer[3] = report_buffer[4] = report_buffer[5] = report_buffer[6] = report_buffer[7] = 0; // clear the report
		// determine the return data length based on which report ID was requested
		if (rq->wValue.bytes[0] == REPID_MOUSE)      return REPSIZE_MOUSE;
		if (rq->wValue.bytes[0] == REPID_KEYBOARD)   return REPSIZE_KEYBOARD;
		if (rq->wValue.bytes[0] == REPID_MMKEY)      return REPSIZE_MMKEY;
		if (rq->wValue.bytes[0] == REPID_SYSCTRLKEY) return REPSIZE_SYSCTRLKEY;
		return 8; // default
		case USBRQ_HID_SET_REPORT:
		if (rq->wLength.word == 2) // check data is available
		{
			// 1st is the report byte, data is the 2nd byte.
			// We don't check report type (it can only be output or feature)
			// we never implemented "feature" reports so it can't be feature
			// so assume "output" reports
			// this means set LED status
			// since it's the only one in the descriptor
			return USB_NO_MSG; // send nothing but call usbFunctionWrite
		}
		else // no data or do not understand data, ignore
		{
			return 0; // send nothing
		}
		default: // do not understand data, ignore
		return 0; // send nothing
	}
}

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{
	//We take the 2nd byte, which is the data byte
	led_state = data[1];
	return 1; // 1 byte read
}

void pressKey5(uint8_t modifiers, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, uint8_t keycode4, uint8_t keycode5)
{
	// construct the report, follow the standard format as described
	// this format is compatible with "boot protocol"
	report_buffer[1] = modifiers;
	report_buffer[2] = 0; // reserved
	report_buffer[3] = keycode1;
	report_buffer[4] = keycode2;
	report_buffer[5] = keycode3;
	report_buffer[6] = keycode4;
	report_buffer[7] = keycode5;
	report_buffer[0] = REPID_KEYBOARD;
	usbReportSend(REPSIZE_KEYBOARD);
}

void pressKey1(uint8_t modifiers, uint8_t keycode1)
{
	pressKey5(modifiers, keycode1, 0, 0, 0, 0);
}



void pressMultimediaKey(uint8_t key)
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

void pressSystemCtrlKey(uint8_t key)
{
	report_buffer[0] = REPID_SYSCTRLKEY;
	report_buffer[1] = key;
	usbReportSend(REPSIZE_SYSCTRLKEY);
	// immediate release
	report_buffer[0] = REPID_SYSCTRLKEY;
	report_buffer[1] = key;
	usbReportSend(REPSIZE_SYSCTRLKEY);
}

char isConnected()
{
	return usb_hasCommed;
}

/* ------------------------------------------------------------------------- */
int   main(void)
{
	uchar key=0, lastKey = 0;
	usbBegin();
	hardwareInit();
	
	while (1){/* main event loop */
		lastKey=key;
		key = getKey();
		
		if(isConnected()){
			if(key!=lastKey){
				switch(key){
					case 0:
					pressKey1(0, 0);
					break;
					case 1:
					pressKey1(0, KEYCODE_F);
					break;
					case 2:
					pressKey1(0, KEYCODE_ARROW_DOWN);
					break;
					case 3:
					pressKey1(0, KEYCODE_KPminus);
					break;
					case 4:
					pressKey1(0, KEYCODE_KPplus);
					break;
					case 5:
					pressKey1(0, KEYCODE_ARROW_RIGHT);
					break;
					case 6:
					pressKey1(0, KEYCODE_ARROW_LEFT);
					break;
					case 7:
					pressKey1(0, KEYCODE_ARROW_UP);
					break;
					case 8:
					pressKey1(0, KEYCODE_F5);
					break;
					case 9:
					pressMultimediaKey(MMKEY_VOL_DOWN);
					break;
					case 10:
					pressMultimediaKey(MMKEY_VOL_UP);
					break;
					case 11:
					pressMultimediaKey(MMKEY_MUTE);
					break;
				}
			}			
			set_high(LED1);
		}
		else{
			set_low(LED1);
		}
		
		if (key!=0){
			set_high(LED2);
		}
		else{
			set_low(LED2);
		}		
		
		usbPoll(); // this needs to be called at least once every 10 ms
	}
	
	return 0;
}

/* -------------------------------------------------------------------pressMultimediaKey(MMKEY_VOL_UP);------ */
