#include "../include/EncoderDecoder.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <codecvt>

using namespace std;

EncoderDecoder::EncoderDecoder(): requestFileName(), writeFileName(),packetsToRead(), currentIndex(), bytes(), currentOpcode(),shouldTerminate(false),loggedin(false),filesize(),currentBlockNumber(1),infile(),counterBytesSent(0)
{
}

EncoderDecoder::EncoderDecoder(const EncoderDecoder& e): requestFileName(e.requestFileName),writeFileName(e.writeFileName),packetsToRead(e.packetsToRead), currentIndex(e.currentIndex),
                                                         bytes(e.bytes), currentOpcode(e.currentOpcode), shouldTerminate(e.shouldTerminate), loggedin(e.loggedin), filesize(e.filesize),
                                                         currentBlockNumber(e.currentBlockNumber), infile(e.infile), counterBytesSent(e.counterBytesSent) {
}

EncoderDecoder & EncoderDecoder::operator=(const EncoderDecoder& e) {
    return *this;
}

string EncoderDecoder::decode(char nextByte) {
    bytes.push_back(nextByte);
    short blocknumber;
    short packetsize;
    vector<char> finalBytes;
    short error_code;

    if(currentIndex < 2) {
        currentIndex++;
        return "";
    }

    if(currentIndex == 2)
        currentOpcode = (short)((bytes.at(0) & 0xff) << 8) + (short)(bytes.at(1) & 0xff);

    switch(currentOpcode) {
        case (short) 3:
            if (currentIndex < 5) {
                currentIndex++;
                return "";
            }

            packetsize = (short) ((bytes.at(2) & 0xff) << 8) + (short) (bytes.at(3) & 0xff);
            if (currentIndex < packetsize + 5 && packetsize != 0) {
                currentIndex++;
                return "";
            }
            blocknumber = (short) ((bytes.at(4) & 0xff) << 8) + (short) (bytes.at(5) & 0xff);
            for (int i = 0; i < packetsize; i++)
                packetsToRead.push_back(*bytes.erase(bytes.begin() + 5));
            if (packetsize < 512) {
                string s1;
                if (requestFileName.compare("") == 0) {
                    if(packetsize == 0)
                        s1 = "";
                    else
                        s1 = parseDataToDirString(packetsToRead);
                    packetsToRead.clear();
                    currentOpcode = -1;
                    bytes.clear();
                    currentIndex = 0;
                    finalBytes.clear();
                    return s1;
                } else {
                    s1 = parseDataToFile(packetsToRead);
                    packetsToRead.clear();
                    currentOpcode = -1;
                    bytes.clear();
                    currentIndex = 0;
                    finalBytes.clear();
                    requestFileName.clear();
                    return s1;
                }
            } else {
                currentOpcode = -1;
                bytes.clear();
                currentIndex = 0;
                finalBytes.clear();
                return "SEND ACK " + to_string(blocknumber);
            }

        case (short) 4:
            if (currentIndex < 3) {
                currentIndex++;
                return "";
            }

            blocknumber = (short) ((bytes.at(2) & 0xff) << 8) + (short) (bytes.at(3) & 0xff);
            if (blocknumber == 0 && writeFileName.compare("") == 0) {
                currentOpcode = -1;
                bytes.clear();
                currentIndex = 0;
                if(!loggedin)
                    loggedin = true;
                if(!shouldTerminate)
                    return "ACK " + to_string(blocknumber);
                else
                    return "TER ACK " + to_string(blocknumber);
            }

            if (writeFileName.compare("") != 0) {

                if (filesize + 1 == counterBytesSent) {
                    string s = to_string(blocknumber) + " WRQ " + writeFileName + " complete";
                    writeFileName = "";
                    currentOpcode = -1;
                    bytes.clear();
                    currentIndex = 0;
                    currentBlockNumber = 1;
                    counterBytesSent = 0;
                    infile->close();
                    return s;
                } else {

                    currentOpcode = -1;
                    bytes.clear();
                    currentIndex = 0;
                    return to_string(blocknumber) + " SEND DATA";
                }
            }


            currentOpcode = -1;
            bytes.clear();
            currentIndex = 0;
            return "";

        case (short) 5:

            if (currentIndex < 4 || bytes.at(currentIndex) != '\0') {
                currentIndex++;
                return "";
            }

            error_code = (short) ((bytes.at(2) & 0xff) << 8) + (short) (bytes.at(3) & 0xff);
            if(error_code == 0) {
                packetsToRead.clear();
                finalBytes.clear();
                requestFileName.clear();
                writeFileName = "";
                currentBlockNumber = 1;
                counterBytesSent = 0;
                infile->close();
                currentOpcode = -1;
                bytes.clear();
                currentIndex = 0;
                packetsToRead.clear();
                requestFileName = "";
                return "ERROR 0";
            }

            currentOpcode = -1;
            bytes.clear();
            currentIndex = 0;
            packetsToRead.clear();
            requestFileName = "";
            writeFileName = "";
            return "ERROR " + to_string(error_code);

        case (short) 9:
            if (currentIndex < 3 || bytes.at(currentIndex) != '\0') {
                currentIndex++;
                return "";
            }

            string filename = "";
            for (unsigned int i = 3; i < bytes.size(); i++)
                filename += bytes.at(i);

            if (bytes.at(2) == '\0')
                filename = "del " + filename;
            else
                filename = "add " + filename;
            currentOpcode = -1;
            bytes.clear();
            currentIndex = 0;
            return "BCAST " + filename;
    }
    return "";
}

