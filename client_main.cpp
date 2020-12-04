#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "Client_helper.h"
#include <unistd.h>

#define BUFFERSIZE 1024 * 1024
#define TIMEOUT 5000

char receiveBuffer[BUFFERSIZE];
int bufferEnd = 0;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond_var = PTHREAD_COND_INITIALIZER;

Client_helper client_helper;

using namespace std;

void error(string message);
void *receiveFun(void *arg);
void *processResponse(void *arg);

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        error("Incorrect argumnts");
    }
    char *clientFolder = argv[1];
    client_helper.setFolder((string)clientFolder);
    char *serverIP = argv[2];
    in_port_t serverPort = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        error("socket failed");
    }
    client_helper.setSocket(sock);
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    // if ((inet_pton(AF_INET, serverIP, &serverAddress.sin_addr.s_addr)) < 0)
    // {

    // }
    if ((connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
    {
        error("connect failed");
    }

    pthread_t receive;
    int *clientSock = (int *)malloc(sizeof(int));
    *clientSock = sock;
    pthread_create(&receive, NULL, receiveFun, (void *)clientSock);

    client_helper.readInputRequests();

    pthread_join(receive, NULL);
    return 0;
}

void error(string message)
{
    cout << message;
    exit(-1);
}

void *receiveFun(void *arg)
{
    int sock = *((int *)arg);
    free(arg);

    pthread_t process_response_thread;
    pthread_create(&process_response_thread, NULL, processResponse, NULL);

    while (true)
    {
        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(sock, &set);
        timeout.tv_sec = 20;
        timeout.tv_usec = 0;
        int val = select(sock + 1, &set, NULL, NULL, &timeout);
        if (val < 0)
        {
        }
        else if (val == 0)
        {
            cout << "*********TIMEOUT**********" << endl;
            break;
        }
        else
        {
            ssize_t numBytesRcvd = recv(sock, receiveBuffer + bufferEnd, BUFFERSIZE - 1 - bufferEnd, 0);
            if (numBytesRcvd > 0)
            {         
                pthread_mutex_lock(&buffer_mutex);
                bufferEnd += numBytesRcvd;
                
                pthread_mutex_unlock(&buffer_mutex);
                pthread_cond_signal(&buffer_cond_var);
            }
        }
    }
    //pthread_join(process_response_thread, NULL);
}

void *processResponse(void *arg)
{
    string responseMessage = "";
    string data = "";
    bool dataFlag;
    int message_number = 0;
    for (int i = 0; i < BUFFERSIZE; i++)
    {
        pthread_mutex_lock(&buffer_mutex);
        if (i == bufferEnd)
        {
            pthread_cond_wait(&buffer_cond_var, &buffer_mutex);
        }
        pthread_mutex_unlock(&buffer_mutex);
        cout << receiveBuffer[i];
        if (receiveBuffer[i] == '\0')
        {
            client_helper.handleResponse(responseMessage, data, message_number);
            if (client_helper.isLast())
            {
                close(client_helper.getSocket());
                exit(0);
            }
            message_number++;
            responseMessage = "";
            data = "";
            dataFlag = false;
        }
        else if (dataFlag)
        {
            data += receiveBuffer[i];
        }
        else
        {
            responseMessage += receiveBuffer[i];
            if (receiveBuffer[i] == '\r' && i + 1 < BUFFERSIZE && receiveBuffer[i + 1] == '\n')
            {
                i++;
                responseMessage += receiveBuffer[i];
                dataFlag = true;
            }
        }
    }
}