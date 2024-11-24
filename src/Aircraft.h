/*
 * Aircraft.h
 *
 *  Created on: Nov. 16, 2024
 *      Author: Elias
 */

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <pthread.h>
#include <sys/neutrino.h>
#include <signal.h>
#include <time.h>

// Aircraft Structure
struct AircraftStatus {
    int time;
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    int channel_id;
    timer_t timer_id;
};

// Aircraft Status for Radar


class Aircraft{
public:
	Aircraft(int id,int arrivalTime, double x,double y, double z,  double speedX,  double speedY, double speedZ);
	static void* start(void *context);

	int getArrivalTime() const;
	int getChid() const;
	//pthread_t getThreadId() const;
	int getId() const;
	double getX() const;
	double getY() const;
	double getZ() const;
	double getSpeedX() const;
	double getSpeedY() const;
	double getSpeedZ() const;

	void setArrivalTime(int arrivalTime);
	void setId(int id);
	void setChid(int chid);
	//void setThreadId(pthread_t thread_id);
	void setX(double x);
	void setY(double y);
	void setZ(double z);
	void setSpeedX(double speedX);
	void setSpeedY(double speedY);
	void setSpeedZ(double speedZ);
private:
	//pthread_t thread_id;
	int chid;
	int arrivalTime;
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool left,arrived;

	void run();
	void updatePosition();

	static Aircraft* current_aircraft;
	static void signal_handler(int sig, siginfo_t* si, void* uc);
	timer_t timer_id;
};

//void* aircraft_thread(void* arg);

#endif
