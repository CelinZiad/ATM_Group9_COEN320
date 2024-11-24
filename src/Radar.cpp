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



Radar::Radar(std::vector<Aircraft>& aircrafts)
    : aircrafts(aircrafts), systemChid(-1) {}

AircraftStatus Radar::pingAircraft(Aircraft &ac) {
    AircraftStatus status;

cout<<ac.getChid();
    int attempts = 5; // Retry a few times
        while (ac.getChid() == 0 && attempts > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for chid
            attempts--;
        }




    int coid = ConnectAttach(0, 0, ac.getChid(), _NTO_SIDE_CHANNEL, 0);


    struct _pulse msg;
    msg.type = COMMAND_RADAR_PING;
    msg.subtype = 0;

    MsgSend(coid, &msg, sizeof(msg), &status, sizeof(status));



    ConnectDetach(coid);
    return status;
}

std::vector<AircraftStatus> Radar::getAllAircraftStatus(std::vector<Aircraft> aircraftss) {
	std::vector<AircraftStatus> allStatus;
	//std::lock_guard<std::mutex> lock(aircraft_mutex);


    for (auto& aircraft : aircraftss) {
    	cout<<"WE OUT!!\n";
        allStatus.push_back(pingAircraft(aircraft));
    }

    return allStatus;
}





