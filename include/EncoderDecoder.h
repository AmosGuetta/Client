#ifndef BOOST_ECHO_CLIENT_ENCODERDECODER_H
#define BOOST_ECHO_CLIENT_ENCODERDECODER_H
#include <iostream>
#include <vector>

using namespace std;

class EncoderDecoder {

public:
    EncoderDecoder();
    EncoderDecoder(const EncoderDecoder& e);
    EncoderDecoder & operator=(const EncoderDecoder& e);
    void encode (vector<char> * outBytes, string line);
    string decode (char nextByte);
    string requestFileName;
    string writeFileName;
    vector<char> packetsToRead;
    int currentIndex;
    vector<char> bytes;
    short currentOpcode;
    bool shouldTerminate;
    bool loggedin;
    int filesize;
    short currentBlockNumber;
    ifstream * infile;
    int counterBytesSent;

    string parseDataToDirString(vector<char> bytess);

    string parseDataToFile(vector<char> bytess);

    bool writefile();
};

#endif
