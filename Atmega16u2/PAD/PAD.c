/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the RelayBoard program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "PAD.h"


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		PAD_Task();
	  	USB_USBTask();
	}
}

//Configuration de la carte
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	USB_Init();
	Serial_Init(SERIAL_SPEED,false);
}


//Envoie les données reçu via la com série vers le PC en USB
void toPC(void)
{	//On test si le buffer série a reçu quelque chose
	if(Serial_IsCharReceived())
	{
		Endpoint_SelectEndpoint(PAD_IN_ENDPOINT);

		uint16_t message = Serial_ReceiveByte();

		if (Endpoint_IsReadWriteAllowed())
		{
			//On écrit 1 octet sur l'endpoint IN
			Endpoint_Write_Stream_LE(&message, 1, NULL);


			Endpoint_ClearIN();
		}
	}
}


void EVENT_USB_Device_ConfigurationChanged(void)
{

	Endpoint_ConfigureEndpoint(PAD_OUT_ENDPOINT, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(PAD_IN_ENDPOINT, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);

}

/** Récupère les informations envoyées par le PC et les réenvoient vers le 328P*/
void fromPC(void)
{
	//On sélectionne l'endpoint lié à la com des LEDs
	Endpoint_SelectEndpoint(PAD_OUT_ENDPOINT);

	//On test si l'endpoint a envoyé quelque chose
	if (Endpoint_IsOUTReceived())
	{
		//On vérifie si on peut lire dans l'endpoint
		if (Endpoint_IsReadWriteAllowed())
		{
			//On envoi l'octet
			Serial_SendByte(Endpoint_Read_8());
		}

		//On relâche le endpoint
		Endpoint_ClearOUT();
	}
}

/** Fonction qui gère les fonctions d'envoi et de reception*/
void PAD_Task(void)
{
	/*Les transmissions n'ont lieu que si l'appareil est configuré*/
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;
	//On scrute l'endpoint OUT
	fromPC();

	//On scrute l'endpoint IN
	toPC();
}
