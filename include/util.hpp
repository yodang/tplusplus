#ifndef UTIL_HPP
#define UTIL_HPP

#include<vector>
#include<map>
#include<string>
#include<stdexcept>
#include<string_view>
#include<memory>
#include<curl/curl.h>

namespace util{

class CurlResponse: public std::streambuf{
public:
    CurlResponse(std::vector<char> v)
    :buf_{v}
    {
        setg(buf_.data(), buf_.data(), buf_.data()+buf_.size());
    }
private:
    std::vector<char> buf_;
};

class CurlRequest{
public:
    CurlRequest()
    :url_{},
    params_{},
    response_{}
    {
        if(!curl_initialized)
        {
            if(curl_global_init(CURL_GLOBAL_DEFAULT)!=0)
                throw std::runtime_error("Curl initialization failed");
        }

        handle_=curl_easy_init();
        if(handle_==nullptr)
            throw std::runtime_error("Cannot get Curl handle");
        
        if(curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, callback)!=0
           || curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this)!=0)
            throw std::runtime_error("Error setting Curl callback");
    }

    //Copy is forbidden but the class can be moved
    CurlRequest(const CurlRequest&)=delete;
    CurlRequest& operator=(const CurlRequest&)=delete;
    CurlRequest(CurlRequest&&)=default;
    CurlRequest& operator=(CurlRequest&&)=default;

    void set_url(std::string url)
    {
        url_=url;
    }

    void set_param(std::string name, const std::string& value)
    {
        auto str=curl_easy_escape(handle_, value.c_str(), value.length());
        params_[name]=std::string{str};
        curl_free(str);
    }

    template<typename Integer>
    void set_param(std::string name, const Integer& value)
    {
        static_assert(std::is_integral_v<Integer>);
        params_[name]=std::to_string(value);
    }

    template<typename T>
    void set_param(std::string name, T begin, T end)
    {
        //static_assert(std::is_integral_v<typename std::iterator_traits<T>::value_type>);
        //Check should be with contiguous_iterator_tag but that's a C++20 feature so we use the next best thing
        static_assert(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<T>::iterator_category>);
        auto str=curl_easy_escape(handle_, reinterpret_cast<const char*>(begin), std::distance(begin, end));
        params_[name]=std::string{str};
        curl_free(str);
    }

    void perform()
    {
        auto request=build_url();
        curl_easy_setopt(handle_, CURLOPT_URL, request.c_str());
        curl_easy_perform(handle_);
    }

    CurlResponse response()
    {
        return CurlResponse(response_);
    }

    ~CurlRequest()
    {
        curl_easy_cleanup(handle_);
    }

    std::string build_url()
    {
        std::string r{};
        r.append(url_);
        if(params_.size()!=0)
            r.append("?");
        
        auto separator{""};
        for(const auto& [name, value]: params_)
        {
            r.append(separator).append(name).append("=").append(value);
            separator="&";
        }

        return r;
    }

    void print_response()
    {
        for(auto c: response_)
            std::cout<<c;
        
        std::cout<<'\n';
    }
private:
    static size_t callback(char* p, size_t size, size_t nmemb, void* userdata)
    {
        CurlRequest* instance=static_cast<CurlRequest*>(userdata);
        instance->response_.insert(instance->response_.end(), p, p+nmemb);
        return nmemb;
    }

    static bool curl_initialized;
    CURL* handle_;
    std::string url_;
    std::map<std::string, std::string> params_;
    std::vector<char> response_;
};
bool CurlRequest::curl_initialized=false;

}

#endif //UTIL_HPP