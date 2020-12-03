#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

#include <string>
#include <vector>
#include <fstream>

using namespace std;

class Server_helper
{
    public:
        Server_helper(int clientSocket);
        virtual ~Server_helper();
        void parseRequest(vector<string> headers, string data);
        int getSocket();

    protected:

    private:
        int sock;
        void handleGETRequest(string URL);
        void handlePOSTRequest(string URL, string data);
        void handleNonExistingFile();
        void handleExistingFile(string path);
        
};

#endif // SERVER_HELPER_H
