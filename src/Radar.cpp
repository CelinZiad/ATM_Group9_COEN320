

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



Radar::Radar(std::vector<std::shared_ptr<Aircraft>>& aircrafts)
    : aircrafts(aircrafts), systemChid(-1) {}

AircraftStatus Radar::pingAircraft(std::shared_ptr<Aircraft> ac) {
    int coid = ConnectAttach(0, 0, ac->getChid(), _NTO_SIDE_CHANNEL, 0);
    CommandMessage msg;
    msg.command = COMMAND_RADAR_PING;
    AircraftStatus response;
    MsgSend(coid, &msg, sizeof(msg), &response, sizeof(response));
    ConnectDetach(coid);
    return response;
}

std::vector<AircraftStatus> Radar::getAllAircraftStatus() {
	 std::vector<AircraftStatus> responses;
	    for (size_t i = 0; i < aircrafts.size(); i++) {
	        // Skip aircraft that have not arrived or have left
	        if (!aircrafts[i]->hasArrived() || aircrafts[i]->hasLeft()) {
	            continue;
	        }

	        int coid = ConnectAttach(0, 0, aircrafts[i]->getChid(), _NTO_SIDE_CHANNEL, 0);
	        if (coid != -1) {
	            CommandMessage msg;
	            msg.command = COMMAND_RADAR_PING;
	            AircraftStatus response;
	            int status = MsgSend(coid, &msg, sizeof(msg), &response, sizeof(response));
	            if (status != -1) {
	                responses.push_back(response);
	            }
	            ConnectDetach(coid);
	        }
	    }
	    return responses;

}
