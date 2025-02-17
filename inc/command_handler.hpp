#include <iostream>
#include <sstream>
#include <string>
#include <map>

class CommandHandler {
    public:
        virtual void handleCommand(const std::string& input) = 0;
    private:
        virtual void handleConnect(std::istringstream& stream) = 0;
        virtual void handleDisconnect();
        virtual void handlePublish(std::istringstream& stream) = 0;
        virtual void handleSubscribe(std::istringstream& stream) = 0;
        virtual void handleUnsubscribe(std::istringstream& stream) = 0;
};