#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>

class CommandHandler {
    public:
        virtual void handleCommand(const std::string& input, int connId) = 0;
    private:
        virtual void handleConnect(std::istringstream& stream, int connId) = 0;
        virtual void handleDisconnect(int connId) = 0;
        virtual void handlePublish(std::istringstream& stream, int connId) = 0;
        virtual void handleSubscribe(std::istringstream& stream, int connId) = 0;
        virtual void handleUnsubscribe(std::istringstream& stream, int connId) = 0;
};

#endif