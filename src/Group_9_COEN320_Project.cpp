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
#include "radar.h"
using namespace std;

int main() {
    vector<Aircraft*> aircrafts;
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
        Aircraft* aircraft = new Aircraft();
        // Read the data in the format: Time ID X Y Z SpeedX SpeedY SpeedZ
        if (!(iss >> aircraft->time >> aircraft->id >> aircraft->x >> aircraft->y >> aircraft->z
                  >> aircraft->speedX >> aircraft->speedY >> aircraft->speedZ)) {
            cerr << "Error: Incorrect file format on line: " << line << endl;
            delete aircraft;
            break;
        }
        aircraft->channel_id = 0; // Initialize channel ID
        aircrafts.push_back(aircraft); // Add the aircraft to the vector
    }

    infile.close();

    // Start aircraft threads at their specified entry times
    for (auto& aircraft : aircrafts) {
        // Sleep until the aircraft's scheduled time
        int sleep_time = aircraft->time;
        if (sleep_time > 0) {
            this_thread::sleep_for(chrono::seconds(sleep_time));
        }

        // Create aircraft thread
        pthread_t tid;
        pthread_create(&tid, nullptr, aircraft_thread, aircraft);
        aircraft->thread_id = tid;

        cout << "Aircraft " << aircraft->id << " has entered the airspace.\n";
    }

    // Start the radar in a separate thread
    pthread_t radar_tid;
    pthread_create(&radar_tid, nullptr, radar_thread_function, &aircrafts);

    // Join aircraft threads
    for (auto& aircraft : aircrafts) {
        pthread_join(aircraft->thread_id, nullptr);
    }

    // Join radar thread
    pthread_join(radar_tid, nullptr);

    // Clean up dynamically allocated aircraft
    for (auto& aircraft : aircrafts) {
        delete aircraft;
    }

    return 0;
}
