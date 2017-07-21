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


class Server
{
public:
    // no copies please, only references
    Server(Server const &) = delete;
    void operator=(Server const &) = delete;

    Server()
    {
        // right now, this only needs to happen once
        ZeroMemory(&address_hints, sizeof(address_hints));
        address_hints.ai_family = AF_INET;
        address_hints.ai_socktype = SOCK_STREAM;
        address_hints.ai_protocol = IPPROTO_TCP;
        address_hints.ai_flags = AI_PASSIVE;
    }
    ~Server()
    {
        free_array();
        free_socket(socket_listen);
        free_socket(socket_client);
    }

    SOCKET &
        connect_client(int port_number)
    {
        if(listen_to(port_number))
        {
            if(!connect_to())
            {
                socket_client = INVALID_SOCKET;
            }
        }
        return socket_client;
    }
    bool
        // SD_RECEIVE, SD_SEND, SD_BOTH
        release_client(int how = SD_BOTH)
    {
        int result = 0;

        // shutdown the send half of the connection since no more data will be sent
        result = ::shutdown(socket_client, how);
        if(result == SOCKET_ERROR)
        {
            cout << "send shutdown failed: " << ::WSAGetLastError() << endl;
            return false;
        }
        connected = false;
        return true;
    }

private:
    // getaddrinfo: address hints
    addrinfo address_hints;
    // getaddrinfo: list of address details
    addrinfo *address_array = 0;

    // socket for listening
    SOCKET socket_listen = INVALID_SOCKET;
    // socket for communication
    SOCKET socket_client = INVALID_SOCKET;

    // state
    bool listening = false;
    bool connected = false;

    // current port number
    int port = 0;

    bool listen_to(int port_number)
    {
        if(listening && port == port_number)
        {
            return true;
        }
        port = port_number;
        listening = false;

        int result = 0;

        // getaddrinfo
        free_array();
        result = ::getaddrinfo(0, to<string>(port).c_str(), &address_hints, &address_array);
        if(result != 0)
        {
            cout << "getaddrinfo failed: " << result << endl;
            return false;
        }
        addrinfo * addr = address_array;

        // socket
        free_socket(socket_listen);
        socket_listen = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(socket_listen == INVALID_SOCKET)
        {
            cout << "socket failed: " << ::WSAGetLastError() << endl;
            return false;
        }

        // bind
        result = ::bind(socket_listen, addr->ai_addr, (int)addr->ai_addrlen);
        if(result == SOCKET_ERROR)
        {
            cout << "bind failed: " << ::WSAGetLastError() << endl;
            return false;
        }

        // listen
        result = ::listen(socket_listen, SOMAXCONN);
        if(result == SOCKET_ERROR)
        {
            cout << "listen failed: " << ::WSAGetLastError() << endl;
            return false;
        }

        listening = true;
        return true;
    }
    bool connect_to()
    {
        connected = false;

        // accept
        free_socket(socket_client);
        socket_client = ::accept(socket_listen, 0, 0);
        if(socket_client == INVALID_SOCKET)
        {
            cout << "accept failed: " << ::WSAGetLastError() << endl;
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
                cout << "closesocket failed: " << ::WSAGetLastError() << endl;
            }
            s = INVALID_SOCKET;
        }
    }
};
