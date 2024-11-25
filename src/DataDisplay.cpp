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

void DataDisplay::listen(){
	int rcvid;
	struct AircraftStatusMessage {
	        int command;
	        size_t count;
	        std::vector<AircraftStatus> aircrafts;
	    } msg;
	    while(1){
	    	rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
	    	switch(msg.command){
	    		case COMMAND_DISPLAY_AIRCRAFT:
	    			MsgReply(rcvid,EOK,NULL,0);//acknowledgement

	    			for (auto &aircraft:msg.aircrafts){
	    				std::cout << "Aircraft ID: " << aircraft.id
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
