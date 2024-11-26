/*
 * CommunicationSystem.h
 *
 *  Created on: Nov. 25, 2024
 *      Author: Elias
 */

#ifndef COMMUNICATIONSYSTEM_H_
#define COMMUNICATIONSYSTEM_H_
#include "Aircraft.h"
struct AircraftVelocity {
    double speedX;
    double speedY;
    double speedZ;
};
class CommunicationSystem {
public:
	CommunicationSystem();
	CommunicationSystem(std::vector<std::shared_ptr<Aircraft>>& aircrafts);
	void send(std::shared_ptr<Aircraft>& R, AircraftVelocity m);

private:
	std::vector<std::shared_ptr<Aircraft>>& aircrafts;
};

#endif /* COMMUNICATIONSYSTEM_H_ */
