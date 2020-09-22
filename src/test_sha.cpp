#include<iostream>
#include"sha.hpp"

static constexpr std::array<std::byte, 20> expected={std::byte{0xda}, std::byte{0x39}, std::byte{0xa3}, std::byte{0xee}, std::byte{0x5e}, std::byte{0x6b}, std::byte{0x4b}, std::byte{0x0d}, std::byte{0x32}, std::byte{0x55}, std::byte{0xbf}, std::byte{0xef}, std::byte{0x95}, std::byte{0x60}, std::byte{0x18}, std::byte{0x90}, std::byte{0xaf}, std::byte{0xd8}, std::byte{0x07}, std::byte{0x09}};


void test_str()
{
    std::string data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    auto result1=SHA::hash(data.begin(), data.end());
    auto result2=SHA::hash(data);
    std::cout<< "string " << (result1==result2)<<'\n';
}

void test_vector()
{
    std::vector<char> data(2000, 'a');
    auto result1=SHA::hash(data.begin(), data.end());
    auto result2=SHA::hash(data);
    std::cout<< "vector " << (result1==result2)<<'\n';
}


void test_longstring()
{
    std::string data(2000, 'a');
    auto result1=SHA::hash(data.begin(), data.end());
    auto result2=SHA::hash(data);
    std::cout<< "long string " << (result1==result2)<<'\n';
}

int main()
{
    test_str();
    test_vector();
    test_longstring();
    return 0;
}