#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Server_helper.h"
#include <pthread.h>
#include <queue>
#include <unistd.h>

using namespace std;

#define BUFFERSIZE 1024 * 1024
#define CLIENTS_THREAD_POOL_SIZE 50


pthread_t clientsThreadPool[CLIENTS_THREAD_POOL_SIZE];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clients_cond_var = PTHREAD_COND_INITIALIZER;
queue<int> clients;

void *clientsFun(void *arg);
void processMessage(Server_helper *server_helper, char buffer[], int bufferEnd);
void error(string message);
void serveClient(int *socket);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
    }

    for (int i = 0; i < CLIENTS_THREAD_POOL_SIZE; i++)
    {
        //cout << "heree" << endl;
        pthread_create(&clientsThreadPool[i], NULL, clientsFun, NULL);
    }
    in_port_t port = atoi(argv[1]);
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket < 0)
    {
        printf("%s\n", "socket failed");
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if ((bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
    {
        printf("%s\n", "bind failed");
    }
    if (listen(serverSocket, 50) < 0)
    {
        printf("%s\n", "listen failed");
    }
    while (1)
    {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocket < 0)
        {
            printf("%s\n", "accept failed");
        }
        char clientName[INET_ADDRSTRLEN];
        if ((inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, clientName, sizeof(clientName))) != NULL)
        {
            printf("Handling client %s %d\n", clientName, ntohs(clientAddress.sin_port));
        }
        pthread_mutex_lock(&clients_mutex);
        clients.push(clientSocket);
        pthread_cond_signal(&clients_cond_var);
        pthread_mutex_unlock(&clients_mutex);
    }

    for (int i = 0; i < CLIENTS_THREAD_POOL_SIZE; i++)
    {
        pthread_join(clientsThreadPool[i], NULL);
    }
    return 0;
}

void processMessage(Server_helper *server_helper, char buffer[], int bufferEnd)
{
    string line = "";
    vector<string> headerLines;
    string data = "";
    //true for data lines and false for header lines
    bool dataFlag = false;
    for (int i = 0; i < bufferEnd; i++)
    {
        cout << buffer[i];
        if (buffer[i] == '\0')
        {
            server_helper->parseRequest(headerLines, data);
            headerLines.clear();
            line = "";
            data = "";
            dataFlag = false;
        }
        else if (!dataFlag && buffer[i] == '\n')
        {
            headerLines.push_back(line);
            line = "";
        }
        else if (!dataFlag && buffer[i] == '\r' && (i + 1) < BUFFERSIZE && buffer[i + 1] == '\n')
        {
            headerLines.push_back(line);
            i++;
            if (i + 1 < BUFFERSIZE && buffer[i + 1] == '\0')
            {
                server_helper->parseRequest(headerLines, data);
                headerLines.clear();
                line = "";
                data = "";
                dataFlag = false;
            }
            else
            {
                dataFlag = true;
            }
        }
        else
        {
            if (dataFlag)
            {
                //cout << buffer[i];
                data += buffer[i];
            }
            else
                line += buffer[i];
        }
    }
}

void *clientsFun(void *arg)
{
    while (1)
    {
        int *sock = (int*)(malloc(sizeof(int)));
        pthread_mutex_lock(&clients_mutex);
        if (clients.empty())
        {
            pthread_cond_wait(&clients_cond_var, &clients_mutex);
            *sock = clients.front();
            clients.pop();
        } 
        else
        {
            *sock = clients.front();
            clients.pop();
        }
        pthread_mutex_unlock(&clients_mutex);
        serveClient(sock);
    }
}
void serveClient(int *socket)
{
    int clientSocket = *socket;
    cout << "SERVING ----> " << clientSocket << endl;
    Server_helper server_helper(clientSocket);
    char buffer[BUFFERSIZE];
    int bufferEnd = 0;
    while(1)
    {
        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(clientSocket, &set);
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        int val = select(clientSocket + 1, &set, NULL, NULL, &timeout);
        if (val < 0)
        {
        }
        else if (val == 0)
        {
            cout << "***** CLIENT " << clientSocket << " TIMEDOUT*****" << endl;
            break;
        }
        else
        {
            ssize_t numBytesRcvd = recv(clientSocket, buffer + bufferEnd, BUFFERSIZE - 1 - bufferEnd, 0);
            bufferEnd += numBytesRcvd;
        }
    }
    //sleep(5);
    processMessage(&server_helper, buffer, bufferEnd);
}
void error(string message)
{
    cout << message;
    exit(-1);
}