void EncoderDecoder::encode(vector<char> * outBytes, string line) {
    string opcode = "";
    outBytes->clear();
    size_t index = line.find(" ");
    if(index != string::npos) {
        opcode = line.substr(0, index);
        line = line.substr(index + 1);
    }
    else
        opcode = line;

    if(opcode.compare("RRQ") == 0) {
        requestFileName = line;
        outBytes->push_back((((short) 1) >> 8) & 0xFF);
        outBytes->push_back(((short) 1) & 0xFF);
        for (unsigned int i = 0; i < line.length(); i++)
            outBytes->push_back(line[i]);
        outBytes->push_back('\0');
    }
    else if(opcode.compare("WRQ") == 0) {
        writeFileName = line;
        if(!writefile()) {
            outBytes->push_back((((short) 5) >> 8) & 0xFF);
            outBytes->push_back(((short) 5) & 0xFF);
            outBytes->push_back('\0');
            infile->close();
            filesize = 0;
            writeFileName = "";
        }
        else {
            outBytes->push_back((((short) 2) >> 8) & 0xFF);
            outBytes->push_back(((short) 2) & 0xFF);
            for (unsigned int i = 0; i < line.length(); i++)
                outBytes->push_back(line[i]);
            outBytes->push_back('\0');
        }
    }
    else if(opcode.compare("DATA") == 0) {
        int packetsize = min(512,filesize - (512 * (currentBlockNumber - 1)));

        outBytes->push_back((((short) 3) >> 8) & 0xFF);
        outBytes->push_back(((short) 3) & 0xFF);

        outBytes->push_back(((short)packetsize >> 8) & 0xFF);
        outBytes->push_back((short)packetsize & 0xFF);

        outBytes->push_back((currentBlockNumber >> 8) & 0xFF);
        outBytes->push_back(currentBlockNumber & 0xFF);

        if(packetsize > 0) {
            infile->seekg((currentBlockNumber - 1) * 512, ios::beg);

            char *buf = new char[packetsize];
            infile->read(buf, packetsize);


            for (int i = 0; i < packetsize; i++) {
                outBytes->push_back(buf[i]);
            }
            delete buf;
        }

        currentBlockNumber++;
        if(packetsize != 512)
            counterBytesSent++;
        counterBytesSent += packetsize;
    }

    else if(opcode.compare("ACK") == 0) {
        outBytes->push_back((((short)4) >> 8) & 0xFF);
        outBytes->push_back(((short)4) & 0xFF);
        int block = atoi(line.c_str());
        outBytes->push_back((((short)block) >> 8) & 0xFF);
        outBytes->push_back(((short)block) & 0xFF);
    }
    else if(opcode.compare("DIRQ") == 0) {
        outBytes->push_back((((short)6) >> 8) & 0xFF);
        outBytes->push_back(((short)6) & 0xFF);
    }
    else if(opcode.compare("LOGRQ") == 0) {

        outBytes->push_back((((short) 7) >> 8) & 0xFF);
        outBytes->push_back(((short) 7) & 0xFF);
        for (unsigned int i = 0; i < line.length(); i++)
            outBytes->push_back(line[i]);
        outBytes->push_back('\0');
    }
    else if(opcode.compare("DELRQ") == 0) {
        outBytes->push_back((((short)8) >> 8) & 0xFF);
        outBytes->push_back(((short)8) & 0xFF);
        for (unsigned int i = 0; i < line.length(); i++)
            outBytes->push_back(line[i]);
        outBytes->push_back('\0');
    }
    else if(opcode.compare("DISC") == 0) {
        outBytes->push_back((((short)10) >> 8) & 0xFF);
        outBytes->push_back(((short)10) & 0xFF);
        if(loggedin)
            shouldTerminate = true;
    }
    else {
        outBytes->push_back((((short) 11) >> 8) & 0xFF);
        outBytes->push_back(((short) 11) & 0xFF);
    }
}

string EncoderDecoder::parseDataToDirString(vector<char> outBytes) {
    string s = "";
    for(unsigned int i = 0; i < packetsToRead.size() - 1; i++) {
        if(packetsToRead.at(i) == '\0')
            s += "\n";
        else
            s += packetsToRead.at(i);
    }
    currentOpcode = -1;
    bytes.clear();
    currentIndex = 0;
    return s;
}

string EncoderDecoder::parseDataToFile(vector<char> outBytes) {
    ofstream outfile(requestFileName, ofstream::binary);
    outfile.write(outBytes.data(),outBytes.size());
    outfile.flush();
    outfile.close();
    string s = "RRQ " + requestFileName + " complete";
    requestFileName = "";
    currentOpcode = -1;
    bytes.clear();
    currentIndex = 0;
    return s;
}

bool EncoderDecoder::writefile() {
    infile = new ifstream(writeFileName, ifstream::binary);
    infile->seekg(0,infile->end);
    filesize = infile->tellg();
    if (!infile->good())
        return false;
    return true;
}