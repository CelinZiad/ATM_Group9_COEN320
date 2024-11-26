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
#define COMMAND_SET_NEW_VELOCITY 667
using namespace std;


Aircraft::Aircraft(int id, int arrivalTime, double x, double y, double z,
                   double speedX, double speedY, double speedZ)
    : id(id), arrivalTime(arrivalTime), x(x), y(y), z(z),
      speedX(speedX), speedY(speedY), speedZ(speedZ) {
    arrived.store(false);
    left.store(false);
    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("Failed to create channel");
    }
}


// Aircraft* Aircraft::current_aircraft = nullptr;

void Aircraft::updatePosition() {
    x += speedX;
    y += speedY;
    z += speedZ;
    if (x < 0 || x > 100000 || y < 0 || y > 100000 || z < 0 || z > 25000) {

        left.store(true);
    }
}

void Aircraft::run() {


    // Client connecting to itself
    int coid = ConnectAttach(0, 0, chid, 0, 0);
    if (coid == -1) {
        std::cout << "Failed attaching" << std::endl;
    }

    struct sigevent sigev;
    SIGEV_PULSE_INIT(&sigev, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

    timer_t updateTimer;
    if (timer_create(CLOCK_MONOTONIC, &sigev, &updateTimer) == -1) {
        cout << "EXIT";
        return;
    }

    struct itimerspec timerValue;

    // Adjust initial timer value for arrivalTime == 0
    if (arrivalTime == 0) {
        timerValue.it_value.tv_sec = 0;
        timerValue.it_value.tv_nsec = 1; // Minimal non-zero value
    } else {
        timerValue.it_value.tv_sec = arrivalTime;
        timerValue.it_value.tv_nsec = 0;
    }

    timerValue.it_interval.tv_sec = 1;
    timerValue.it_interval.tv_nsec = 0;

    timer_settime(updateTimer, 0, &timerValue, NULL);

    int rcvid;
    CommandMessage msg;

    while (1) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0) {
            switch (msg.header.code) {
            case CODE_TIMER:
                if (!arrived.load()) {
                    arrived.store(true);
                    cout << "Aircraft " << id << " has entered the airspace.\n";
                } else if (left.load()) {
                    cout << "Aircraft " << id << " has left the airspace.\n";
                    return; // Exit the thread
                } else {
                    updatePosition();
                }
                break;
            default:
                cout << "Unknown code\n";
                break;
            }
        } else {
            switch (msg.command) {
            case COMMAND_RADAR_PING:
                AircraftStatus response;
                response.time = arrivalTime;
                response.id = id;
                response.x = x;
                response.y = y;
                response.z = z;
                response.speedX = speedX;
                response.speedY = speedY;
                response.speedZ = speedZ;
                MsgReply(rcvid, EOK, &response, sizeof(response));
                break;
            case COMMAND_SET_NEW_VELOCITY:
                speedX = msg.newVelocity.speedX;
                speedY = msg.newVelocity.speedY;
                speedZ = msg.newVelocity.speedZ;
                std::cout << "Aircraft " << id << " updated velocity: ("  << speedX << ", " << speedY << ", " << speedZ << ")\n";
                MsgReply(rcvid, EOK, NULL, 0);
            	break;
            default:
                cout << "Unknown command\n";
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

