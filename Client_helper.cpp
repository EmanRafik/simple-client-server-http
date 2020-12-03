#include "Client_helper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <unistd.h>

#define BUFFERSIZE 1024 * 1024

char sendBuffer[BUFFERSIZE];
//char receiveBuffer[BUFFERSIZE];
int sock;

using namespace std;

Client_helper::Client_helper()
{
    //ctor
}

Client_helper::~Client_helper()
{
    //dtor
}

void Client_helper::setSocket(int socket)
{
    sock = socket;
}

void Client_helper::setFolder(string folder)
{
    this->clientFolder = folder;
}

void Client_helper::readInputRequests()
{
    string name = clientFolder + "/input.txt";
    fstream file;
    file.open(name, ios::in);
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            cout << line << endl;
            parseLine(line);
        }
        file.close();
    }
}

void Client_helper::parseLine(string line)
{
    istringstream ss(line);
    //true is for get request, false is for post request
    bool isGET;
    //true for requests other than get and post
    bool unSupportedRequest;
    string token;
    string filePath;
    string hostName;
    int portNumber;

    int i = 0;
    while (ss >> token)
    {
        switch (i)
        {
        case 0:
            if (token == "client_get")
                isGET = true;
            else if (token == "client_post")
                isGET = false;
            else
                unSupportedRequest = true;

            break;
        case 1:
            filePath = token;
            paths.push_back(filePath);
            break;
        case 2:
            hostName = token;
            break;
        case 3:
            portNumber = stoi(token);
            break;
        default:
            break;
        }
        if (unSupportedRequest)
            break;
        i++;
    }
    if (hostName == "localhost")
    {
        if (isGET)
        {
            string request = "GET " + filePath + " HTTP/1.1\r\n";
            //sendAndReceive(request, filePath);
            for (int i = 0; i < request.length(); i++)
            {
                sendBuffer[i] = request.at(i);
            }
            sendBuffer[request.length()] = '\0';
            send(sock, sendBuffer, request.length()+1, 0);
            cout << "********sent********" << endl;
        }
        else
        {
            string file = getFileName(filePath);
            file = clientFolder + file;
            ifstream stream(file);
            string data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            string request = "POST " + filePath + " HTTP/1.1\r\n"  + data;
            //sendAndReceive(request, filePath);
            for (int i = 0; i < request.length(); i++)
            {
                sendBuffer[i] = request.at(i);
            }
            sendBuffer[request.length()] = '\0';
            send(sock, sendBuffer, request.length()+1, 0);
        }
    }
} 

string Client_helper::getFileName(string path)
{
    for (int i = path.length() - 1; i >= 0; i--)
    {
        if (path.at(i) == '/')
        {
            return path.substr(i);
        }
    }
}

void Client_helper::handleResponse(string responseMessage, string data, int message_number)
{
    cout << responseMessage << endl;
    if (responseMessage == "HTTP/1.1 200 OK\r\n" && data.length() > 0)
    {
        string name = getFileName(paths[message_number]);
        string path = clientFolder + name;
        ofstream outputFile(path);
        outputFile << data;
        cout << data;
        outputFile.close();
    }
}
