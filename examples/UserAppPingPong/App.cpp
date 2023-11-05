#include "App.h"

// Broker class which handles information received from CAN bus.
// These functions are called from within CANPingPong::Can1->Loop() whenever there are new CAN messages in the queue.
// CAN IDs you want to react to must be specified in the CAN profile class (PingPongNotificationsFromCAN in this example).
// CANPingPong::Can1->Loop()   ->  CANPingPong::HandleCanMessage() ->  RxFromCAN::ReceivedXxxx()
// The reason for having this broker class is that it allows us to completely separate the profile definition from any code
// which is related to e.g. motor control. this means, a single header file is sufficient to define the profile and the same header file
// can be used on both sides of the CAN bus without having to include all the stuff which may be required on one side only.

/* -------------------------------------------------------------------------- */
/*                                   Broker                                   */
/* -------------------------------------------------------------------------- */
AppDeviceBroker::AppDeviceBroker(): 
    ReceivedID(-1), 
    RTR(false), 
    ReceivedFloatVal(1.0f){
}

void AppDeviceBroker::ReceivedPong(const char* pText) {
    Serial.printf("Received: %s\n", pText);
    ReceivedID = CANID_PP_PONG;
}

void AppDeviceBroker::ReceivedPing(const char* pText) {
    Serial.printf("Received: %s\n", pText);
    ReceivedID = CANID_PP_PING;
}

void AppDeviceBroker::ReceivedFloat(const float Val) {
    Serial.printf("Rcvd float: %.3f\n", Val);
    ReceivedFloatVal = Val;
    ReceivedID = CANID_PP_FLOAT;
}

void AppDeviceBroker::ReceivedRequestInt() {
    Serial.printf("Received: RTR\n");
    ReceivedID = CANID_PP_RTRINT;
    RTR = true;
}

void AppDeviceBroker::ReceivedInt(int Val) {
    Serial.printf("Rcvd int: %d\n", Val);
    ReceivedID = CANID_PP_RTRINT;
}

/* -------------------------------------------------------------------------- */
/*                                   Device                                   */
/* -------------------------------------------------------------------------- */
AppDevice::AppDevice(SimpleCan* pCan, AppNotificationsFromCAN* _pRxCommands) : SimpleCANProfile(pCan) {
    pRxCommands = _pRxCommands;
}

void AppDevice::CANRequestInt(int DeviceID) {
    Can1->RequestMessage(2, PP_MAKE_CAN_ID(DeviceID, CANID_PP_RTRINT));
}

void AppDevice::HandleCanMessage(const SimpleCanRxHeader rxHeader, const uint8_t *rxData) {
    // Serial.println("@");
    #ifdef _STM32_DEF_  
        digitalToggle(LED_BUILTIN);
    #endif

    #define MAX_STRLEN  16
    
    char Str[MAX_STRLEN];
    float Val=0;
    switch(PP_GET_MESSAGE_ID(rxHeader.Identifier))
    {
        case CANID_PP_PING:
            CANGetString(rxData, Str, min(MAX_STRLEN-1, (int)rxHeader.DataLength));
            pRxCommands->ReceivedPing(Str);
            break;
        case CANID_PP_PONG:
            CANGetString(rxData, Str, min(MAX_STRLEN, (int)rxHeader.DataLength));
            pRxCommands->ReceivedPong(Str);
            break;
        case CANID_PP_FLOAT:
            Val = CANGetFloat(rxData);
            pRxCommands->ReceivedFloat(Val);
            break;
        case CANID_PP_RTRINT:
            if (rxHeader.RxFrameType==CAN_REMOTE_FRAME)
                pRxCommands->ReceivedRequestInt();
            else
            {
                int ValI = CANGetInt(rxData);
                pRxCommands->ReceivedInt(ValI);
            }
            break;
        default:
            Serial.printf("y:0x%x DLC=0x%x ", rxHeader.Identifier, rxHeader.DataLength);
    } 
}

/* -------------------------------------------------------------------------- */
/*                              User Application                              */
/* -------------------------------------------------------------------------- */
UserApp::UserApp(AppDevice* canDevice, AppDeviceBroker* canBroker)
{
    _canDevice = canDevice;
    _canBroker = canBroker; 
}

