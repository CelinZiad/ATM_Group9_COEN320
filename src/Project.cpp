#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <ctime>
#include "aircraft.h"
#include "computersystem.h"
#include "radar.h"
using namespace std;

int main() {
    vector<Aircraft> aircrafts;


    string line;

    // Read aircraft info from aircraft_data.txt (initial data)
    string file_name = "/tmp/aircraft_data.txt";
    ifstream infile(file_name);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << file_name << " - " << strerror(errno) << endl;
        return 1;
    }

    while (getline(infile, line)) {
        istringstream iss(line);
        int arrivalTime, id;
        double x, y, z, speedX, speedY, speedZ;


        if (!(iss >> arrivalTime >> id >> x >> y >> z >> speedX >> speedY >> speedZ)) {
            cerr << "Error: Incorrect file format on line: " << line << endl;
            break;
        }


        //Aircraft aircraft = new Aircraft(id, arrivalTime, x, y, z, speedX, speedY, speedZ);
        aircrafts.push_back(Aircraft(id, arrivalTime, x, y, z, speedX, speedY, speedZ)); // Add the aircraft to the vector

    }
    int numberOfAircraft=aircrafts.size();
    pthread_t aircraftThreads[numberOfAircraft];


    // Start aircraft threads at their specified entry times
    for (size_t i=0;i<aircrafts.size();i++) {
        // Sleep until the aircraft's scheduled time
        int sleep_time = aircrafts[i].getArrivalTime();
        if (sleep_time > 0) {
            this_thread::sleep_for(chrono::seconds(sleep_time));
        }

        // Create aircraft thread

        pthread_create(&aircraftThreads[i], NULL,&Aircraft::start, &aircrafts[i]);


        cout << "Aircraft " << aircrafts[i].getId() << " has entered the airspace.\n";
    }
    ComputerSystem compSystem= ComputerSystem(aircrafts);


    //create computer system

    pthread_t computer_system_tid;
    pthread_create(&computer_system_tid, nullptr, &ComputerSystem::start, &compSystem);

        //aircraft threads join
    	for (size_t i=0;i<aircrafts.size();i++) {
    		pthread_join(aircraftThreads[i], NULL);
         }

    // Join Computer System Thread thread
    pthread_join(computer_system_tid, nullptr);


    return 0;
}

