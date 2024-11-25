#include "ComputerSystem.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <sys/siginfo.h>
#include <time.h>

#define COMMAND_DISPLAY_AIRCRAFT 1111

#define COMPUTER_SYSTEM_NUM_PERIODIC_TASKS 4
#define AIRSPACE_VIOLATION_CONSTRAINT_TIMER 11
#define LOG_AIRSPACE_TO_CONSOLE_TIMER 12
#define OPERATOR_COMMAND_CHECK_TIMER 13
#define LOG_AIRSPACE_TO_FILE_TIMER 14

using namespace std;

ComputerSystem::ComputerSystem(std::vector<std::shared_ptr<Aircraft>>& aircrafts)
    : chid(-1), operatorChid(-1), displayChid(-1), aircrafts(aircrafts), radar(aircrafts) {}


void ComputerSystem::createTasks() {
    periodicTask periodicTasks[COMPUTER_SYSTEM_NUM_PERIODIC_TASKS] = {
        { AIRSPACE_VIOLATION_CONSTRAINT_TIMER, 1 },
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
            timerValue.it_value.tv_nsec = 1; // Minimal non-zero value
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
            case LOG_AIRSPACE_TO_CONSOLE_TIMER:
                logSystem();
                break;
            case AIRSPACE_VIOLATION_CONSTRAINT_TIMER:
                checkViolation();
                break;
            case OPERATOR_COMMAND_CHECK_TIMER:
                // Implement operator command check if needed
                break;
            case LOG_AIRSPACE_TO_FILE_TIMER:
                // Implement logging to file if needed
                break;
            default:
                std::cout << "ComputerSystem: received unknown pulse code: " << (int)msg.header.code << std::endl;
                break;
            }
        } else {
            // Handle other messages if needed
        }
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

void ComputerSystem::checkViolation() {
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

    if ((x1max < x2min && x2max < x1min) && (y1max < y2min && y2max < y1min) && (z1max < z2min && z2max < z1min)) {
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
