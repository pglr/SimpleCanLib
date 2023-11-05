#pragma once
#include "AppProfile.h"
#include "SimpleCAN.h"
#include "Arduino.h"

/* -------------------------------------------------------------------------- */
/*                                Notifications                               */
/* -------------------------------------------------------------------------- */
class AppNotificationsFromCAN
{
    public:
        virtual void ReceivedPing(const char* pText)=0;
        virtual void ReceivedPong(const char* pText)=0;
        virtual void ReceivedFloat(float Val)=0;
        virtual void ReceivedRequestInt()=0;
        virtual void ReceivedInt(int Val)=0;
};

/* -------------------------------------------------------------------------- */
/*                                   Broker                                   */
/* -------------------------------------------------------------------------- */
class AppDeviceBroker : public AppNotificationsFromCAN {
    public:
        AppDeviceBroker();

        void ReceivedPong(const char* pText);
        void ReceivedPing(const char* pText);
        void ReceivedFloat(const float Val);
        void ReceivedRequestInt();
        void ReceivedInt(int Val);

        int ReceivedID;
        bool RTR;
        float ReceivedFloatVal;
};

/* -------------------------------------------------------------------------- */
/*                                   Device                                   */
/* -------------------------------------------------------------------------- */
class AppDevice : public SimpleCANProfile
{
    public:
        AppDevice(SimpleCan* pCan, AppNotificationsFromCAN* _pRxCommands);

        void CANRequestInt(int DeviceID);
        void HandleCanMessage(const SimpleCanRxHeader rxHeader, const uint8_t *rxData);

    private:
        AppNotificationsFromCAN* pRxCommands;
};

/* -------------------------------------------------------------------------- */
/*                              User Application                              */
/* -------------------------------------------------------------------------- */
class UserApp 
{
    public:
        UserApp(AppDevice* canDevice, AppDeviceBroker* canBroker);
        void spinSome();
        void setID(int myID);

    private:
        AppDevice* _canDevice;
        AppDeviceBroker* _canBroker; 
        int _myID = 0;
};