/*
 * DataDisplay.cpp
 *
 *  Created on: Nov. 25, 2024
 *      Author: Elias
 */

#include "DataDisplay.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <vector>
#include <time.h>
#include <iostream>
using namespace std;
#define COMMAND_DISPLAY_AIRCRAFT 1111

DataDisplay::DataDisplay():chid(-1) {}

#include "DataDisplay.h"
#include <iostream>
#include <vector>
#include <sys/neutrino.h>
#include <errno.h>
using namespace std;

#define COMMAND_DISPLAY_AIRCRAFT 1111
#define COMMAND_DISPLAY_AUGMENTED_INFO 2222

void DataDisplay::listen() {
    int rcvid;
    struct AircraftStatusMessage {
        int command;
        size_t count;
        std::vector<AircraftStatus> aircrafts;
    } msg;

    struct DisplayCommandMessage {
        int command;
        AircraftStatus aircraft;
    } displayMsg;

    while (1) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) {
            perror("DataDisplay: MsgReceive failed");
            continue;
        }

        switch (msg.command) {
        case COMMAND_DISPLAY_AIRCRAFT:
            MsgReply(rcvid, EOK, NULL, 0); // Acknowledgement
            for (auto &aircraft : msg.aircrafts) {
                std::cout << "Aircraft ID: " << aircraft.id
                          << " | Position: (" << aircraft.x << ", " << aircraft.y << ", " << aircraft.z << ")"
                          << " | Speed: (" << aircraft.speedX << ", " << aircraft.speedY << ", " << aircraft.speedZ << ")\n";
            }
            break;

        case COMMAND_DISPLAY_AUGMENTED_INFO:
            memcpy(&displayMsg, &msg, sizeof(displayMsg));
            MsgReply(rcvid, EOK, NULL, 0);
            {
                AircraftStatus &aircraft = displayMsg.aircraft;
                std::cout << "Augmented Info - Aircraft ID: " << aircraft.id
                          << " | Position: (" << aircraft.x << ", " << aircraft.y << ", " << aircraft.z << ")"
                          << " | Speed: (" << aircraft.speedX << ", " << aircraft.speedY << ", " << aircraft.speedZ << ")\n";
            }
            break;

        default:
            MsgReply(rcvid, ENOTSUP, NULL, 0);
            break;
        }
    }
}

// ... existing methods ...


void DataDisplay::run(){
	chid=ChannelCreate(0);
	listen();

}


void* DataDisplay::start(void *context) {
	auto d = (DataDisplay*) context;
	d->run();
	return NULL;
}

int DataDisplay::getChid() const {return chid;}
