#pragma once
/* -------------------------------------------------------------------------- */
/*                                 Message IDs                                */
/* -------------------------------------------------------------------------- */
#define CANID_PP_PING    1       // Message is "Ping"
#define CANID_PP_PONG    2       // Message is "Pong"
#define CANID_PP_FLOAT   3       // Message is a floating point value
#define CANID_PP_RTRINT  4       // Request an int from the client


/* -------------------------------------------------------------------------- */
/*                                   MACROS                                   */
/* -------------------------------------------------------------------------- */
// maybe move them to the SimpleCan Layer as well ? or abstract AppClass
#define PP_MAKE_CAN_ID(Device, Message)     ((Device<<8) | Message) 
#define PP_GET_MESSAGE_ID(CanID)            (CanID & 0xff)
#define PP_GET_DEVICE_ID(CanID)             (CanID>>8)

// The actually used CAN ID consists of 8 bits as defined by the CANID_PP_XXX message IDs,
// or'ed together with an 8 bit device identifier. This is required to avoid that two devices will try 
// bus arbitration with identical CAN IDs, which is not allowed. For the demo program,
// the device IDs are generated randomly at start up.
