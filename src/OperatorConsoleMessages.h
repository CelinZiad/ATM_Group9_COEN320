#ifndef OPERATOR_CONSOLE_MESSAGES_H
#define OPERATOR_CONSOLE_MESSAGES_H

// Command codes used in the messages
#define OPCON_CONSOLE_COMMAND_GET_USER_COMMAND 100
#define OPCON_CONSOLE_COMMAND_ALERT 101
#define OPCON_USER_COMMAND_NO_COMMAND_AVAILABLE 200
#define OPCON_USER_COMMAND_DISPLAY_PLANE_INFO 201
#define OPCON_USER_COMMAND_SET_PLANE_VELOCITY 202
#define OPCON_USER_COMMAND_UPDATE_CONGESTION_VALUE 203
#define COMMAND_EXIT_THREAD 300

// Command strings
#define OPCON_COMMAND_STRING_SHOW_PLANE "show"
#define OPCON_COMMAND_STRING_SET_VELOCITY "set_velocity"
#define OPCON_COMMAND_STRING_UPDATE_CONGESTION "update_congestion"

// Other constants
struct Vec3 {
    double x;
    double y;
    double z;
};

struct OperatorConsoleCommandMessage {
    int systemCommandType;
    int plane1;
    int plane2;
    int collisionTimeSeconds;
};

struct OperatorConsoleResponseMessage {
    int userCommandType;
    int planeNumber;
    Vec3 newVelocity;
    int newCongestionValue;
};

#endif // OPERATOR_CONSOLE_MESSAGES_H
