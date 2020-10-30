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
    auto req=dl.announce();
    req.perform();
    req.print_response();
    auto response=req.response();
    auto stream=std::istream{&response};
    auto swarm=read(stream);
    auto peers=parse_compact_peers(std::get<String>(std::get<Dictionnary>(swarm).entries["peers"]));
    std::cout << std::setbase(10);
    for(auto peer: peers)
    {
        std::cout << peer.address().to_string()<< " " <<peer.port()<< "\n";
    }
    std::cout << '\n';

    return 0;
}