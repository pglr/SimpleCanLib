/*

    Demo code for portable CAN bus library
   
	(c) 2022 Christian Schmidmer, use is subject to MIT license
*/
#include <Arduino.h>
#include <math.h>
#include "SimpleCAN.h"
//#include "PingPongCANProfile.h"
#include "App.h"
#include "AppProfile.h"

// Create a Broker, create a Device, setup device, link broker functions to device (=callbacks), overhand device to Broker ? to make the send calls possible | 
// by linking the broker to the device, also set the reference to the device inside the Broker? 
// not the Broker needs the reference, but rather the App --> create a App Class where the actual function code is living, have a loop method for running the app.

// App has a init function(id), creates both broker and device, links them, inits both and then handles the actual app from within the loop()
// AppClass DemoApp(&Device, &Broker)
// DemoApp.init()
// ...
// DemoApp.loop()
// maybe DemoApp.Device->can1.loop() as extra ? why is this separate anyways ?


// Instatiation of the class which receives messages from the CAN bus.
// This class depends on your application!
AppDeviceBroker CANBroker;
// The actual CAN bus class, which handles all communication.
AppDevice CANDevice(CreateCanLib(GPIO_NUM_26,GPIO_NUM_25), &CANBroker); //Todo: have a link method
UserApp app(&CANDevice, &CANBroker);

int MyDeviceID=0;

void setup() 
{
	Serial.begin(BAUDRATE);
	delay(3000);

	Serial.println("Started");

	// Create a random device ID to avoid(make it less likely to have) conflicts on the CAN bus.
	MyDeviceID = random(1, 255); // Todo: move this to a method _setCANiD(int _canID)
	app.setID(MyDeviceID);
	CANDevice.Init(); // maybe init with id here? overload inits!

	// Set bus termination on/off (may not be available on all platforms).
	if (CAN_OK!=CANDevice.Can1->SetBusTermination(true))
		Serial.println("Setting CAN bus termination via software not possible");
	Serial.printf("Setup done, random device ID is %d\n", MyDeviceID);
}

void loop()
{
	app.spinSome(); // make all the can-magic do its stuff behind the curtain
}
