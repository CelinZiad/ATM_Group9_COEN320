#include "ComputerSystem.h"
#include "OperatorConsoleMessages.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <sys/siginfo.h>
#include <time.h>

#define COMMAND_DISPLAY_AIRCRAFT 1111
#define COMMAND_DISPLAY_AUGMENTED_INFO 2222

#define COMPUTER_SYSTEM_NUM_PERIODIC_TASKS 4
#define COLLISION_CHECK_TIMER 11
#define LOG_AIRSPACE_TO_CONSOLE_TIMER 12
#define OPERATOR_COMMAND_CHECK_TIMER 13
#define LOG_AIRSPACE_TO_FILE_TIMER 14

using namespace std;

ComputerSystem::ComputerSystem(std::vector<std::shared_ptr<Aircraft>>& aircrafts)
    : chid(-1), operatorChid(-1), displayChid(-1), aircrafts(aircrafts), radar(aircrafts) {}


void ComputerSystem::createTasks() {
    periodicTask periodicTasks[COMPUTER_SYSTEM_NUM_PERIODIC_TASKS] = {
        { COLLISION_CHECK_TIMER, 1 },
        { LOG_AIRSPACE_TO_CONSOLE_TIMER, 5 },
        { OPERATOR_COMMAND_CHECK_TIMER, 1 },
        { LOG_AIRSPACE_TO_FILE_TIMER, 30 }
    };

    if ((chid = ChannelCreate(0)) == -1) {
        std::cout << "channel creation failed. Exiting thread." << std::endl;
        return;
    }

    int coid;
    if ((coid = ConnectAttach(0, 0, chid, 0, 0)) == -1) {
        std::cout << "ComputerSystem: failed to attach to self. Exiting thread.";
        return;
    }

    for (int i = 0; i < COMPUTER_SYSTEM_NUM_PERIODIC_TASKS; i++) {
        periodicTask pt = periodicTasks[i];
        struct sigevent sigev;
        timer_t timer;
        SIGEV_PULSE_INIT(&sigev, coid, SIGEV_PULSE_PRIO_INHERIT, pt.timerCode, 0);
        if (timer_create(CLOCK_MONOTONIC, &sigev, &timer) == -1) {
            std::cout << "ComputerSystem: failed to initialize timer. Exiting thread.";
            return;
        }

        struct itimerspec timerValue;
        if (pt.timerCode == LOG_AIRSPACE_TO_CONSOLE_TIMER) {
            timerValue.it_value.tv_sec = 0;
            timerValue.it_value.tv_nsec = 1;
        } else {
            timerValue.it_value.tv_sec = pt.taskIntervalSeconds;
            timerValue.it_value.tv_nsec = 0;
        }
        timerValue.it_interval.tv_sec = pt.taskIntervalSeconds;
        timerValue.it_interval.tv_nsec = 0;

        timer_settime(timer, 0, &timerValue, NULL);
    }
}

void ComputerSystem::listen() {
    int rcvid;
    ComputerSystemMessage msg;
    while (1) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0) {
            // Pulse received
            switch (msg.header.code) {
            case COLLISION_CHECK_TIMER:
                 checkCollision();
                 break;
            case LOG_AIRSPACE_TO_CONSOLE_TIMER:
                logSystem();
                break;
            case OPERATOR_COMMAND_CHECK_TIMER:
            	checkOperatorConsole();
                break;

            default:
                std::cout << "ComputerSystem: received unknown pulse code: " << (int)msg.header.code << std::endl;
                break;
            }
        } else {

        }
    }
}

void ComputerSystem::checkOperatorConsole() {
    int coid = ConnectAttach(0, 0, operatorChid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        std::cerr << "ComputerSystem: Failed to connect to operator console." << std::endl;
        return;
    }

    OperatorConsoleCommandMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.systemCommandType = OPCON_CONSOLE_COMMAND_GET_USER_COMMAND;

    OperatorConsoleResponseMessage response;
    memset(&response, 0, sizeof(response));

    int status = MsgSend(coid, &msg, sizeof(msg), &response, sizeof(response));
    if (status == -1) {
        std::cerr << "ComputerSystem: Failed to send message to operator console." << std::endl;
        ConnectDetach(coid);
        return;
    }

    processOperatorCommand(response);

    ConnectDetach(coid);
}