void UserApp::setID(int myID){
    _myID = myID;
}

void UserApp::spinSome(){
    static uint32_t LastAction=millis();
	static uint32_t LastFloatAction=millis();
	static uint32_t LastRTR=millis();

	int RandWait = random(-500, 1000);

	// Test of regular messages:
	// What is sent next to the CAN bus depends on what was received last. 
	// When a PING was received, send a PONG and vice versa.
	// To get the whole thing started, a PONG is sent every 5s without having received anything.
	// This is just for testing. Usually you would invoke actions for incomming messages
	// directly in the broker class.
	if ((_canBroker->ReceivedID==CANID_PP_PING && LastAction+1000<millis()) || (LastAction+RandWait+5000<millis()) )
	{
		Serial.println("Sending Pong");
		_canDevice->CANSendText("Pong", PP_MAKE_CAN_ID(_myID, CANID_PP_PONG));
		LastAction=millis();

		// Make sure we don't react twice to the same message.
		_canBroker->ReceivedID = -1;		
	}
	else if (_canBroker->ReceivedID==CANID_PP_PONG && LastAction+RandWait+1000<millis())
	{
		Serial.println("Sending Ping");
		_canDevice->CANSendText("Ping", PP_MAKE_CAN_ID(_myID, CANID_PP_PING));
		LastAction=millis();

		// Make sure we don't react twice to the same message.
		_canBroker->ReceivedID = -1;		
	}
	else if (_canBroker->ReceivedID==CANID_PP_RTRINT && _canBroker->RTR)
	{
		// React to an RTR request message. The reply should be the number "1234". If something else is 
		// received, check the byte order used by the devices!
		_canBroker->RTR=false;
		_canDevice->CANSendInt(1234, PP_MAKE_CAN_ID(_myID, CANID_PP_RTRINT));

		// Make sure we don't react twice to the same message.
		_canBroker->ReceivedID = -1;		
	}


	// Every 3s just send a float value. This can be used to check if all devices on 
	// the bus use the same floating point number representation and byte order.
	if (LastFloatAction+RandWait+3000<millis() )
	{
		float NewVal = _canBroker->ReceivedFloatVal*2.5;
		if (NewVal==0) NewVal=1.0f;
		if (NewVal>1000000) NewVal=-1.0;
		if(NewVal<-1000000) NewVal = 1.0;

		Serial.printf("Sending: %.3f\n", NewVal);
		_canDevice->CANSendFloat(NewVal, PP_MAKE_CAN_ID(_myID, CANID_PP_FLOAT));
		LastFloatAction=millis();
	}

	// Test of RTR messages
	// Every 5s request an int value. Response should be the number 1234 in binary form.
	if (LastRTR+RandWait+5000<millis() )
	{
		Serial.printf("Request int\n");
		_canDevice->CANRequestInt(_myID);
		LastRTR=millis();
	}

	// Get some statistics on bus errors.
	static int LastTxErrors=0;
	static int LastRxErrors=0;
	static int LastOtherErrors = 0;
	static uint32_t LastStatus = 0;
	uint32_t Status = 0;
	char StatusStr[MAX_STATUS_STR_LEN]={0};

	_canDevice->Can1->GetStatus(&Status, StatusStr);
	if (_canDevice->Can1->GetTxErrors()!=LastTxErrors || _canDevice->Can1->GetRxErrors()!=LastRxErrors || _canDevice->Can1->GetOtherErrors()!=LastOtherErrors || LastStatus!=Status)
	{
		LastTxErrors = _canDevice->Can1->GetTxErrors();
		LastRxErrors = _canDevice->Can1->GetRxErrors();
		LastOtherErrors = _canDevice->Can1->GetOtherErrors();
		LastStatus = Status;

		Serial.printf("New Status=%s, RxErrors=%d, TxErrors=%d, Other=%d\n", StatusStr, LastTxErrors, LastRxErrors, LastOtherErrors);
	}

	// Update message queues.
	_canDevice->Can1->Loop();
}

