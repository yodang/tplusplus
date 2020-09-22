#include<iostream>

#include"bencode.hpp"

using namespace bencode;

void print_bencode(std::ostream& s, const Bencoded& d);

void print_int(std::ostream& s, const Int& d)
{
    s << d;
}

void print_str(std::ostream& s, const String& d)
{
    s << "'" << d << "'";
}

void print_list(std::ostream& s, const List& d)
{
    s << "[";
    for(auto e: d.list)
    {
        print_bencode(s, e);
        s << ",";
    }
    s << "]";
}

void print_dict(std::ostream& s, const Dictionnary& d)
{
    s << "{";
    for(auto e: d.entries)
    {
        s << "'" << e.first << "'";
        s << ":";
        print_bencode(s, e.second);
        s << ",";
    }
    s << "}";
}

void print_bencode(std::ostream& s, const Bencoded& d)
{
    if(std::holds_alternative<Dictionnary>(d))
        print_dict(s, std::get<Dictionnary>(d));
    else if(std::holds_alternative<List>(d))
        print_list(s, std::get<List>(d));
    else if(std::holds_alternative<Int>(d))
        print_int(s, std::get<Int>(d));
    else if(std::holds_alternative<String>(d))
        print_str(s, std::get<String>(d));
}

int main()
{
    Bencoded data=read(std::cin);
    print_bencode(std::cout, data);

    return 0;
}