void ComputerSystem::processOperatorCommand(const OperatorConsoleResponseMessage& msg) {
    switch (msg.userCommandType) {
    case OPCON_USER_COMMAND_NO_COMMAND_AVAILABLE:
        // No command, do nothing
        break;
    case OPCON_USER_COMMAND_DISPLAY_PLANE_INFO:
        // Request to display augmented info about a plane
        {
            int planeId = msg.planeNumber;
            // Find the aircraft with this ID
            auto it = std::find_if(aircrafts.begin(), aircrafts.end(),
                [planeId](const std::shared_ptr<Aircraft>& ac) {
                    return ac->getId() == planeId;
                });
            if (it != aircrafts.end()) {
                // Get the status
                AircraftStatus status = radar.pingAircraft(*it);

                // Send the status to DataDisplay
                int coid = ConnectAttach(0, 0, displayChid, _NTO_SIDE_CHANNEL, 0);
                if (coid == -1) {
                    std::cerr << "ComputerSystem: Failed to connect to data display." << std::endl;
                    return;
                }

                struct DisplayCommandMessage {
                    int command;
                    AircraftStatus aircraft;
                } displayMsg;
                displayMsg.command = COMMAND_DISPLAY_AUGMENTED_INFO;
                displayMsg.aircraft = status;

                MsgSend(coid, &displayMsg, sizeof(displayMsg), NULL, 0);
                ConnectDetach(coid);
            } else {
                std::cerr << "ComputerSystem: Aircraft with ID " << planeId << " not found." << std::endl;
            }
        }
        break;
    case OPCON_USER_COMMAND_SET_PLANE_VELOCITY:
        // Request to set new velocity for a plane
        {
            int planeId = msg.planeNumber;
            Vec3 newVelocity = msg.newVelocity;
            // Find the aircraft
            auto it = std::find_if(aircrafts.begin(), aircrafts.end(),
                [planeId](const std::shared_ptr<Aircraft>& ac) {
                    return ac->getId() == planeId;
                });
            if (it != aircrafts.end()) {
                // Send command to aircraft to change velocity
                CommunicationSystem comms(aircrafts);
                AircraftVelocity velocity;
                velocity.speedX = newVelocity.x;
                velocity.speedY = newVelocity.y;
                velocity.speedZ = newVelocity.z;
                comms.send(*it, velocity);
            } else {
                std::cerr << "ComputerSystem: Aircraft with ID " << planeId << " not found." << std::endl;
            }
        }
        break;
    default:
        std::cerr << "ComputerSystem: Unknown operator command type " << msg.userCommandType << std::endl;
        break;
    }
}

void ComputerSystem::logSystem() {
    aircraftsStatus = radar.getAllAircraftStatus();
    struct AircraftStatusMessage {
        int command;
        size_t count;
        std::vector<AircraftStatus> aircrafts;
    } msg;
    msg.command=COMMAND_DISPLAY_AIRCRAFT;
	msg.count = aircraftsStatus.size();
	for (size_t i = 0; i < aircraftsStatus.size(); i++){
		msg.aircrafts.push_back(aircraftsStatus[i]);
	}

	int coid = ConnectAttach(0, 0, displayChid, _NTO_SIDE_CHANNEL, 0);
	 if (coid == -1) {
	        perror("ConnectAttach failed");
	        return;
	    }
	int msgSnd=MsgSend(coid, &msg, sizeof(msg), NULL, 0) ;

	if (msgSnd == -1) {
	        perror("MsgSend to DataDisplay failed");
	    }
}

void ComputerSystem::run() {
    createTasks();
    listen();
}

void ComputerSystem::checkCollision() {
    for (size_t i = 0; i < aircrafts.size() - 1; i++) {
        for (size_t j = i + 1; j < aircrafts.size(); j++) {
            checkSeparation(aircrafts[i], aircrafts[j]);
        }
    }
}

void ComputerSystem::checkSeparation(const std::shared_ptr<Aircraft>& ac1, const std::shared_ptr<Aircraft>& ac2) {
    int verticalLimit = 1000;
    int horizontalLimit = 3000;

    int x1max = ac1->getX() + horizontalLimit;
    int x1min = ac1->getX() - horizontalLimit;
    int y1max = ac1->getY() + horizontalLimit;
    int y1min = ac1->getY() - horizontalLimit;
    int z1max = ac1->getZ() + verticalLimit;
    int z1min = ac1->getZ() - verticalLimit;

    int x2max = ac2->getX() + horizontalLimit;
    int x2min = ac2->getX() - horizontalLimit;
    int y2max = ac2->getY() + horizontalLimit;
    int y2min = ac2->getY() - horizontalLimit;
    int z2max = ac2->getZ() + verticalLimit;
    int z2min = ac2->getZ() - verticalLimit;

    if ((x1max >= x2min && x2max >= x1min) && (y1max >= y2min && y2max >= y1min)&& (z1max >= z2min && z2max >= z1min)) {
        std::cout << "Collision detected between Aircraft " << ac1->getId() << " and Aircraft " << ac2->getId() << std::endl;
    }
}

void* ComputerSystem::start(void* context) {
    auto cs = static_cast<ComputerSystem*>(context);
    cs->run();
    return NULL;
}

int ComputerSystem::getChid() const { return chid; }

void ComputerSystem::setOperatorChid(int id) { operatorChid = id; }

void ComputerSystem::setDisplayChid(int id) { displayChid = id; }
