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



class Server
{
public:
    // no copies please, only references
    Server(Server const &) = delete;
    void operator=(Server const &) = delete;

    Server()
    {
        // right now, this only needs to happen once
        memset(&address_hints, 0, sizeof(address_hints));
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
        // SHUT_RD, SHUT_WR, SHUT_RDWR
        release_client(int how = SHUT_RDWR)
    {
        int result = 0;

        // shutdown the send half of the connection since no more data will be sent
        result = ::shutdown(socket_client, how);
        if(result == SOCKET_ERROR)
        {
            cout << "send shutdown failed: " << errno << endl;
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

        //char local_hostname[256] = {};
        //gethostname(local_hostname, 256);
        //struct hostent *hp = 0;
        //hp = gethostbyname(local_hostname);
        //if(hp == 0)
        //{
        //    cout << "gethostbyname failed: " << errno << endl;
        //    return false;
        //}

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
        //cout << "socket" << endl;
        socket_listen = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        //cout << "socket end" << endl;
        if(socket_listen == INVALID_SOCKET)
        {
            cout << "socket failed: " << errno << endl;
            return false;
        }

        // bind
        //cout << "bind" << endl;
        result = ::bind(socket_listen, addr->ai_addr, (int)addr->ai_addrlen);
        //cout << "bind end" << endl;
        if(result == SOCKET_ERROR)
        {
            cout << "bind failed: " << errno << endl;
            return false;
        }

        // DEBUG
        //cout << "family " << address_server.sin_family << endl;
        //cout << "port " << address_server.sin_port << endl;
        //cout << "address " << address_server.sin_addr.s_addr << endl;
        //cout << endl;

        // listen
        //cout << "listen" << endl;
        result = ::listen(socket_listen, SOMAXCONN);
        //cout << "listen end" << endl;
        if(result == SOCKET_ERROR)
        {
            cout << "listen failed: " << errno << endl;
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
        //cout << "accept" << endl;
        socket_client = ::accept(socket_listen, 0, 0);
        //cout << "accept end" << endl;
        if(socket_client == INVALID_SOCKET)
        {
            cout << "accept failed: " << errno << endl;
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
            if(::close(s) == SOCKET_ERROR)
            {
                cout << "closesocket failed: " << errno << endl;
            }
            s = INVALID_SOCKET;
        }
    }
};
