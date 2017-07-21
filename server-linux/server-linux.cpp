// server
// windows version

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
#include <fstream>
#include <map>

using namespace std;

#include "template-functions.h"

#include "sockets-linux-client.h"
#include "sockets-linux-server.h"


// main

#define SOCKET int // linux uses int instead of a SOCKET struct

bool find_free_port(int & port)
{
     Client client;
     SOCKET server_socket = client.connect_server("localhost", port);
     while ( server_socket != INVALID_SOCKET )
     {
          ++port;
          server_socket = client.connect_server("localhost", port);
     }
     return client.error_state() == false;
}

bool is_terminate_message(string const & message)
{
     if ( message == "Terminate." )
     {
          return true;
     }
     if ( message == "Terminate" )
     {
          return true;
     }
     if ( message == "terminate." )
     {
          return true;
     }
     if ( message == "terminate" )
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
     // Get keys
     string filename = prompt("Enter a file name: ");
     ifstream in(filename);
     if ( !in )
     {
          cout << "Cannot open input file. Program terminates!!!"
               << endl;
          return 1;
     }
     map<string, string> public_keys;
     string line;
     while ( getline(in, line) )
     {
          if ( line.empty() || line.at(0) == '\n' ) { continue; }

          string id;
          string key;

          stringstream ss(line);
          ss >> id;
          ss >> key;
          if ( ss.bad() ) { continue; }
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
     //
     //
     {
          Server server;

          string port = prompt("Enter server port number: ");
          int port_number = to<int>(port);
          cout << port_number
               << endl;
          if ( find_free_port(port_number) == false )
          {
               cout << "Could not connect to port number: " << port_number << "."
                    << endl;
               return 1;
          }
          cout << "Listening to port number: " << port_number
               << endl
               << endl;

          while ( true )
          {
               //cout << "connect_client" << endl;
               SOCKET client_socket = server.connect_client(port_number);
               //cout << "connect_client end" << endl;
               if ( client_socket == INVALID_SOCKET ) { break; }

               cout << "Accepted a client." << endl;
               // send/receive
               {
                    int result = 0;

                    // recv ID
                    string recv_message;
                    while ( true )
                    {
                         char buffer[LEN + 1] = {};
                         int length = recv(client_socket, buffer, LEN, 0);
                         if ( length < 0 )
                         {
                              cout << "recv failed: " << errno << "."
                                   << endl;
                              break;
                         }
                         recv_message += buffer;
                         // note: "length - 1" is last char in string
                         if ( buffer[length - 1] == 0 )
                         {
                              break;
                         }
                    }
                    if ( recv_message.size() == 0 )
                    {
                         cout << "NO MESSAGE" << endl;
                    }
                    else
                    {
                         cout << "request: '" << recv_message << "'" << endl;
                    }

                    if ( !is_terminate_message(recv_message) )
                    {
                         // send KEY
                         string send_message;
                         map<string, string>::iterator it = public_keys.find(recv_message);
                         if ( it != public_keys.end() ) { send_message = it->second; }

                         result = send(
                              client_socket,
                              send_message.c_str(),
                              send_message.size() + 1, // +1 to send NULL terminator
                              0
                         );
                         if ( result == SOCKET_ERROR )
                         {
                              cout << "send failed: " << errno
                                   << endl;
                         }
                    }

                    cout << endl;
                    server.release_client();

                    if ( is_terminate_message(recv_message) ) { break; }

               }
          } // while ( true )
     }

     return 0;
}
