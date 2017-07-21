// REQUIRES
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



// HACK
// TODO: erase later
//struct in_addr
//{
//    unsigned long s_addr;  // load with inet_aton()
//};
//
//struct sockaddr_in
//{
//    short            sin_family;   // e.g. AF_INET
//    unsigned short   sin_port;     // e.g. htons(3490)
//    struct in_addr   sin_addr;     // see struct in_addr, below
//    char             sin_zero[8];  // zero this if you want to
//};

#define SOCKET int // linux uses int instead of a SOCKET struct
//#define SOMAXCONN 64 // number of simultaneous socket connections
#define INVALID_SOCKET -1 // anything that is -1 is invalid for linux
#define SOCKET_ERROR -1 // most functions that fail return -1 for linux



class Client
{
public:
    // no copies please, only references
    Client(Client const &) = delete;
    void operator=(Client const &) = delete;

    Client()
    {
        // right now, this only needs to happen once
        memset(&address_hints, 0, sizeof(address_hints));
        address_hints.ai_family = AF_INET;
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
        // SHUT_RD, SHUT_WR, SHUT_RDWR
        release_server(int how = SHUT_RDWR)
    {
        int result = 0;

        // shutdown the send half of the connection since no more data will be sent
        result = ::shutdown(socket_server, how);
        if(result == SOCKET_ERROR)
        {
            error = true;
            cout << "send shutdown failed: " << errno << endl;
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
            if(value) { *value = errno; }
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
            cout << "getaddrinfo failed: " << gai_strerror(result) << endl;
            return false;
        }
        addrinfo * addr = address_array;

        while(addr != 0)
        {
            // socket
            free_socket(socket_server);
            //cout << "socket" << endl;
            socket_server = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            //cout << "socket end" << endl;
            if(socket_server == INVALID_SOCKET)
            {
                error = true;
                cout << "socket failed: " << errno << endl;
                return false;
            }

            // DEBUG
            //cout << "family " << address_server.sin_family << endl;
            //cout << "port " << address_server.sin_port << endl;
            //cout << "address " << address_server.sin_addr.s_addr << endl;
            //cout << endl;

            // connect
            //cout << "connect" << endl;
            result = ::connect(socket_server, addr->ai_addr, (int)addr->ai_addrlen);
            //cout << "connect end" << endl;
            if(result == SOCKET_ERROR)
            {
                addr = addr->ai_next;
                continue;
            }

            break;
        }
        if(addr == 0)
        {
            //error = true;
            //cout << "connect failed: " << errno << endl;
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
            if(::close(s) != 0)
            {
                error = true;
                cout << "closesocket failed: " << errno << endl;
            }
            s = INVALID_SOCKET;
        }
    }
};
