/*
 * CommunicationSystem.cpp
 *
 *  Created on: Nov. 25, 2024
 *      Author: Elias
 */

#include "CommunicationSystem.h"
#include <iostream>
#define COMMAND_SET_NEW_VELOCITY 667
CommunicationSystem::CommunicationSystem(){

}
CommunicationSystem::CommunicationSystem(std::vector<std::shared_ptr<Aircraft>>& aircrafts):aircrafts(aircrafts){

}

void Communication::send(std::shared_ptr<Aircraft>& R, AircraftVelocity& m){
	int planeChid=R->getChid();

	int coid;
	coid=ConnectAttach(0,0,planeChid,0,0);
	if(coid==-1){
		std::cout << "client connection failed. Exiting thread" << std::endl;
		return;
	}
	struct CommandMessage {
			struct _pulse header;
	        int command;
	        AircraftVelocity newVelocity;
	    } msg;
	    msg.command = COMMAND_SET_NEW_VELOCITY;
	    msg.newVelocity = m;
	    int sendId;
	    sendId = MsgSend(coid, &msg, sizeof(msg), NULL, 0);
	    ConnectDetach(coid);
		if (sendId == -1) {
			std::cout << "Message failed to send!" << std::endl;
			return;
		}
		return;
}



