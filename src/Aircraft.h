#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <pthread.h>
#include <sys/neutrino.h>
#include <signal.h>
#include <time.h>
#include <atomic>

// Aircraft Status Structure
struct AircraftStatus {
    int time;
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
};
struct AircraftVelocity {
    double speedX;
    double speedY;
    double speedZ;
};

struct CommandMessage {
			struct _pulse header;
	        int command;
	        AircraftVelocity newVelocity;
	    };

class Aircraft {
public:
    Aircraft(int id, int arrivalTime, double x, double y, double z, double speedX, double speedY, double speedZ);
    static void* start(void* context);

    // Delete copy constructor and assignment operator
    Aircraft(const Aircraft&) = delete;
    Aircraft& operator=(const Aircraft&) = delete;

    // Getters
    int getArrivalTime() const;
    int getChid() const;
    int getId() const;
    double getX() const;
    double getY() const;
    double getZ() const;
    double getSpeedX() const;
    double getSpeedY() const;
    double getSpeedZ() const;

    // Setters
    void setArrivalTime(int arrivalTime);
    void setId(int id);
    void setChid(int chid);
    void setX(double x);
    void setY(double y);
    void setZ(double z);
    void setSpeedX(double speedX);
    void setSpeedY(double speedY);
    void setSpeedZ(double speedZ);

    // Status Checkers
    bool hasArrived() const { return arrived.load(); }
    bool hasLeft() const { return left.load(); }

private:
    int chid;
    int arrivalTime;
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    std::atomic<bool> arrived;
    std::atomic<bool> left;

    void run();
    void updatePosition();

    timer_t timer_id;
};

#endif // AIRCRAFT_H
