#ifndef RADAR_H
#define RADAR_H

#include "Aircraft.h"
#include <vector>
#include <memory>

class Radar {
public:
    Radar(std::vector<std::shared_ptr<Aircraft>>& aircrafts);
    void setSystemChid(int chid) { systemChid = chid; }
    AircraftStatus pingAircraft(std::shared_ptr<Aircraft> ac);
    std::vector<AircraftStatus> getAllAircraftStatus();

private:
    std::vector<std::shared_ptr<Aircraft>>& aircrafts;
    int systemChid;
};

#endif // RADAR_H
