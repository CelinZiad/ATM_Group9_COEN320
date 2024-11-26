#ifndef OPERATOR_CONSOLE_H
#define OPERATOR_CONSOLE_H

#include <queue>
#include <string>
#include <vector>
#include <pthread.h>
#include <atomic>
#include "OperatorConsoleMessages.h"

class OperatorConsole {
public:
    OperatorConsole();
    int getChid() const;
    void run();
    static void* start(void *context);

private:
    int chid;
    void listen();
    static void* cinRead(void *param);
    static void tokenize(std::vector<std::string> &dest, std::string &str);
    static pthread_mutex_t mutex;
    static std::queue<OperatorConsoleResponseMessage> responseQueue;
};

#endif // OPERATOR_CONSOLE_H
