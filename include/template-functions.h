// REQUIRES
//
// #include <iostream>
// // headers for container classes
// using namespace std;

// for any std container that implements const_iterator
// doesn't work with: map, 
template<typename CONTAINER, typename STREAM>
void print(CONTAINER const & c, STREAM & o)
{
    for(typename CONTAINER::const_iterator it = c.begin();
        it != c.end();
        ++it)
    {
        o << *it;
    }
    o << endl;
}

// REQUIRES
//
// #include <sstream>
// using namespace std;

// for anything stringstream can handle
template<typename TO, typename FROM>
TO to(FROM const & a)
{
    TO b;
    stringstream ss;
    ss << a;
    ss >> b;
    return b;
}
