#ifndef RADAR_H
#define RADAR_H

#include "aircraft.h"
#include <vector>
using namespace std;

void* radar_thread_function(void* arg);
void check_separation(const vector<AircraftStatus>& aircraft_statuses);

#endif // RADAR_H
