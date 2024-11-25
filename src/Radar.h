/*
 * Radar.h
 *
 *  Created on: Nov. 16, 2024
 *      Author: Elias
 */

#ifndef RADAR_H
#define RADAR_H

#include "aircraft.h"
#include <vector>


class Radar{
public:
	 	Radar();
	    Radar(std::vector<Aircraft> aircrafts);
	    void setSystemChid(int chid) { systemChid = chid; }
	    AircraftStatus pingAircraft(Aircraft& ac);
	    std::vector<AircraftStatus> getAllAircraftStatus(std::vector<Aircraft>& aircraftss);
	    std::vector<Aircraft> aircrafts;
	    int systemChid=-1;

private:

};

#endif // RADAR_H
