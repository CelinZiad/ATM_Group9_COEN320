/*
 * Radar.cpp
 *
 *  Created on: Nov. 16, 2024
 *      Author: Elias
 */

#include "Radar.h"
#include <sys/neutrino.h>
#include <mutex>
#include <errno.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

#define COMMAND_RADAR_PING 2
std::mutex aircraft_mutex;



Radar::Radar(std::vector<Aircraft> aircrafts)
    : aircrafts(aircrafts), systemChid(-1) {}

AircraftStatus Radar::pingAircraft(Aircraft& ac) {


    int coid = ConnectAttach(0, 0, ac.getChid(), _NTO_SIDE_CHANNEL, 0);
    PlaneCommandMessage msg;
    msg.command = COMMAND_RADAR_PING;
    AircraftStatus response;

    MsgSend(coid, &msg, sizeof(msg), &response, sizeof(response));

    cout<<"AFTER MSGSEND!!\n";

    ConnectDetach(coid);
    return response;
}

std::vector<AircraftStatus> Radar::getAllAircraftStatus(std::vector<Aircraft>& aircraftss) {
	std::vector<AircraftStatus> responses;
	for (size_t i = 0; i < aircraftss.size(); i++) {
		// Connect to plane's message passing channel.
		int coid = ConnectAttach(0, 0, aircraftss[i].getChid(), _NTO_SIDE_CHANNEL,0);

		PlaneCommandMessage msg;
		msg.command= COMMAND_RADAR_PING;
		//msg.subtype = 0;
		AircraftStatus response;
		MsgSend(coid, &msg, sizeof(msg), &response, sizeof(response));

		ConnectDetach(coid);
		responses.push_back(response);


	}
	return responses;

}





