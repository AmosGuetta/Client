#include <stdlib.h>
#include "../include/connectionHandler.h"
#include "../include/EncoderDecoder.h"
#include <boost/thread.hpp>
#include <vector>

using namespace std;

bool connected = false;
bool shouldTerminate = false;

ConnectionHandler * connectionHandler;
EncoderDecoder encodec;
void process(string basic_string);

bool badCommand(string basic_string);

void readFromServer() {
    string result;
    char * c = new char[1];

    while(connected) {
        connectionHandler->getBytes(c);
        result = encodec.decode(c[0]);
        if (result.compare("") != 0)
            process(result);
    }
    delete[] c;
}

void process(string receiveMessage) {
    vector<char> * buffer = new vector<char>();
    size_t index = receiveMessage.find("SEND ACK");
    size_t index1 = receiveMessage.find(" SEND DATA");
    size_t index2 = receiveMessage.find("WRQ");
    size_t index3 = receiveMessage.find("TER");
    if(index != string::npos) {
        receiveMessage = receiveMessage.substr(5);
        encodec.encode(buffer,receiveMessage);
        if (!connectionHandler->sendBytes(buffer->data(),buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        }
        //else
          //  cout << "Sent " << receiveMessage.length() << " bytes to server" << endl;
        buffer->clear();
    }
    else if(receiveMessage.compare("SEND DATA") == 0) {
        encodec.encode(buffer, "DATA");
        if (!connectionHandler->sendBytes(buffer->data(), buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        } //else
            //cout << "Sent " << receiveMessage.length() << " bytes to server" << endl;
        buffer->clear();
    }
    else  if(index1 != string::npos) {
        cout << "ACK " << receiveMessage.substr(0,index1) << endl;
        encodec.encode(buffer, "DATA");
        if (!connectionHandler->sendBytes(buffer->data(), buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        } //else
            //cout << "Sent " << receiveMessage.length() << " bytes to server" << endl;
        buffer->clear();
    }
    else if(index2 != string::npos) {
        cout << "ACK " << receiveMessage.substr(0,receiveMessage.find_first_of(' ')) << endl;
        cout << receiveMessage.substr(receiveMessage.find_first_of(' ') + 1) << endl;
    }
    else if(index3 != string::npos) {
        cout << receiveMessage.substr(receiveMessage.find_first_of(' ') + 1) << endl;
        connected = false;
    }
    else {
        cout << receiveMessage << endl;
    }
    delete buffer;
}

void writeToServer() {
    while(connected) {
        try {
            const short bufsize = 1024;
            char buf[bufsize];
            cin.getline(buf, bufsize);
            string line(buf);
            if(!badCommand(line))
                line = "a";

            vector<char> *buffer = new vector<char>();
            encodec.encode(buffer, line);

            char sendbuf[buffer->size()];
            int length = buffer->size();
            for (int i = 0; i < length; i++)
                sendbuf[i] = buffer->at(i);

            if (!connectionHandler->sendBytes(sendbuf, length)) {
                cout << "Disconnected. Exiting...\n" << endl;
                break;
            }
            delete buffer;
        }
        catch (errno_t) {}
    }

}

bool badCommand(string line) {
    if(line.substr(0,3).compare("ACK") == 0)
        return false;
    else if(line.compare("SEND DATA") == 0)
        return false;
    else if(line.compare("DATA") == 0)
        return false;
    else
        return true;
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " host port" << endl << endl;
        return -1;
    }
    string host = argv[1];
    short port = atoi(argv[2]);

    connectionHandler = new ConnectionHandler(host,port);
    if (!connectionHandler->connect()) {
        cerr << "Cannot connect to " << host << ":" << port << endl;
        return 1;
    }

    connected = true;
    boost::thread readThread(readFromServer);
    boost::thread writeThread(writeToServer);
    readThread.join();
    writeThread.join();
    connectionHandler->close();
    delete connectionHandler;
    return 0;
}
