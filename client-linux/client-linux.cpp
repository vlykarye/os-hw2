// socket headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

// other headers
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>

using namespace std;

#include "template-functions.h"

#include "sockets-client-linux.h"


// main

bool is_terminate(string const & message)
{
    if(message == "Terminate.")
    {
        return true;
    }
    if(message == "Terminate")
    {
        return true;
    }
    if(message == "terminate.")
    {
        return true;
    }
    if(message == "terminate")
    {
        return true;
    }

    return false;
}
string prompt(string const & text)
{
    string input;
    cout << text;
    getline(cin, input);
    return input;
}

#define LEN 1024 // recv buffer length

int main(int argc, char** argv)
{
    // Start client
    //if(winsock.load(2, 2))
    {
        Client client;

        char local_hostname[256] = {};
        gethostname(local_hostname, 256);

        string hostname = prompt("Enter a server host name: ");
        while(hostname != local_hostname)
        {
            cout << "Error. By requirement, the host name must equal '" << local_hostname << "'." << endl;
            hostname = prompt("Enter a server host name: ");
        }
        string port = prompt("Enter server port number: ");
        cout << endl;
        int p = to<int>(port);

        //cout << "connect_server" << endl;
        SOCKET server_socket = client.connect_server(hostname, p);
        if(server_socket == INVALID_SOCKET)
        {
            cout << "Could not connect to the server." << "\n" << "Please check the port number." << endl;
        }
        //cout << "connect_server end" << endl;
        if(server_socket != INVALID_SOCKET)
        {
            cout << "Connected to the server." << endl;
            // send/receive
            int result = 0;

            // send ID
            string send_message = prompt("Enter a user name: ");
            // desc: "send_message.size() + 1" so that the NULL terminator is sent
            result = send(server_socket, send_message.c_str(), send_message.size() + 1, 0);
            if(result == SOCKET_ERROR)
            {
                cout << "send failed: " << errno << endl;
            }

            if(!is_terminate(send_message))
            {
                // recv KEY
                string recv_message;
                while(true)
                {
                    char buffer[LEN + 1] = {};
                    int length = recv(server_socket, buffer, LEN, 0);
                    if(length < 0)
                    {
                        cout << "recv failed: " << errno << "." << endl;
                        break;
                    }
                    recv_message += buffer;
                    // desc: "length - 1" is last char in string
                    if(buffer[length - 1] == 0)
                    {
                        break;
                    }
                }
                if(recv_message.size() == 0)
                {
                    cout << "NOT FOUND" << endl;
                }
                else
                {
                    cout << recv_message << endl;
                }
            }

            cout << endl;
            client.release_server();
        }
    }

    return 0;
}
