/*
 * ComputerSystem.h
 *
 *  Created on: Nov. 19, 2024
 *      Author: Elias
 */

#ifndef COMPUTERSYSTEM_H_
#define COMPUTERSYSTEM_H_

#include <vector>
#include "Aircraft.h"
#include "Radar.h"

typedef struct {
	int timerCode;
	int taskIntervalSeconds;
} periodicTask;
typedef struct {
	struct _pulse header;
	int command;
} ComputerSystemMessage;

class ComputerSystem {
private:
    int chid;
    int operatorChid;
    int displayChid;
    std::vector<Aircraft> aircrafts;
    std::vector<AircraftStatus> aircraftsStatus;
    Radar radar;
    void listen();
    void createTasks();

public:
    ComputerSystem(std::vector<Aircraft> aircrafts);

    int getChid() const;
    void setOperatorChid(int id);
    void setDisplayChid(int id);
    void checkViolation();
    void checkSeparation(Aircraft ac1, Aircraft ac2);
    void logSystem();
    static void* start(void* context);
    void run();
    void initialize();
    void setAircrafts(std::vector<Aircraft> aircraftss);
};

#endif
