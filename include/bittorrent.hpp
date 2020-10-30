#ifndef BITTORRENT_HPP
#define BITTORRENT_HPP

#include<string_view>
#include<array>
#include<vector>
#include<numeric>
#include<sstream>
#include<string>
#include<cstring>
#include<typeinfo>
#include<algorithm>
#include<random>

#include<boost/asio.hpp>

#include"bencode.hpp"
#include"sha.hpp"
#include "util.hpp"

namespace bittorrent {

using namespace std::string_literals;

static constexpr size_t BT_HASH_SIZE=20;

struct File{
    std::vector<std::string> path;
    size_t length;
};

class Torrent{
public:
    using hash=std::array<std::byte, BT_HASH_SIZE>;

    //FIXME: no sanity checking is done on the content of the data structure
    Torrent(const bencode::Dictionnary data)
    {
        using namespace bencode;
        std::ostringstream s{};
        auto infos=std::get<Dictionnary>(data.entries.at("info"s));
        bencode::write_dict(s, infos);
        auto content=s.str();
        info_hash_=SHA::hash(content.begin(), content.end());
        piece_length_= std::get<Int>(infos.entries.at("piece length"s));
        name_ = std::get<String>(infos.entries.at("name"s));
        announce_ = std::get<String>(data.entries.at("announce"s));
        hashes_ = read_hashes(std::get<String>(infos.entries.at("pieces"s)));
        if(auto l=std::get_if<Int>(&(infos.entries.at("length"s))))
            length_=*l;
        else
            length_=0;
        
        if(auto iter=infos.entries.find("files"s); iter!=infos.entries.end())
        {
            auto f=std::get<List>((*iter).second);
            files_=read_files(f);
        }
        else
           files_={};
    }
    
    const std::string_view announce() const
    {
        return announce_;
    }
    
    const std::string_view name() const
    {
        return name_;
    }

    std::vector<hash> pieces_hash() const
    {
        return hashes_;
    }

    hash pieces_hash(int i) const
    {
        return hashes_[i];
    }

    hash info_hash() const
    {
        return info_hash_;
    }
    size_t piece_length() const
    {
        return piece_length_;
    }
    
    //Returns an empty vector if the torrent is single file
    std::vector<File> files() const;
    
    //Returns 0 if the torrent has multiple files
    size_t length() const;

    //Expects the index of the file in the vector returned by files() as the argument.
    //Throws if the index is out of bound
    std::pair<int, size_t> file_start(int file_index)
    {
        auto piece=piece_length();
        auto files=this->files();
        auto len=std::accumulate(files.begin(), files.begin()+file_index, 0, [](auto a, auto b){return a+b.length;});

        return std::make_pair(len/piece, len%piece);
    }
private:
    bencode::Dictionnary data_;
    hash info_hash_;
    size_t piece_length_;
    std::string name_;
    std::string announce_;
    std::vector<hash> hashes_;
    size_t length_;
    std::vector<File> files_;

    static std::vector<hash> read_hashes(const std::string& hash_str)
    {
        std::vector<hash> result{};
        result.reserve(hash_str.length()/BT_HASH_SIZE);
        auto start=hash_str.begin();
        while(std::distance(start, hash_str.end())>=BT_HASH_SIZE)
        {
            hash e{};
            memcpy(e.begin(), &(*start), BT_HASH_SIZE);
            result.push_back(e);
            start++;
        }
        return result;
    }
    static std::vector<File> read_files(const bencode::List file_list)
    {
        std::vector<File> result{};

        for(auto e: file_list.list)
        {
            bencode::Dictionnary file=std::get<bencode::Dictionnary>(e);
            bencode::List path=std::get<bencode::List>(file.entries.at("path"s));
            std::vector<std::string> p{};
            std::transform(path.list.begin(), path.list.end(), p.begin(), [](auto e){return std::get<bencode::String>(e);});
            result.push_back({
                p,
                static_cast<size_t>(std::get<bencode::Int>(file.entries.at("length"s)))
            });
        }

        return result;
    }
};

class Transfer{
public:
    enum class Event{
        started,
        completed,
        stopped,
        none
    };

    Transfer(Torrent torrent)
    :torrent_{torrent},
    uploaded_{0},
    downloaded_{0},
    port_{6969},
    //peers_{},
    peer_id_{"TPP-"}
    {
        std::random_device r{};
        std::default_random_engine gen{r()};

        for(int i=0; i<16; i++)
        {
            peer_id_.push_back(static_cast<char>(gen()));
        }
    }

    util::CurlRequest announce(Event e=Event::none)
    {
        util::CurlRequest req{};
        req.set_url(std::string{torrent_.announce()});
        req.set_param("peer_id"s, peer_id_);
        auto hash=torrent_.info_hash();
        req.set_param("info_hash"s, hash.begin(), hash.end());
        req.set_param("port"s, port_);
        req.set_param("uploaded"s, uploaded_);
        req.set_param("downloaded"s, downloaded_);
        req.set_param("compact"s, 1);

        return req;
    }
private:
    std::vector<boost::asio::ip::tcp::endpoint> peers_;
    Torrent torrent_;
    std::string peer_id_;
    size_t uploaded_;
    size_t downloaded_;
    int port_;
};
/*
class Client{
public:
private:
    std::vector<Transfer> active;
};
*/

std::vector<boost::asio::ip::tcp::endpoint> parse_compact_peers(const std::string_view data)
{
    using namespace boost::asio::ip;
    std::vector<tcp::endpoint> result{};
    //process each record of 6 "characters"
    for(int i=0; i<data.length(); i+=6)
    {
        std::array<uint8_t, 4> addr{data[i], data[i+1], data[i+2], data[i+3]};
        uint16_t port=static_cast<uint16_t>((data[i+4]<<8)|data[i+5]);
        result.push_back({make_address_v4(addr), port});
    }
    return result;
}

std::vector<boost::asio::ip::tcp::endpoint> parse_compact_peers6(const std::string_view data)
{
    using namespace boost::asio::ip;
    std::vector<tcp::endpoint> result{};
    //process each record of 18 "characters"
    for(int i=0; i<data.length(); i+=18)
    {
        std::array<uint8_t, 16> addr{
            data[i],
            data[i+1],
            data[i+2],
            data[i+3],
            data[i+4],
            data[i+5],
            data[i+6],
            data[i+7],
            data[i+8],
            data[i+9],
            data[i+10],
            data[i+11],
            data[i+12],
            data[i+13],
            data[i+14],
            data[i+15]
        };
        uint16_t port=static_cast<uint16_t>((data[i+16]<<8)|data[i+17]);
        result.push_back({make_address_v6(addr), port});
    }
    return result;
}

}

#endif //BITTORRENT_HPP