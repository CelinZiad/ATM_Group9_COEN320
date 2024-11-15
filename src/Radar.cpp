#include "radar.h"
#include <iostream>
#include <map>
#include <cmath>
#include <chrono>
#include <thread>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <mutex>
#include <errno.h>
using namespace std;

mutex aircraft_mutex;

void* radar_thread_function(void* arg) {
    vector<Aircraft*>* aircrafts = static_cast<vector<Aircraft*>*>(arg);

    // Map to store connection IDs for each aircraft
    map<int, int> aircraft_connections;

    // Connect to each aircraft's channel
    for (auto& ac : *aircrafts) {
        // Wait until the aircraft thread has set up its channel
        while (ac->channel_id == 0) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        int coid = ConnectAttach(0, 0, ac->channel_id, _NTO_SIDE_CHANNEL, 0);
        if (coid == -1) {
            perror("ConnectAttach failed");
            continue;
        }
        aircraft_connections[ac->id] = coid;
    }

    // Radar loop
    while (true) {
        // Sleep for 5 seconds
        this_thread::sleep_for(chrono::seconds(5));

        vector<AircraftStatus> aircraft_statuses;

        // Request status from each aircraft
        for (auto it = aircraft_connections.begin(); it != aircraft_connections.end(); ) {
            int id = it->first;
            int coid = it->second;

            char msg = 0; // Empty message
            AircraftStatus status;

            int ret = MsgSend(coid, &msg, sizeof(msg), &status, sizeof(status));
            if (ret == -1) {
                if (errno == ESRCH) {
                    // Aircraft has exited, remove from map
                    cout << "Aircraft " << id << " is no longer available.\n";
                    ConnectDetach(coid);
                    it = aircraft_connections.erase(it);
                    continue;
                } else {
                    perror("MsgSend failed");
                    ++it;
                    continue;
                }
            }

            aircraft_statuses.push_back(status);
            ++it;
        }

        // Display the airspace
        cout << "\nCurrent Airspace Status:\n";
        for (const auto& status : aircraft_statuses) {
            cout << "Aircraft ID: " << status.id
                      << " | Position: (" << status.x << ", " << status.y << ", " << status.z << ")"
                      << " | Speed: (" << status.speedX << ", " << status.speedY << ", " << status.speedZ << ")\n";
        }

        // Check for separation violations
        check_separation(aircraft_statuses);

        // Exit radar loop if no aircraft remain
        if (aircraft_connections.empty()) {
            cout << "No more aircraft in the airspace. Radar is shutting down.\n";
            break;
        }
    }

    // Cleanup connections
    for (const auto& [id, coid] : aircraft_connections) {
        ConnectDetach(coid);
    }

    pthread_exit(nullptr);
}

void check_separation(const vector<AircraftStatus>& aircraft_statuses) {
    const double MIN_HORIZONTAL_SEPARATION = 3000.0;
    const double MIN_VERTICAL_SEPARATION = 1000.0;

    for (size_t i = 0; i < aircraft_statuses.size(); ++i) {
        for (size_t j = i + 1; j < aircraft_statuses.size(); ++j) {
            const auto& ac1 = aircraft_statuses[i];
            const auto& ac2 = aircraft_statuses[j];

            double dx = ac1.x - ac2.x;
            double dy = ac1.y - ac2.y;
            double dz = ac1.z - ac2.z;

            double horizontal_distance = sqrt(dx * dx + dy * dy);
            double vertical_distance = fabs(dz);

            if (horizontal_distance < MIN_HORIZONTAL_SEPARATION &&
                vertical_distance < MIN_VERTICAL_SEPARATION) {
                cout << "ALERT: Separation violation between Aircraft "
                          << ac1.id << " and Aircraft " << ac2.id << "\n";
                // Emit alarm or take necessary action
            }
        }
    }
}
