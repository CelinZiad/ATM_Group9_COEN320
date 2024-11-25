/*
 * Aircraft.cpp
 *
 *  Created on: Nov. 16, 2024
 *      Author: Elias
 */

#include "aircraft.h"
#include <iostream>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <cstring>
#include <signal.h>
#include <time.h>
#include <errno.h>
#define CODE_TIMER 1
#define COMMAND_RADAR_PING 2
using namespace std;


Aircraft::Aircraft(int id,int arrivalTime, double x,double y, double z,  double speedX,  double speedY, double speedZ): id(id),arrivalTime(arrivalTime), x(x), y(y), z(z), speedX(speedX), speedY(speedY), speedZ(speedZ){
this->left=false;
this->chid=0;
this->arrived=false;
//this->thread_id=-1;

}

// Aircraft* Aircraft::current_aircraft = nullptr;

void Aircraft::updatePosition(){
this->x+=this->speedX;
this->y+=this->speedY;
this->z+=this->speedZ;
if (this->x < 0 || this->x > 100000 ||this->y < 0 || this->y > 100000 ||this->z < 0 || this->z > 25000) {
	left=true;
        cout << "Aircraft " << this->id << " has left the airspace.\n";
        ChannelDestroy(this->chid);
        pthread_exit(nullptr);
        timer_delete(this->timer_id);
	}

}





void Aircraft::run() {


	//create new comm channel and store handle in chid
	chid = ChannelCreate(0);



	    //client connecting to itself
	    	int coid;
	    	coid= ConnectAttach(0,0,chid,0,0);
	    	if(coid==-1){
	    		std::cout<<"Failed attaching"<<std::endl;
	    	}
	    	struct sigevent sigev;
	    	SIGEV_PULSE_INIT(&sigev, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

	    	timer_t updateTimer;
	    	if (timer_create(CLOCK_MONOTONIC, &sigev, &updateTimer) == -1) {
	    		cout<<"EXIT";
	    		return;
	    	}

	    	struct itimerspec timerValue;
	    	timerValue.it_value.tv_sec = this->arrivalTime;
	    	timerValue.it_value.tv_nsec = 0;
	    	timerValue.it_interval.tv_sec = 1;
	    	timerValue.it_interval.tv_nsec = 0;

	    	timer_settime(updateTimer, 0, &timerValue, NULL);

	int rcvid;
	 PlaneCommandMessage msg;

	while(1){
				rcvid=MsgReceive(chid, &msg, sizeof(msg), NULL);
		        if (rcvid == 0) {
		           switch(msg.header.code){
		           case CODE_TIMER:
		        	   if(!arrived){
		        		   arrived=true;

		        		   break;
		        	   }else if(left)break;
		        	   updatePosition();
		        	   break;
		           default:
		        	   cout<<"Unknown code\n";
		        	   break;
		           }
		        } else{
		        	switch(msg.command){
		        	case COMMAND_RADAR_PING:
		        		AircraftStatus response;
		        		response.time=this->arrivalTime;
		        		response.id=this->id;
		        		response.x=this->x;
		        		response.y=this->y;
		        		response.z=this->z;
		        		response.speedX=this->speedX;
		        		response.speedY=this->speedY;
		        		response.speedZ=this->speedZ;

		        		MsgReply(rcvid, EOK, &response, sizeof(response));
		        		updatePosition();
		        		break;
		        	default:
		        		cout<<"Unknown command\n";
		        	}
		        }

	}
}


void* Aircraft::start(void *context) {
	auto a = (Aircraft*) context;
	a->run();
	return NULL;
}

//getters
int Aircraft::getArrivalTime() const {return arrivalTime;}

int Aircraft::getChid() const {return chid;}

int Aircraft::getId() const {return id;}

double Aircraft::getX() const {return x;}

double Aircraft::getY() const {return y;}

double Aircraft::getZ() const {return z;}

double Aircraft::getSpeedX() const {return speedX;}

double Aircraft::getSpeedY() const {return speedY;}

double Aircraft::getSpeedZ() const {return speedZ;}

// Setters
void Aircraft::setArrivalTime(int arrivalTime){this->arrivalTime=arrivalTime;}

void Aircraft::setChid(int chid) {this->chid = chid;}

void Aircraft::setId(int id) { this->id = id;}

void Aircraft::setX(double x) {this->x = x;}

void Aircraft::setY(double y) {this->y = y;}

void Aircraft::setZ(double z) {this->z = z;}

void Aircraft::setSpeedX(double speedX) {this->speedX = speedX;}

void Aircraft::setSpeedY(double speedY) {this->speedY = speedY;}

void Aircraft::setSpeedZ(double speedZ) {this->speedZ = speedZ;}

