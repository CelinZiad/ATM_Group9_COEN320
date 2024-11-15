#include "aircraft.h"
#include <iostream>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <cstring>
#include <signal.h>
#include <time.h>
#include <errno.h>
using namespace std;

void update_position(int sig, siginfo_t* si, void* uc) {
    Aircraft* ac = static_cast<Aircraft*>(si->si_value.sival_ptr);

    // Update position
    ac->x += ac->speedX;
    ac->y += ac->speedY;
    ac->z += ac->speedZ;

    // Log the update
    cout << "Aircraft " << ac->id << " updated position to ("
              << ac->x << ", " << ac->y << ", " << ac->z << ")\n";

    // Check if the aircraft is still within the airspace bounds
    if (ac->x < 0 || ac->x > 100000 ||
        ac->y < 0 || ac->y > 100000 ||
        ac->z < 0 || ac->z > 25000) {
        cout << "Aircraft " << ac->id << " has left the airspace.\n";

        // Clean up timer
        timer_delete(ac->timer_id);

        // Clean up channel
        ChannelDestroy(ac->channel_id);

        // Terminate the aircraft thread
        pthread_exit(nullptr);
    }
}

void* aircraft_thread(void* arg) {
    Aircraft* ac = static_cast<Aircraft*>(arg);

    // Create a channel to receive messages
    int chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        pthread_exit(nullptr);
    }

    // Store the channel ID in the aircraft struct
    ac->channel_id = chid;

    // Set up the signal handler for position updates
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = update_position;
    sigemptyset(&sa.sa_mask);

    // Use a unique signal for each aircraft to avoid conflicts
    int signal_num = SIGRTMIN + (ac->id % (SIGRTMAX - SIGRTMIN));
    if (sigaction(signal_num, &sa, NULL) == -1) {
        perror("sigaction failed");
        pthread_exit(nullptr);
    }

    // Set up the timer for position updates
    struct sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signal_num;
    sev.sigev_value.sival_ptr = ac;

    if (timer_create(CLOCK_REALTIME, &sev, &ac->timer_id) == -1) {
        perror("timer_create failed");
        pthread_exit(nullptr);
    }

    // Start the timer to expire every 1 second
    struct itimerspec its;
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 1;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(ac->timer_id, 0, &its, NULL) == -1) {
        perror("timer_settime failed");
        pthread_exit(nullptr);
    }

    // Message handling loop
    while (true) {
        // Non-blocking receive to handle radar requests
        struct _pulse msg;
        int rcvid = MsgReceive(ac->channel_id, &msg, sizeof(msg), NULL);

        if (rcvid == -1) {
            if (errno == EINTR) {
                // Interrupted by a signal (position update), continue
                continue;
            } else {
                perror("MsgReceive failed");
                break;
            }
        }

        if (rcvid == 0) {
            // Pulse message received (shouldn't happen in this case)
            continue;
        }

        if (msg.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        if (msg.type == _IO_MSG) {
            // Respond with current status
            AircraftStatus status;
            status.id = ac->id;
            status.x = ac->x;
            status.y = ac->y;
            status.z = ac->z;
            status.speedX = ac->speedX;
            status.speedY = ac->speedY;
            status.speedZ = ac->speedZ;

            MsgReply(rcvid, EOK, &status, sizeof(status));
        }
    }

    // Cleanup
    timer_delete(ac->timer_id);
    ChannelDestroy(ac->channel_id);
    pthread_exit(nullptr);
}
