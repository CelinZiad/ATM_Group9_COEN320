#include "OperatorConsole.h"
#include <iostream>
#include <atomic>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <errno.h>


pthread_mutex_t OperatorConsole::mutex = PTHREAD_MUTEX_INITIALIZER;
std::queue<OperatorConsoleResponseMessage> OperatorConsole::responseQueue;

OperatorConsole::OperatorConsole() : chid(-1) {}

int OperatorConsole::getChid() const {
    return chid;
}

void OperatorConsole::run() {
    // Create a communication channel
    if ((chid = ChannelCreate(0)) == -1) {
        std::cout << "Operator console: channel creation failed. Exiting thread." << std::endl;
        return;
    }

    // Start the console reader thread
    pthread_t cinReaderThread;
    std::atomic_bool cinReaderStopFlag(false);
    pthread_create(&cinReaderThread, NULL, &OperatorConsole::cinRead, &cinReaderStopFlag);

    // Start listening for messages
    listen();

    // Stop the console reader thread
    cinReaderStopFlag = true;
    pthread_join(cinReaderThread, NULL);
}

void OperatorConsole::listen() {
    int rcvid;
    OperatorConsoleCommandMessage msg;
    while (1) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        switch (msg.systemCommandType) {
        case OPCON_CONSOLE_COMMAND_GET_USER_COMMAND: {
            // When the computer system asks us, check if any commands are waiting and return the applicable response.
            pthread_mutex_lock(&mutex);
            if (responseQueue.empty()) {
                OperatorConsoleResponseMessage resMsg;
                resMsg.userCommandType = OPCON_USER_COMMAND_NO_COMMAND_AVAILABLE;
                MsgReply(rcvid, EOK, &resMsg, sizeof(resMsg));
            } else {
                OperatorConsoleResponseMessage resMsg = responseQueue.front();
                responseQueue.pop();
                MsgReply(rcvid, EOK, &resMsg, sizeof(resMsg));
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        default:
            std::cout << "OperatorConsole: received unknown command " << msg.systemCommandType << std::endl;
            MsgError(rcvid, ENOSYS);
            break;
        }
    }
}

void* OperatorConsole::start(void *context) {
    auto c = (OperatorConsole*) context;
    c->run();
    return NULL;
}

void* OperatorConsole::cinRead(void *param) {
    // Get the flag we monitor to know when to stop reading
    std::atomic_bool *stop = (std::atomic_bool*) param;

    std::string msg;
    while (!(*stop)) {
        // Get a command from cin and break it up by spaces
        std::getline(std::cin, msg);
        std::vector<std::string> tokens;
        tokenize(tokens, msg);

        if (tokens.size() == 0)
            continue;

        if (tokens[0] == OPCON_COMMAND_STRING_SHOW_PLANE) {
            if (tokens.size() < 2) {
                std::cout << "OperatorConsole: Error: must provide plane number" << std::endl;
                continue;
            }
            try {
                // Parse the plane number and prepare a response in the queue.
                int planeNum = std::stoi(tokens[1]);

                OperatorConsoleResponseMessage res;
                res.userCommandType = OPCON_USER_COMMAND_DISPLAY_PLANE_INFO;
                res.planeNumber = planeNum;

                pthread_mutex_lock(&mutex);
                responseQueue.push(res);
                pthread_mutex_unlock(&mutex);
            } catch (std::invalid_argument &e) {
                std::cout << "OperatorConsole: Error: not a valid integer" << std::endl;
                continue;
            }
        } else if (tokens[0] == OPCON_COMMAND_STRING_SET_VELOCITY) {
            if (tokens.size() < 5) {
                std::cout << "Error: must provide plane number and 3 velocity components (x,y,z)" << std::endl;
                continue;
            }
            try {
                // Parse the plane number and velocity components and prepare a response.
                int planeNum = std::stoi(tokens[1]);
                double components[3];
                for (size_t i = 0; i < 3; i++) {
                    components[i] = std::stod(tokens[2 + i]);
                }
                Vec3 velocity { components[0], components[1], components[2] };

                OperatorConsoleResponseMessage res;
                res.userCommandType = OPCON_USER_COMMAND_SET_PLANE_VELOCITY;
                res.planeNumber = planeNum;
                res.newVelocity = velocity;

                pthread_mutex_lock(&mutex);
                responseQueue.push(res);
                pthread_mutex_unlock(&mutex);
            } catch (std::invalid_argument &e) {
                std::cout << "OperatorConsole: Error: not a valid number" << std::endl;
                continue;
            }
        } else {
            std::cout << "OperatorConsole: Unknown command" << std::endl;
            continue;
        }
    }
    return NULL;
}

// Breaks up a string by spaces
void OperatorConsole::tokenize(std::vector<std::string> &dest, std::string &str) {
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ' ')) {
        dest.push_back(token);
    }
}
