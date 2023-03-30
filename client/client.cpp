//boost 1.81

#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <string>
#include <stdio.h>


using namespace std;
using boost::asio::ip::tcp;

using namespace std;
string nowtime()
{
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    return (to_string(now->tm_mday) + "." + to_string(now->tm_mon) + "." + to_string(now->tm_year + 1900) +
        " " + to_string(now->tm_hour) + ":" + to_string(now->tm_min) + ":" + to_string(now->tm_sec));
}

int logger(string logstr)
{
    ofstream log("log_client.txt", ios::app);

    log << nowtime() << ": " << logstr << endl;
    log.close();
    return 0;
}

string ansStringFromJson(const char req[])
{
    boost::json::error_code errorCode;
    string str = req;
    boost::json::value jsonValue = boost::json::parse(str, errorCode);

    auto jsonString = jsonValue.as_object()["string"].as_string();

    std::string myString = boost::json::serialize(jsonString);
    myString.erase(0, 1);
    myString.erase(myString.length() - 1, 1);

    return myString;
}


class client
{
public:
    client(boost::asio::io_context& io_context, const string& ip_add, const string& port, char* msg) : resolver_(io_context), socket_(io_context)
    
    {
        tcp::resolver::results_type endpoints = resolver_.resolve(tcp::v4(), ip_add, port);
        strcpy(answer, msg);
        async_connect(socket_, endpoints,boost::bind(&client::handle_connect, this, boost::asio::placeholders::error));
    }

private:
  

    void handle_connect(const boost::system::error_code& err)
    {
        if (!err)
        {
            logger ("successful connection");
            boost::asio::async_write(socket_,
                boost::asio::buffer(answer, max_length),
                boost::bind(&client::handle_write, this,
                    boost::asio::placeholders::error));

        }
        else
        {
            logger("connection error");
            std::cout << "Error: " << err.message() << "\n";
        }
    }

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error)
        {
            logger("successful reading");
            ans = ansStringFromJson(answer);
            cout << ans<< endl;
            socket_.close();
        }
        else
        {
            logger("reading error");
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {

        if (!error)

        {
            logger("successful writing");
            socket_.async_read_some(boost::asio::buffer(answer, max_length),
                boost::bind(&client::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
           
        }
        else
        {
            logger("writing error");
            delete this;
        }
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    enum { max_length = 1024 };
    char answer[1024];
    string ans;

};

int main(int argc, char* argv[])
{

    try
    {

        if (argc != 3)
        {
            std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
            logger("No arguments cmd");
            return 1;
        }

        

        enum { max_length = 1024 };
        std::cout << "Enter message: ";
        char str[max_length];
        std::cin.getline(str, max_length);
        string chek;
        chek = str;
        
        char req[max_length] = "{\"string\":\"";
        strcat(req,str);
        strcat(req, "\"}");

        boost::asio::io_context io_context;

        client client(io_context, argv[1], argv[2], req);

        io_context.run();
    }
    catch (std::exception& e)
    {
        string str="exception ";
        str += e.what();
        logger(str);
    }

    return 0;
}