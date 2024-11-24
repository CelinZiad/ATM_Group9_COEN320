/*
 * ComputerSystem.cpp
 *
 *  Created on: Nov. 19, 2024
 *      Author: Elias
 */

#include "ComputerSystem.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <sys/siginfo.h>
#include <time.h>
#include <iostream>
using namespace std;




ComputerSystem::ComputerSystem(std::vector<Aircraft> aircrafts) : chid(-1), operatorChid(-1), displayChid(-1) {
	this->aircrafts=aircrafts;

}
void ComputerSystem::setAircrafts(std::vector<Aircraft> aircraftss) {
	aircrafts = std::move(aircraftss);
}
int ComputerSystem::getChid() const {
	return chid;
}

void ComputerSystem::setOperatorChid(int id) {
	operatorChid = id;
}

void ComputerSystem::setDisplayChid(int id) {
	displayChid = id;
}

void ComputerSystem::initialize() {

    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("Failed to create channel");
        return;
    }



    radar.setSystemChid(chid);
}

void ComputerSystem::run() {
    initialize();

    while (true) {
    	cout<<"Inside computer System\n";

        std::vector<AircraftStatus> allStatus = radar.getAllAircraftStatus(aircrafts);
        cout<<"After Radar";

        for (const auto& status : allStatus) {
            for (auto& aircraft : aircrafts) {
                if (aircraft.getId() == status.id) {
                    aircraft.setX(status.x);
                    aircraft.setY(status.y);
                    aircraft.setZ(status.z);
                    aircraft.setSpeedX(status.speedX);
                    aircraft.setSpeedY(status.speedY);
                    aircraft.setSpeedZ(status.speedZ) ;
                    break;
                }
            }
        }
        cout << "\nCurrent Airspace Status:\n";
              for (const auto& status : allStatus) {
                  cout << "Aircraft ID: " << status.id
                            << " | Position: (" << status.x << ", " << status.y << ", " << status.z << ")"
                            << " | Speed: (" << status.speedX << ", " << status.speedY << ", " << status.speedZ << ")\n";
              }
        checkViolation();
    }
}


void ComputerSystem::checkViolation(){
	for(size_t i=0;i<aircrafts.size()-1;i++){
		for(size_t j=i+1;j<aircrafts.size();j++){
			checkSeparation(aircrafts[i],aircrafts[j]);
		}
	}
}
void ComputerSystem::checkSeparation(Aircraft ac1,Aircraft ac2){
	int verticalLimit=1000;
	int horizontalLimit=3000;

	int x1max=ac1.getX()+horizontalLimit;
	int x1min=ac1.getX()-horizontalLimit;
	int y1max=ac1.getY()+horizontalLimit;
	int y1min=ac1.getY()-horizontalLimit;
	int z1max=ac1.getZ()+verticalLimit;
	int z1min=ac1.getZ()-verticalLimit;

	int x2max=ac2.getX()+horizontalLimit;
	int x2min=ac2.getX()-horizontalLimit;
	int y2max=ac2.getY()+horizontalLimit;
	int y2min=ac2.getY()-horizontalLimit;
	int z2max=ac2.getZ()+verticalLimit;
	int z2min=ac2.getZ()-verticalLimit;

	if ((x1max < x2min && x2max < x1min) && (y1max < y2min && y2max < y1min)&& (z1max < z2min && z2max < z1min)){
		std::cout<<"Collision";
	}
}
void* ComputerSystem::start(void *context) {
	auto cs = (ComputerSystem*) context;
	cs->run();
	return NULL;
}






