// REQUIRES
//
// #pragma comment(lib, "Ws2_32.lib")
// #include <winsock2.h>
// #include <ws2tcpip.h>
//
// #include <iostream>
// using namespace std;


// singleton to handle winsock library start and shutdown
class WinSock
{
public:
    // no copies please, only references
    WinSock(WinSock const &) = delete;
    void operator=(WinSock const &) = delete;

    // get WinSock reference
    static WinSock &
        Get_Instance()
    {
        static WinSock instance;
        return instance;
    }

    bool
        // use to init and re-init the winsock library with specific version
        load(unsigned char const major_version,
             unsigned char const minor_version)
    {
        cleanup();

        if(!m_running && !m_error)
        {
            if(init(major_version, minor_version))
            {
                return true;
            }
        }
        return false;
    }

private:
    WinSock() {}
    ~WinSock()
    {
        cleanup();
    }

    bool
        init(unsigned char const major,
             unsigned char const minor)
    {
        if(!m_running && !m_error)
        {
            // initialize winsock
            int e = ::WSAStartup(MAKEWORD(major, minor), &m_wsadata);
            if(!e)
            {
                //cout << "WSAStartup" << endl;
                m_running = true;
                return true;
            }
            cout << "WSAStartup failed: " << e << endl;
            m_error = true;
        }
        return false;
    }

    bool
        cleanup()
    {
        if(m_running && !m_error)
        {
            // shutdown winsock
            int e = ::WSACleanup();
            if(!e)
            {
                //cout << "WSACleanup" << endl;
                m_running = false;
                return true;
            }
            cout << "WSACleanup failed: " << e << endl;
            m_error = true;
        }
        return false;
    }

    // object state
    bool m_running = false;
    bool m_error = false;

    // data members
    WSADATA m_wsadata;
};
