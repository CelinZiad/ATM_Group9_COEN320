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
#define COMPUTER_SYSTEM_NUM_PERIODIC_TASKS 4
#define AIRSPACE_VIOLATION_CONSTRAINT_TIMER 11
#define LOG_AIRSPACE_TO_CONSOLE_TIMER 12
#define OPERATOR_COMMAND_CHECK_TIMER 13
#define LOG_AIRSPACE_TO_FILE_TIMER 14
using namespace std;




ComputerSystem::ComputerSystem(std::vector<Aircraft> aircrafts) : chid(-1), operatorChid(-1), displayChid(-1),aircrafts(std::move(aircrafts)),  radar(this->aircrafts){
}
void ComputerSystem::setAircrafts(std::vector<Aircraft> aircraftss) {
	aircrafts = std::move(aircraftss);
}
int ComputerSystem::getChid() const {return chid;}

void ComputerSystem::setOperatorChid(int id) {operatorChid = id;}

void ComputerSystem::setDisplayChid(int id) {displayChid = id;}

void ComputerSystem::initialize() {
   chid = ChannelCreate(0);
    if (chid == -1) {
        perror("Failed to create channel");
        return;
    }
    radar.setSystemChid(chid);
}
void ComputerSystem::createTasks(){
	periodicTask periodicTasks[COMPUTER_SYSTEM_NUM_PERIODIC_TASKS] = { {
	AIRSPACE_VIOLATION_CONSTRAINT_TIMER, 1 },
			{ LOG_AIRSPACE_TO_CONSOLE_TIMER, 5 }, {
			OPERATOR_COMMAND_CHECK_TIMER, 1 },
			{ LOG_AIRSPACE_TO_FILE_TIMER, 30 } };

	if ((chid = ChannelCreate(0)) == -1) {
		std::cout << "channel creation failed. Exiting thread." << std::endl;
		return;
	}

	// Open a client to our own connection to be used for timer pulses and store the handle in coid.
	int coid;
	if ((coid = ConnectAttach(0, 0, chid, 0, 0)) == -1) {
		std::cout
				<< "ComputerSystem: failed to attach to self. Exiting thread.";
		return;
	}

	// For each periodic task, initialize a timer with the associated code and interval.
	for (int i = 0; i < COMPUTER_SYSTEM_NUM_PERIODIC_TASKS; i++) {
		periodicTask pt = periodicTasks[i];
		struct sigevent sigev;
		timer_t timer;
		SIGEV_PULSE_INIT(&sigev, coid, SIGEV_PULSE_PRIO_INHERIT, pt.timerCode,0);
		if (timer_create(CLOCK_MONOTONIC, &sigev, &timer) == -1) {
			std::cout
					<< "ComputerSystem: failed to initialize timer. Exiting thread.";
			return;
		}

		struct itimerspec timerValue;
		timerValue.it_value.tv_sec = pt.taskIntervalSeconds;
		timerValue.it_value.tv_nsec = 0;
		timerValue.it_interval.tv_sec = pt.taskIntervalSeconds;
		timerValue.it_interval.tv_nsec = 0;

		// Start the timer.
		timer_settime(timer, 0, &timerValue, NULL);
	}

}

void ComputerSystem::listen(){
	int rcvid;
	ComputerSystemMessage msg;
	while (1) {
		// Wait for any type of message.
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {
			// Handle internal switches from the pulses of the various timers.
			switch (msg.header.code) {
			case LOG_AIRSPACE_TO_CONSOLE_TIMER:
				logSystem();
				break;
			case AIRSPACE_VIOLATION_CONSTRAINT_TIMER:
				//violationCheck();
				break;
			case OPERATOR_COMMAND_CHECK_TIMER:
				//opConCheck();
				break;
			case LOG_AIRSPACE_TO_FILE_TIMER:
				//logSystem(true);
				break;
			default:
				std::cout
						<< "ComputerSystem: received pulse with unknown code: "
						<< msg.header.code << " and unknown command: "
						<< msg.command << std::endl;
				break;
			}
		}else{
			/*switch (msg.command) {
			case COMMAND_OPERATOR_REQUEST:
				//MsgSend(operatorChid, &msg, sizeof(msg), NULL, 0);
				break;
			case COMMAND_EXIT_THREAD:
				// Required to allow all threads to gracefully terminate when the program is terminating
				cout << "ComputerSystem: " << "Received EXIT command" << endl;
				MsgReply(rcvid, EOK, NULL, 0);
				return;
			default:
				std::cout << "ComputerSystem: " << "received unknown command "
						<< msg.command << std::endl;
				MsgError(rcvid, ENOSYS);
				break;*/
			}

		}
}

void ComputerSystem::logSystem(){

	this->aircraftsStatus=radar.getAllAircraftStatus(aircrafts);
	size_t aircraftCount = aircraftsStatus.size();
	for (size_t i = 0; i < aircraftCount; i++){
		 cout << "Aircraft ID: " << aircraftsStatus[i].id
		     << " | Position: (" << aircraftsStatus[i].x << ", " << aircraftsStatus[i].y << ", " << aircraftsStatus[i].z << ")"
		     << " | Speed: (" << aircraftsStatus[i].speedX << ", " << aircraftsStatus[i].speedY << ", " << aircraftsStatus[i].speedZ << ")\n";
	}
}

void ComputerSystem::run(){
	createTasks();

	listen();
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






