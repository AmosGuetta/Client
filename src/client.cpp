#include <stdlib.h>
#include "../include/connectionHandler.h"
#include "../include/EncoderDecoder.h"
#include <boost/thread.hpp>


using namespace std;

bool connected = false;


ConnectionHandler * connectionHandler;
EncoderDecoder encodec;
void process(string basic_string);

bool badCommand(string basic_string);

void readFromServer() {
    string result;
    char * c = new char[1];

    while(connected && !encodec.shouldTerminate) {
        connectionHandler->getBytes(c);
        result = encodec.decode(c[0]);
        if (result.compare("") != 0)
            process(result);
    }
    delete[] c;
}

void process(string receiveMessage) {
    vector<char> * buffer = new vector<char>();
    size_t index  = receiveMessage.find("SEND ACK");
    size_t index1 = receiveMessage.find("SEND DATA");
    size_t index2 = receiveMessage.find("Upload");
    size_t index3 = receiveMessage.find("TER");
    if(index != string::npos) {
        receiveMessage = receiveMessage.substr(5);
        encodec.encode(buffer,receiveMessage);
        if (!connectionHandler->sendBytes(buffer->data(),buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        }
        buffer->clear();
    }
    else if(receiveMessage.compare("SEND DATA") == 0) {
        encodec.encode(buffer, "DATA");
        if (!connectionHandler->sendBytes(buffer->data(), buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        }
        buffer->clear();
    }
    else  if(index1 != string::npos) {
        cout << "ACK " << receiveMessage.substr(0,index1) << endl;
        encodec.encode(buffer, "DATA");
        if (!connectionHandler->sendBytes(buffer->data(), buffer->size())) {
            cout << "Disconnected. Exiting...\n" << endl;
        }
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
    while(connected && !encodec.shouldTerminate) {
        try {
            const short bufferSize = 1024;
            char buf[bufferSize];
            cin.getline(buf, bufferSize);
            string message(buf);
            if(!badCommand(message))
                message = "a";

            vector<char> *buffer = new vector<char>();
            encodec.encode(buffer, message);

            char sendBuffer[buffer->size()];
            int length = buffer->size();
            for (int i = 0; i < length; i++)
                sendBuffer[i] = buffer->at(i);

            if (!connectionHandler->sendBytes(sendBuffer, length)) {
                break;
            }
            delete buffer;
        }
        catch (errno_t) {}
    }

}

bool badCommand(string command) {
    if(command.substr(0,3).compare("ACK") == 0)
        return false;
    else if(command.compare("SEND DATA") == 0)
        return false;
    else if(command.compare("DATA") == 0)
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
