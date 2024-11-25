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
#include "DataDisplay.h"
#include "computersystem.h"
#include "radar.h"
using namespace std;

int main() {
	vector<std::shared_ptr<Aircraft>> aircrafts;

    DataDisplay display;
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
        aircrafts.push_back(make_shared<Aircraft>(id, arrivalTime, x, y, z, speedX, speedY, speedZ)); // Add the aircraft to the vector

    }
    int numberOfAircraft=aircrafts.size();
    pthread_t aircraftThreads[numberOfAircraft];


    for (size_t i=0;i<aircrafts.size();i++) {
        // Create aircraft thread
        pthread_create(&aircraftThreads[i], NULL,&Aircraft::start, aircrafts[i].get());
    }
    ComputerSystem compSystem(aircrafts);
    //data display thread
    pthread_t data_display_tid;
    pthread_create(&data_display_tid, NULL, &DataDisplay::start, &display);
    compSystem.setDisplayChid(display.getChid());

    std::this_thread::sleep_for(std::chrono::milliseconds(1 * 1000));

    //create computer system

    pthread_t computer_system_tid;
    pthread_create(&computer_system_tid, nullptr, &ComputerSystem::start, &compSystem);

        //aircraft threads join
    	for (size_t i=0;i<aircrafts.size();i++) {
    		pthread_join(aircraftThreads[i], NULL);
         }

    // Join Computer System Thread thread
    pthread_join(computer_system_tid, nullptr);
    //join datadisplay
     pthread_join(data_display_tid, nullptr);


    return 0;
}

