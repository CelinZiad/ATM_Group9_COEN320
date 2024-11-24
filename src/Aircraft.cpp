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

#define COMMAND_RADAR_PING 1
using namespace std;


Aircraft::Aircraft(int id,int arrivalTime, double x,double y, double z,  double speedX,  double speedY, double speedZ): id(id),arrivalTime(arrivalTime), x(x), y(y), z(z), speedX(speedX), speedY(speedY), speedZ(speedZ){
this->left=false;
this->chid=0;
this->arrived=false;
//this->thread_id=-1;

}

Aircraft* Aircraft::current_aircraft = nullptr;

void Aircraft::updatePosition(){
this->x+=this->speedX;
this->y+=this->speedY;
this->z+=this->speedZ;

std::cout << "Aircraft " << this->id << " updated position to ("
           << this->x << ", " << this->y << ", " << this->z << ")\n";
if (this->x < 0 || this->x > 100000 ||this->y < 0 || this->y > 100000 ||this->z < 0 || this->z > 25000) {
	left=true;
        cout << "Aircraft " << this->id << " has left the airspace.\n";
	}

}



void Aircraft::signal_handler(int sig, siginfo_t* si, void* uc) {

    if (current_aircraft) {
        current_aircraft->updatePosition();
    }
}


void Aircraft::run() {

	 current_aircraft = this;
	//create new comm channel and store handle in chid
	chid = ChannelCreate(0);


	    //client connecting to itself
	    	int coid;
	    	coid= ConnectAttach(0,0,chid,0,0);
	    	if(coid==-1){
	    		std::cout<<"Failed attaching"<<std::endl;
	    	}


	    // Set up signal handler
	    struct sigaction sa;
	        sa.sa_flags = SA_SIGINFO;
	        sa.sa_sigaction = signal_handler;
	        sigemptyset(&sa.sa_mask);

	        // Use a unique signal for each aircraft
	        int signal_num = SIGRTMIN + (id % (SIGRTMAX - SIGRTMIN));
	        if (sigaction(signal_num, &sa, NULL) == -1) {
	            perror("sigaction failed");
	            pthread_exit(nullptr);
	        }

	        // Set up timer
	        struct sigevent sev;
	        memset(&sev, 0, sizeof(sev));
	        sev.sigev_notify = SIGEV_SIGNAL;
	        sev.sigev_signo = signal_num;
	        sev.sigev_value.sival_ptr = this;

	        if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
	            perror("timer_create failed");
	            pthread_exit(nullptr);
	        }

	        // Configure and start timer (1 second interval)
	        struct itimerspec its;
	        its.it_value.tv_sec = this->arrivalTime;
	        its.it_value.tv_nsec = 0;
	        its.it_interval.tv_sec = 1;
	        its.it_interval.tv_nsec = 0;

	        if (timer_settime(timer_id, 0, &its, NULL) == -1) {
	            perror("timer_settime failed");
	            pthread_exit(nullptr);
	        }



	int rcvid;
	while(1){

		struct _pulse msg;

		rcvid=MsgReceive(chid, &msg, sizeof(msg), NULL);


		if (rcvid == -1) {
		            if (errno == EINTR) {
		                // Interrupted by a signal (position update), continue
		                continue;
		            } else {
		                perror("MsgReceive failed");
		                break;
		            }
		        }

		        if (rcvid == 0) {
		           if(!arrived){
		        	   arrived=true;
		           }else if(left){
		        	   break;
		           }
		        }
		        //for future might be error here --Elias
		        if (msg.type == _IO_CONNECT) {
		            MsgReply(rcvid, EOK, NULL, 0);
		            continue;
		        }
		        //and here --Elias
		        if (msg.type == COMMAND_RADAR_PING) {
		            // Respond with current status
		        	cout<<"MSG RECEIVED FRL";
		            AircraftStatus status;
		            status.id = this->id;
		            status.x = this->x;
		            status.y = this->y;
		            status.z = this->z;
		            status.speedX = this->speedX;
		            status.speedY = this->speedY;
		            status.speedZ = this->speedZ;

		            MsgReply(rcvid, EOK, &status, sizeof(status));
		        }


	}
	timer_delete(timer_id);
	ChannelDestroy(chid);
	    pthread_exit(nullptr);

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




















/*
void update_position(int sig, siginfo_t* si, void* uc) {
    Aircraft* aircraft = static_cast<Aircraft*>(si->si_value.sival_ptr);

    // Update position
    aircraft->x += aircraft->speedX;
    aircraft->y += aircraft->speedY;
    aircraft->z += aircraft->speedZ;

    // Log the update
    cout << "Aircraft " << aircraft->id << " updated position to ("
              << aircraft->x << ", " << aircraft->y << ", " << aircraft->z << ")\n";

    // Check if the aircraft is still within the airspace bounds
    if (aircraft->x < 0 || aircraft->x > 100000 ||
        aircraft->y < 0 || aircraft->y > 100000 ||
        aircraft->z < 0 || aircraft->z > 25000) {
        cout << "Aircraft " << aircraft->id << " has left the airspace.\n";

        // Clean up timer
        timer_delete(aircraft->timer_id);

        // Clean up channel
        ChannelDestroy(aircraft->channel_id);

        // Terminate the aircraft thread
        pthread_exit(nullptr);
    }
}

void* aircraft_thread(void* arg) {
    Aircraft* aircraft = static_cast<Aircraft*>(arg);

    // Create a channel to receive messages
    int chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        pthread_exit(nullptr);
    }

    // Store the channel ID in the aircraft struct
    aircraft->channel_id = chid;

    // Set up the signal handler for position updates
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = update_position;
    sigemptyset(&sa.sa_mask);

    // Use a unique signal for each aircraft to avoid conflicts
    int signal_num = SIGRTMIN + (aircraft->id % (SIGRTMAX - SIGRTMIN));
    if (sigaction(signal_num, &sa, NULL) == -1) {
        perror("sigaction failed");
        pthread_exit(nullptr);
    }

    // Set up the timer for position updates
    struct sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signal_num;
    sev.sigev_value.sival_ptr = aircraft;

    if (timer_create(CLOCK_REALTIME, &sev, &aircraft->timer_id) == -1) {
        perror("timer_create failed");
        pthread_exit(nullptr);
    }

    // Start the timer to expire every 1 second
    struct itimerspec its;
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 1;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(aircraft->timer_id, 0, &its, NULL) == -1) {
        perror("timer_settime failed");
        pthread_exit(nullptr);
    }

    // Message handling loop
    while (true) {
        // Non-blocking receive to handle radar requests
        struct _pulse msg;
        int rcvid = MsgReceive(aircraft->channel_id, &msg, sizeof(msg), NULL);

        if (rcvid == -1) {
            if (errno == EINTR) {
                // Interrupted by a signal (position update), continue
                continue;
            } else {
                perror("MsgReceive failed");
                break;
            }
        }

        if (rcvid == 0) {
            // Pulse message received (shouldn't happen in this case)
            continue;
        }

        if (msg.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        if (msg.type == _IO_MSG) {
            // Respond with current status
            AircraftStatus status;
            status.id = aircraft->id;
            status.x = aircraft->x;
            status.y = aircraft->y;
            status.z = aircraft->z;
            status.speedX = aircraft->speedX;
            status.speedY = aircraft->speedY;
            status.speedZ = aircraft->speedZ;

            MsgReply(rcvid, EOK, &status, sizeof(status));
        }
    }

    // Cleanup
    timer_delete(aircraft->timer_id);
    ChannelDestroy(aircraft->channel_id);
    pthread_exit(nullptr);
}*/
