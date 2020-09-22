#ifndef BENCODE_HPP
#define BENCODE_HPP
#include<map>
#include<vector>
#include<iostream>
#include<cassert>
#include<exception>
#include<variant>

namespace bencode {

enum class BType{
    STRING,
    INT,
    LIST,
    DICT
};
struct List;
struct Dictionnary;

using String=std::string;
using Int=int;
using Bencoded=std::variant<List,Dictionnary,String,Int>;

struct List{
    std::vector<Bencoded> list;
    BType type() const{
        return BType::LIST;
    }

    List()
    :list{}
    {
    }

    template<typename T>
    List(T v)
    :list{v}
    {
    }
};

struct Dictionnary{
    std::map<std::string, Bencoded> entries;
    BType type() const{
        return BType::DICT;
    }

    Dictionnary()
    :entries{}
    {
    }

    template<typename T>
    Dictionnary(T m)
    :entries{m}
    {
    }
};

Bencoded read(std::istream& s);

Int read_int(std::istream& s)
{
    if(s.get() != 'i')
        throw std::runtime_error{"Bad input"};

    Int acc=0;
    char c;
    bool first=true;
    bool neg=false;
    while((c=s.get())!='e')
    {
        if(first && c=='-')
            neg=true;
        else if(c>='0' && c<='9')
        {
            acc*=10;
            acc+=c-'0';
        }
        else
            throw std::runtime_error{"Bad input"};
    }

    return (neg? -acc:acc);
}

String read_string(std::istream& s)
{
    int len;
    s >> len;
    if(s.get() != ':')
     throw std::runtime_error{"Bad input"};

    String val{};
    while((len--)>0)
        val.push_back(s.get());
    
    return val;
}

List read_list(std::istream& s)
{
    if(s.get() != 'l')
        throw std::runtime_error{"Bad input"};
    
    List l{};

    while(s.peek()!='e')
    {
        Bencoded val=read(s);
        l.list.push_back(val);
    }
    s.ignore();
    return l;
}

Dictionnary read_dict(std::istream& s)
{
    if(s.get() != 'd')
        throw std::runtime_error{"Bad input"};
    
    Dictionnary d{};

    while(s.peek()!='e')
    {
        String key=read_string(s);
        Bencoded val=read(s);
        d.entries[key]=val;
    }
    s.ignore();
    return d;
}

Bencoded read(std::istream& s)
{
    char c=s.peek();
    switch(c)
    {
        case 'l':
            return read_list(s);
        case 'd':
            return read_dict(s);
        case 'i':
            return read_int(s);
        default:
            return read_string(s);
    }
}

void write(std::ostream& s, const Bencoded& d);

void write_list(std::ostream&s, const List& d)
{
    s<<"l";
    for(auto e: d.list)
        write(s, e);
    s<<"e";
}

void write_int(std::ostream&s, const Int& d)
{
    s<<"i" << d <<"e";
}

void write_str(std::ostream&s, const String& d)
{
    s<< d.length() << ":" << d;
}

void write_dict(std::ostream&s, const Dictionnary& d)
{
    s<<"d";
    for(auto e: d.entries)
    {
        write_str(s, e.first);
        write(s, e.second);
    }
    s<<"e";
}

void write(std::ostream& s, const Bencoded& d)
{
    if(std::holds_alternative<Dictionnary>(d))
        write_dict(s, std::get<Dictionnary>(d));
    else if(std::holds_alternative<List>(d))
        write_list(s, std::get<List>(d));
    else if(std::holds_alternative<Int>(d))
        write_int(s, std::get<Int>(d));
    else if(std::holds_alternative<String>(d))
        write_str(s, std::get<String>(d));
}


}

#endif //BENCODE_HPP