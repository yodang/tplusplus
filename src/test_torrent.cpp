#include<iostream>
#include<iomanip>
#include<cstddef>
#include"bencode.hpp"
#include"bittorrent.hpp"

using namespace bencode;
using namespace bittorrent;


int main()
{
    Bencoded data=read(std::cin);
    Torrent t{std::get<Dictionnary>(data)};

    std::cout << std::setbase(16);

    for(auto i: t.info_hash())
        std::cout<<std::to_integer<int>(i)<<" ";

    std::cout<<std::endl;

    Transfer dl{t};

    std::cout << "request\n";
    std::cout << dl.announce()<< "\n";

    return 0;
}