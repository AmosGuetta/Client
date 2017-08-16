#include "../include/connectionHandler.h"

using boost::asio::ip::tcp;
using namespace std;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

ConnectionHandler::ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(), socket_(io_service_) {
}

ConnectionHandler::~ConnectionHandler() {
    close();
}

void ConnectionHandler::close() {
    socket_.close();
}

bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " 
        << host_ << ":" << port_ << std::endl;
    try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getBytes(char bytes[]) {
	boost::system::error_code error;
    try {
        if(!error) {
			socket_.read_some(boost::asio::buffer(bytes,1), error);
           // cout << "Read: " << (int)bytes[0] << endl;
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp) {
//            cout << "number of bytes: " << bytesToWrite << endl;
//            for(int i = 0; i < bytesToWrite; i++)
//                cout << ": " << (int)bytes[i] << endl;
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if (error)
            throw boost::system::system_error(error);
    } catch (std::exception &e) {
        return false;
    }
    return true;
}