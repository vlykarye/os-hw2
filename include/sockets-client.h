// REQUIRES
//
// #pragma comment(lib, "Ws2_32.lib")
// #include <winsock2.h>
// #include <ws2tcpip.h>
//
// #include <iostream>
// #include <string>
// using namespace std;
//
// template<typename TO, typename FROM>
// TO to(FROM const & a)
// {
//     TO b;
//     stringstream ss;
//     ss << a;
//     ss >> b;
//     return b;
// }


class Client
{
public:
    // no copies please, only references
    Client(Client const &) = delete;
    void operator=(Client const &) = delete;

    Client()
    {
        // right now, this only needs to happen once
        ZeroMemory(&address_hints, sizeof(address_hints));
        address_hints.ai_family = AF_UNSPEC;
        address_hints.ai_socktype = SOCK_STREAM;
        address_hints.ai_protocol = IPPROTO_TCP;
    }
    ~Client()
    {
        free_array();
        free_socket(socket_server);
    }

    SOCKET &
        connect_server(string host_name, int port_number)
    {
        if(!connect_to(host_name, port_number))
        {
            socket_server = INVALID_SOCKET;
        }
        return socket_server;
    }
    bool
        // SD_RECEIVE, SD_SEND, SD_BOTH
        release_server(int how = SD_BOTH)
    {
        int result = 0;

        // shutdown the send half of the connection since no more data will be sent
        result = ::shutdown(socket_server, how);
        if(result == SOCKET_ERROR)
        {
            error = true;
            cout << "send shutdown failed: " << ::WSAGetLastError() << endl;
            return false;
        }
        connected = false;
        return true;
    }
    bool
        error_state(int * value = 0)
    {
        if(error)
        {
            if(value) { *value = WSAGetLastError(); }
            error = false;
            return true;
        }
        return false;
    }

private:
    // getaddrinfo: address hints
    addrinfo address_hints;
    // getaddrinfo: list of address details
    addrinfo *address_array = 0;

    // socket for communication
    SOCKET socket_server = INVALID_SOCKET;

    // state
    bool connected = false;
    bool error = false;

    // current port number
    string hostname;
    int port = 0;

    bool connect_to(string host_name, int port_number)
    {
        if(connected && hostname == host_name && port == port_number)
        {
            return true;
        }
        hostname = host_name;
        port = port_number;
        connected = false;

        int result = 0;

        // getaddrinfo
        free_array();
        result = ::getaddrinfo(hostname.c_str(), to<string>(port).c_str(), &address_hints, &address_array);
        if(result != 0)
        {
            error = true;
            cout << "getaddrinfo failed: " << result << endl;
            return false;
        }

        addrinfo * addr = address_array;
        while(addr != 0)
        {
            // socket
            free_socket(socket_server);
            socket_server = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if(socket_server == INVALID_SOCKET)
            {
                error = true;
                cout << "socket failed: " << ::WSAGetLastError() << endl;
                return false;
            }

            // connect
            result = ::connect(socket_server, addr->ai_addr, (int)addr->ai_addrlen);
            if(result == SOCKET_ERROR)
            {
                addr = addr->ai_next;
                continue;
            }

            break;
        }
        if(addr == 0)
        {
            //cout << "connect failed: " << ::WSAGetLastError() << endl;
            return false;
        }

        connected = true;
        return true;
    }

    void free_array()
    {
        if(address_array)
        {
            ::freeaddrinfo(address_array);
        }
        address_array = 0;
    }
    void free_socket(SOCKET & s)
    {
        if(s != INVALID_SOCKET)
        {
            if(::closesocket(s) != 0)
            {
                error = true;
                cout << "closesocket failed: " << ::WSAGetLastError() << endl;
            }
            s = INVALID_SOCKET;
        }
    }
};
