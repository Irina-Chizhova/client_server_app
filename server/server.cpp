//boost 1.81

#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <windows.h>
#include <boost/json.hpp>
#include <boost/json/value.hpp>
#include "tinyexpr.h"
#include <ctime>
#include <fstream>

using namespace std;
using boost::asio::ip::tcp;
using boost::json::value;

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
	ofstream log("log_server.txt", ios::app);

	log << nowtime() << ": " << logstr << endl;
	log.close();
	return 0;
}


string reqStringFromJson(const char req[])
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

string calculate(const char req[])
{
	string str = reqStringFromJson(req);
	double answer = te_interp(str.c_str(), 0);
	str = to_string(answer); \
	return str;
}

class session
{
public:
	session(boost::asio::io_context& io_context)
		: socket_(io_context)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		logger("successful starting");
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if (!error)
		{
			logger("successful reading");
			char ans[max_length] = "{\"string\":\"";
			strcat(ans, calculate(data_).c_str());
			strcat(ans, "\"}");
			strcpy(data_, ans);

			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, max_length),
				boost::bind(&session::handle_write, this,
					boost::asio::placeholders::error));
	
		}
		else
		{
			logger("");
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error)
	{

		if (!error)

		{
			logger("successful writing");
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
				boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			logger("writing error");
			delete this;
		}
	}

	tcp::socket socket_;
	boost::json::string message;
	enum { max_length = 1024 };
	char data_[max_length];
	char answer[1024];
	std::string my_string;
};

class server
{
public:
	server(boost::asio::io_context& io_context, short port)
		: io_context_(io_context),
		acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		start_accept();
	}

private:
	void start_accept()
	{
		session* new_session = new session(io_context_);
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
				boost::asio::placeholders::error));
	}

	void handle_accept(session* new_session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			logger("starting session successful");
			new_session->start();
		}
		else
		{
			logger("starting session error");
			delete new_session;
		}

		start_accept();
	}

	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
	try
	{

		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			logger("No arguments cmd");
			return 1;
		}

		boost::asio::io_context io_context;

		using namespace std; // For atoi.
		server s(io_context, atoi(argv[1]));

		io_context.run();
	}
	catch (std::exception& e)
	{
		string str = "exception ";
		str += e.what();
		logger(str);
	}

	return 0;
}