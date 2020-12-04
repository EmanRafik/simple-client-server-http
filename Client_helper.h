#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <string>
#include <vector>
#include <pthread.h>

using namespace std;

class Client_helper
{
    public:
        Client_helper();
        virtual ~Client_helper();
        void readInputRequests();
        void setSocket(int socket);
        int getSocket();
        void setFolder(string folder);
        void handleResponse(string responseMessage, string data, int message_number);
        bool isLast();

    protected:

    private:
        string clientFolder;
        vector<string> paths;
        void parseLine(string line);
        string getFileName(string path);
        bool END;
        int sent;
        int received;
        pthread_mutex_t sent_mutex;
        pthread_mutex_t rec_mutex;
        pthread_mutex_t end_mutex;
};

#endif // CLIENT_HELPER_H
