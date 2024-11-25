/*
 * DataDisplay.h
 *
 *  Created on: Nov. 25, 2024
 *      Author: Elias
 */

#ifndef DATADISPLAY_H_
#define DATADISPLAY_H_
#include "Aircraft.h"
class DataDisplay {
public:

	DataDisplay();
	static void* start(void *context);
	int getChid() const;
	void run();
	void listen();
private:
	int chid;

};

#endif /* DATADISPLAY_H_ */
