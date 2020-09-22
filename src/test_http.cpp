#include<iostream>
#include "util.hpp"

int main()
{
    util::CurlRequest req{};

    req.set_url("https://www.google.com/");
    req.perform();
    for(auto e:req.response())
        std::cout<<e;
    
    std::cout<<"\n";

    return 0;
}