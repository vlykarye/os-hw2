#include "stdafx.h"

using namespace std;

#include "template-functions.h"

#include "sockets-winsock.h"
#include "sockets-client.h"
#include "sockets-server.h"


// main

bool find_free_port(int & port)
{
    Client client;
    SOCKET server_socket = client.connect_server("localhost", port);
    while(server_socket != INVALID_SOCKET)
    {
        ++port;
        server_socket = client.connect_server("localhost", port);
    }
    return client.error_state() == false;
}
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
    // Get keys
    string filename = "keys21.txt";// prompt("Enter a file name: ");
    ifstream in(filename);
    if(!in)
    {
        cout << "Cannot open input file. Program terminates!!!" << endl;
        return 1;
    }
    map<string, string> public_keys;
    string line;
    while(getline(in, line))
    {
        if(line.empty() || line.at(0) == '\n') { continue; }

        string id;
        string key;

        stringstream ss(line);
        ss >> id;
        ss >> key;
        if(ss.bad()) { continue; }
        public_keys.insert(pair<string, string>(id, key));
    }
    in.close();

    // print map
    //for(map<string, string>::const_iterator it = public_keys.begin();
    //    it != public_keys.end();
    //    ++it)
    //{
    //    static int count = 0;
    //    cout << count++ << ": " << it->first << " " << it->second << endl;
    //}

    // Start server
    WinSock &winsock = WinSock::Get_Instance();
    if(winsock.load(2, 2))
    {
        Server server;

        string port = prompt("Enter server port number: ");
        int p = to<int>(port);
        cout << p << endl;
        if(find_free_port(p) == false)
        {
            return 1;
        }
        cout << "Listening to port number: " << p << "." << endl;
        cout << endl;

        while(true)
        {
            SOCKET client_socket = server.connect_client(p);
            if(client_socket != INVALID_SOCKET)
            {
                //cout << "Accepted a client." << endl;
                // send/receive
                {
                    int result = 0;

                    // recv ID
                    string recv_message;
                    while(true)
                    {
                        char buffer[LEN + 1] = {};
                        int length = recv(client_socket, buffer, LEN, 0);
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

                    if(recv_message == "Terminate.")
                    {
                        server.release_client();
                        break;
                    }

                    // send KEY
                    string send_message;
                    map<string, string>::iterator it = public_keys.find(recv_message);
                    if(it != public_keys.end()) { send_message = it->second; }

                    // desc: "send_message.size() + 1" so that the NULL terminator is sent
                    result = send(client_socket, send_message.c_str(), send_message.size() + 1, 0);
                    if(result == SOCKET_ERROR)
                    {
                        cout << "send failed: " << WSAGetLastError() << endl;
                    }

                    // disconnect from client
                    server.release_client();
                }
            }
        }
    }

    return 0;
}
