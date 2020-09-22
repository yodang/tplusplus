#ifndef SHA_HPP
#define SHA_HPP

#include<openssl/sha.h>
#include<memory>
#include<vector>
#include<type_traits>

/**
 * Wrapper around libcrypto sha-1 API
 */

class SHA{
public:
    SHA()
    :ctx_{}
    {
    }

    template<typename T>
    static std::array<std::byte, 20> hash(T begin, T end)
    {
        static_assert(std::is_pod_v<typename std::iterator_traits<T>::value_type>);
        //Check should be with contiguous_iterator_tag but that's a C++20 feature so we use the next best thing
        static_assert(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<T>::iterator_category>);
        std::array<std::byte, 20> result{};
        size_t length=std::distance(begin, end)*sizeof(*begin);
        SHA1(reinterpret_cast<const unsigned char*>(&(*begin)), length, reinterpret_cast<unsigned char*>(result.data()));
        return result;
    }
    template<typename T>
    static std::array<std::byte, 20> hash(const T& data)
    {
        static_assert(std::is_pod_v<typename T::value_type>);
        //Check should be with contiguous_iterator_tag but that's a C++20 feature so we use the next best thing
        static_assert(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<typename T::iterator>::iterator_category>);
        std::array<std::byte, 20> result{};
        SHA1(reinterpret_cast<const unsigned char*>(data.data()), (data.size())*sizeof(typename T::value_type), reinterpret_cast<unsigned char*>(result.data()));
        return result;
    }
private:
    std::unique_ptr<SHA_CTX> ctx_;
};

#endif //SHA_HPP