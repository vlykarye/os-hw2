#include "stdafx.h"

using namespace std;

#include "template-functions.h"

#include "sockets-winsock.h"
#include "sockets-client.h"


// main

string prompt(string const & text)
{
    string input;
    cout << text;
    getline(cin, input);
    return input;
}

#define LEN 4 // recv buffer length

int main(int argc, char** argv)
{
    // Start client
    WinSock &winsock = WinSock::Get_Instance();
    if(winsock.load(2, 2))
    {
        Client client;

        char local_hostname[256] = {};
        gethostname(local_hostname, 256);

        string hostname = "DULLAHAN";// prompt("Enter a server host name: ");
        while(hostname != local_hostname)
        {
            cout << "Error. By requirement, the host name must equal '" << local_hostname << "'." << endl;
            hostname = prompt("Enter a server host name: ");
        }
        string port = prompt("Enter server port number: ");
        cout << endl;
        int p = to<int>(port);

        SOCKET server_socket = client.connect_server(hostname, p);
        if(server_socket != INVALID_SOCKET)
        {
            //cout << "Connected to the server." << endl;
            // send/receive
            int result = 0;

            // send ID
            string send_message = prompt("Enter a user name: ");
            // desc: "send_message.size() + 1" so that the NULL terminator is sent
            result = send(server_socket, send_message.c_str(), send_message.size() + 1, 0);
            if(result == SOCKET_ERROR)
            {
                cout << "send failed: " << WSAGetLastError() << endl;
            }

            if(send_message != "Terminate.")
            {
                // recv KEY
                string recv_message;
                while(true)
                {
                    char buffer[LEN + 1] = {};
                    int length = recv(server_socket, buffer, LEN, 0);
                    if(length < 0)
                    {
                        cout << "recv failed: " << WSAGetLastError() << "." << endl;
                        break;
                    }
                    recv_message += buffer;
                    // desc: "length - 1" is last char in string
                    if(buffer[length - 1] == 0)
                    {
                        break;
                    }
                }
                cout << recv_message << endl;
            }

            client.release_server();
        }
    }

    return 0;
}
