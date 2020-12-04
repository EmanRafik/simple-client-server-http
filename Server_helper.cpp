#include "Server_helper.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

using namespace std;

#define BUFFERSIZE 1024 * 1024

char sendBuffer[BUFFERSIZE];


Server_helper::Server_helper(int clientSocket)
{
    //ctor
    this->sock = clientSocket;
}

Server_helper::~Server_helper()
{
    //dtor
}
int Server_helper::getSocket()
{
    return sock;
}
void Server_helper::parseRequest(vector<string> headers, string data)
{
    if (headers.size() > 0)
    {
        string requestLine = headers[0];
        cout << "**** " << sock << " " << requestLine << endl;
        istringstream stream(requestLine);
        string requestField;
        //True for GET and false for POST
        bool methodIsGET;
        //True if method is not true or false
        bool unSupportedMethod = false;
        string URL;
        //True of HTTP version is not 1.1
        bool unSupportedVersion = false;
        int i = 0;
        while (stream >> requestField)
        {
            switch (i)
            {
            case 0:
                if (requestField.compare("GET") == 0)
                    methodIsGET = true;
                else if (requestField.compare("POST") == 0)
                    methodIsGET = false;
                else
                    unSupportedMethod = true;                    
                break;
            case 1:
                URL = requestField;
                break;
            case 2:
                if (requestField.compare("HTTP/1.1") != 0)
                    unSupportedVersion = true;
                break;
            default:
                break;
            }
            i++;
        }
        string host;
        bool presistantConnection = true;
        for (unsigned int i = 1; i < headers.size(); i++)
        {
            string headerField = headers[i];
            cout << headerField << endl;
            int spaceIndex = headerField.find(" ");
            string name = headerField.substr(0, spaceIndex - 1);
            string value = headerField.substr(spaceIndex + 1);
            //will be used for caching
            if (name.compare("Host") == 0)
                host = value;
            if (name.compare("Connection") == 0 && value.compare("close") == 0)
                presistantConnection = false;
        }
        if (!unSupportedVersion && !unSupportedMethod)
        {
            if (methodIsGET)
            {
                handleGETRequest(URL);
            }
            else
            {
                handlePOSTRequest(URL, data);
            }
        }
    }
}

void Server_helper::handleGETRequest(string URL)
{
    string filePath = "server" + URL;
    ifstream file(filePath);
    if (file.is_open())
    {
        handleExistingFile(filePath);
    }
    else
    {
        handleNonExistingFile();
    }
}

void Server_helper::handlePOSTRequest(string URL, string data)
{
    string path = "server" + URL;
    ofstream file(path);
    if (file.is_open())
    {
        file << data;
        file.close();
    }

    string responseMessage = "HTTP/1.1 200 OK\r\n\r\n";
    for (int i = 0; i < responseMessage.length(); i++)
    {
        sendBuffer[i] = responseMessage.at(i);
    }
    sendBuffer[responseMessage.length()] = '\0';
    send(sock, sendBuffer, responseMessage.length()+1, 0);
}

void Server_helper::handleNonExistingFile()
{
    string respondMessage = "HTTP/1.1 404 Not Found\r\n\r\n";
    for (int i = 0; i < respondMessage.length(); i++)
    {
        sendBuffer[i] = respondMessage.at(i);
    }
    sendBuffer[respondMessage.length()] = '\0';
    send(sock, sendBuffer, respondMessage.length()+1, 0);
}

void Server_helper::handleExistingFile(string path)
{
    string responseMessage = "HTTP/1.1 200 OK\r\n\r\n";
    ifstream stream(path);
    string data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    responseMessage = responseMessage + data;
    for (int i = 0; i < responseMessage.length(); i++)
    {
        sendBuffer[i] = responseMessage.at(i);
    }
    sendBuffer[responseMessage.length()] = '\0';
    send(sock, sendBuffer, responseMessage.length()+1, 0);
}