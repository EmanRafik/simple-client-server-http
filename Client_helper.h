#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <string>
#include <vector>

using namespace std;

class Client_helper
{
    public:
        Client_helper();
        virtual ~Client_helper();
        void readInputRequests();
        void setSocket(int socket);
        void setFolder(string folder);
        void handleResponse(string responseMessage, string data, int message_number);

    protected:

    private:
        string clientFolder;
        vector<string> paths;
        void parseLine(string line);
        string getFileName(string path);
};

#endif // CLIENT_HELPER_H
