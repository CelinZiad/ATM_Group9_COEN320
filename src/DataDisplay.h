#ifndef DATADISPLAY_H_
#define DATADISPLAY_H_
#include "Aircraft.h"

#define COMMAND_DISPLAY_AIRCRAFT 1111
#define COMMAND_DISPLAY_AUGMENTED_INFO 2222

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
