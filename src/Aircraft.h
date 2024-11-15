#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <pthread.h>
#include <sys/neutrino.h>
#include <signal.h>
#include <time.h>

// Aircraft Structure
struct Aircraft {
    int time;           // Time when the aircraft enters the airspace
    int id;             // Aircraft ID
    double x, y, z;     // Position coordinates
    double speedX, speedY, speedZ; // Speed components
    int channel_id;     // Channel ID for message passing
    pthread_t thread_id; // Thread ID for the aircraft
    timer_t timer_id;   // Timer ID for position updates
};

// Aircraft Status for Radar
struct AircraftStatus {
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
};

void* aircraft_thread(void* arg);

#endif
