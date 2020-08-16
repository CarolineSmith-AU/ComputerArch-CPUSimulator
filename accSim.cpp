#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>

//Caroline Smith
//Project 1 - Accumulator based machine

using namespace std;

//function declerations
string readFromMem(int offset, string *memArray, int sizeInBytes);
void writeDataToMem(int offset, string hexValue, string *memArrayPointer, int sizeInBytes);
string eraseFromMem(int offset, string *memArrayPointer, int sizeInBytes);
string hexToDec(string hexVal);
string decToHex(int decVal, int numToPrepend);
string prependZeros(string value, int numToPrepend);
void initDataMemory(ifstream& inStream, string *dataArrayPointer);
int initTextMemory(ifstream& inStream, string *textArrayPointer);
string trimString(string stringToTrim, string charOfChoice);
void disassembleData(ifstream& inStream, ofstream& outStream, map<string, string> &addMap, map<string, string> &sizeMap);
void disassembleText(ifstream& inStream, ofstream& outStream, map<string, string> &addMap, map<string, string> &sizeMap);
void executeCode(int Instruction_Count, string *dataArrayPointer, string *textArrayPointer, int& accumulator);
void load(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator);
void store(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator);
void mult(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator);
void add(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator);

//------------------------------Typedefs------------------------------
//Bit sizing
typedef unsigned int uint8;   //8-bit unsigned integer
typedef unsigned int uint16;  //16-bit unsigned integer
typedef unsigned int uint32;  //32-bit unsigned integer
typedef int int32;            //32-bit signed integer
typedef unsigned int uint48;  //48-bit unsigned integer (size of instruction stored in memory)

//Byte sizing
const uint8 Hex_Digits_In_Byte = 2;   //2 hex digits per 1 byte
const uint16 Hex_Digits_In_Word = 4;  //4 hex digits per 2 byte
const uint32 Hex_Digits_In_DWord = 8; //8 hex digits per 4 byte

//Instruction and stack pointer typedefs
typedef unsigned int Instruction_Pointer;

//Instruction opcode names
typedef uint8 Instruction_Opcode;

//Memory segment base address
typedef uint32 Segement_Base_Address;

//Size of instruction in memory
typedef uint48 Mem_Item_Size;

//Defining typedef constants
const Instruction_Opcode Load = 1;
const Instruction_Opcode Store = 2;
const Instruction_Opcode Mult = 3;
const Instruction_Opcode Add = 4;
const Instruction_Opcode End = 5;
const Segement_Base_Address Data = 512;  //HexVal = 0200h
const Segement_Base_Address Text = 513;  //HexVal = 0201h
const Mem_Item_Size Instruction_Size = 6; 
//const Mem_Item_Size Stack_Data_Size = 8;

//Relevant files
const string SOURCE_FILE = "accCode.s";
const string ENCODED_FILE = "accCodeEncoded.txt";

//Program size variables
int ProgramDataSize = 0;
int ProgramInstructionSize = 0;

int main() {
   //String array typedefs to represent memory segments
   typedef string Data_Segment[1000];
   typedef string Text_Segment[1000];

   //Accumulator
   int32 Accumulator = 0;

   //Empty(uninitialized) memory array declarations
   Data_Segment dataArray;
   Text_Segment textArray;

   //HashMaps
   map<string, string> addMap;  //maps labels to hex value addresses represented as strings
   map<string, string> sizeMap;    //maps labels to their sizes (in bytes)

    //Read source code and translate to binary
    ifstream inStream;
    inStream.open(SOURCE_FILE.c_str(), std::ifstream::in);
    if (inStream.fail()) {
		cout << "Failed opening file." << endl;
		exit(1);
	}
    ofstream outStream;
    outStream.open(ENCODED_FILE.c_str(), ios::app);
    disassembleData(inStream, outStream, addMap, sizeMap);
    inStream.clear();
    inStream.seekg(0, ios::beg);
    disassembleText(inStream, outStream, addMap, sizeMap);
    outStream.close();
    inStream.close();

    //Read disassembled code and load memory
    inStream.open(ENCODED_FILE.c_str(), std::ifstream::in);
    if (inStream.fail()) {
		cout << "Failed to opening file." << endl;
		exit(1);
	}
    initDataMemory(inStream, dataArray);
    int Instruction_Count = initTextMemory(inStream, textArray);
    inStream.close();

    //Execute code instructions
    executeCode(Instruction_Count, dataArray, textArray, Accumulator);

    cout << "\n" << endl;
    cout << "****************************** Program Statistics ******************************" << "\n" << endl;
    cout << "Number of Bits in '.data' segment: " << ProgramDataSize << "\n" << endl;
    cout << "Number of Bits in '.text' segment: " << ProgramInstructionSize << "\n" << endl;
    cout << "Total Bits in Program: " << ProgramDataSize + ProgramInstructionSize << "\n" << endl;
    cout << "Total Bytes in Program: " << (ProgramDataSize + ProgramInstructionSize) / 8 << "\n" << endl;

    return 0;
}

//***Returns the hexadecimal value data located at specified address
//***PARAMS: offset - decimal offset of data in memory segment
//           *memArray - the array corresponding to the memory segment data is stored in
//           sizeInBytes - number of bytes data holds in memory 
string readFromMem(int offset, string *memArray, int sizeInBytes) {
    string dataString = "";
    int MSBPosition = offset + (sizeInBytes - 1);
    for (int i = MSBPosition; i >= offset; i--) {
        dataString += memArray[i];
    }
    return dataString;
}

//***Writes data to memory using little endien order
//***PARAMS: offset - decimal offset of data in memory segment
//           hexValue - hexadecimal value to be stored in memory
//           *memArrayPointer - pointer to the corresponding memory array
//           sizeInBytes - number of bytes data holds in memory 
void writeDataToMem(int offset, string hexVal, string *mArrayPointer, int sizeInBytes) {
    int count = 0;
    int MSBPosition = offset + (sizeInBytes - 1);
    for (int i = MSBPosition; i >= offset; i--) {
        mArrayPointer[i] = hexVal.substr(count, Hex_Digits_In_Byte);
        count += 2;
    }
}

//***Erases data from specified memory address
//***PARAMS: offset - decimal offset of data in memory segment
//           *memArrayPointer - the array corresponding to the memory segment data is stored in
//           sizeInBytes - number of bytes to erase
string eraseFromMem(int offset, string *memArrayPointer, int sizeInBytes) {
    string erasedVal;
    for (int i = sizeInBytes; i > 0; i--) {
        erasedVal = erasedVal.insert(0, memArrayPointer[offset]);
        memArrayPointer[offset] = "";
        offset++;
    }
    return erasedVal;
}

//***Uses binary encoded file to initialize data memory segment
//***PARAMS: instream - an open ifstream
//           *dataArrayPointer - the array corresponding to the memory segment data is stored in
 void initDataMemory(ifstream& inStream, string *dataArrayPointer) {
     string line = "";
     int memOffset; //memory offset from base address
     const string whitespace = " \n\r\t\f\v";
     string hexValueDataSegment = decToHex(Data, 4);
     string hexValueEndInstr = decToHex(End, 2) + "000000000000";

     //Various string positions
     const int hexDigitsInAddr = 8;
     const int startOfAddr = 2;
     const int endOfSizeByte = 2;
     const int startOfValue = 10;
     const int startOfOffset = 6;
     const int hexDigitsInOffset = 4;

    while (line.compare(hexValueDataSegment) != 0) { //loop until ".data" segment is found
         getline(inStream, line);
         line = trimString(line, whitespace);
     }
     if (inStream.eof()) { //".data" section was not found in file
         cout << "End of file reached. .DATA section not found.";
     } else { //.data segment found
         getline(inStream, line);
         line = trimString(line, whitespace);
         while (line.compare(hexValueEndInstr) != 0) { //loop through ".data" segment until ".text" or "eof" is reached
             if (line.compare("") != 0) {
                 int sizeInBytes;
                 string address;
                 string value;

                 //Get data from encoded string
                 sizeInBytes = stoi(hexToDec(line.substr(0, endOfSizeByte)));
                 address = line.substr(startOfAddr, hexDigitsInAddr);
                 value = line.substr(startOfValue);
                 memOffset = stoi(hexToDec(address.substr(startOfOffset, hexDigitsInOffset)));

                 //Save value to data segment of memory
                 writeDataToMem(memOffset, value, dataArrayPointer, sizeInBytes);
                 ProgramDataSize += (sizeInBytes * 8);
             }
             getline(inStream, line);
             line = trimString(line, whitespace);
         }
     }
 }

//***Returns number of instructions after reading binary encoded file and initializinng text memory segment
//***PARAMS: instream - an open ifstream
//           *textArrayPointer - the array corresponding to the memory segment data is stored in
 int initTextMemory(ifstream& inStream, string *textArrayPointer) {
     int instrCount = 0;
     string line = "";
     int memOffset = 0; //memory offset from base address
     const string whitespace = " \n\r\t\f\v";
     string hexValueTextSegment = decToHex(Text, 4);
     string hexValueEndInstr = decToHex(End, 2) + "0000000000";

     //Various string positions
     const int hexDigitsInAddr = 8;
     const int startOfAddr = 2, endOfByteSize = 2, endOfInstrType = 2;
     const int startOfValue = 10;
     const int startOfOffset = 6;
     const int hexDigitsInOffset = 4;

    while (line.compare(hexValueTextSegment) != 0) { //loop until ".text" segment is found
         getline(inStream, line);
         line = trimString(line, whitespace);
     }
     if (inStream.eof()) { //".text" section was not found in file
         cout << "End of file reached. .DATA section not found.";
     } else { //.data segment found
         getline(inStream, line);
         line = trimString(line, whitespace);
         while (line.compare(hexValueEndInstr) != 0) { //loop through ".text" segment until END is reached
             if (line.compare("") != 0) {
                 //Save instruction to text segment of memory
                 writeDataToMem(memOffset, line, textArrayPointer, Instruction_Size);  
                 ProgramInstructionSize += (Instruction_Size * 8);
                 instrCount++;           
             }
             getline(inStream, line);
             line = trimString(line, whitespace);
             memOffset += Instruction_Size;
         }
         writeDataToMem(memOffset, line, textArrayPointer, Instruction_Size);
         ProgramInstructionSize += (Instruction_Size * 8);
         instrCount++; 
     }
     return instrCount;
 }

//***Reads ASCII source code file to create data segment of encoded file
//***PARAMS: instream - an open ifstream
//           outStream - an open ofstream
//           &addMap - map of labels and corresponding addresses
//           &sizeMap - map of labels and corresponding sizes (in bytes)
 void disassembleData(ifstream& inStream, ofstream& outStream, map<string, string> &addMap, map<string, string> &sizeMap) {
     string line = "";
     int memOffset = 0; //memory offset from base address
     const string whitespace = " \n\r\t\f\v";

     while (line.compare(".data") != 0) { //loop until ".data" segment is found
         getline(inStream, line);
         line = trimString(line, whitespace);
     }
     if (inStream.eof()) { //".data" section was not found in file
         cout << "End of file reached. .DATA section not found.";
     } else { //.data segment found
         outStream << decToHex(Data, 4) + "\n";
         getline(inStream, line);
         line = trimString(line, whitespace);
         while (line.compare("END") != 0) { //loop through ".data" segment until ".text" or "eof" is reached
             if (line.compare("") != 0) {
                 string label;
                 string address;
                 string sizeString;
                 string size;
                 string value;
                 string encodedData;

                 //Get memory address label 
                 label = line.substr(0, line.find_first_of(":"));
                 label = trimString(label, whitespace);
                 address = decToHex(Data, 4) + decToHex(memOffset, 4);
                 addMap[label] = address;

                 //Get size of label in bytes
                 sizeString = line.substr(line.find_first_of(":") + 1, sizeof(line));
                 sizeString = trimString(sizeString, whitespace);
                 value = sizeString.substr(sizeString.find_first_of(whitespace), sizeString.find_last_not_of(whitespace) + 1);
                 value = trimString(value, whitespace);
                 sizeString = sizeString.substr(0, sizeString.find_first_of(whitespace));
                 if (sizeString.compare(".byte") == 0) { //byte => 1 byte
                     size = "1";
                 } else if (sizeString.compare(".word") == 0) { //word => 2 bytes
                     size = "2";
                 } else { //doubleword => 4 bytes
                     size = "4";
                 }
                 sizeMap[label] = prependZeros(size, 1);
                if (value.compare("?") == 0) {
                    encodedData = prependZeros(size, 1) + address + decToHex(0, 4);
                } else {
                    encodedData = prependZeros(size, 1) + address + decToHex(stoi(value), stoi(size) * 2);
                }
                outStream << encodedData + "\n";
                memOffset += stoi(size);
             }
             getline(inStream, line);
             line = trimString(line, whitespace);
         }
         outStream << prependZeros(to_string(End), 1) + "000000000000" + "\n";
     }
 }

//***Reads ASCII source code file to creaate text segment of encoded file
//***PARAMS: instream - an open ifstream
//           outStream - an open ofstream
//           &addMap - map of labels and corresponding addresses
//           &sizeMap - map of labels and corresponding sizes (in bytes)
 void disassembleText(ifstream& inStream, ofstream& outStream, map<string, string> &addMap, map<string, string> &sizeMap) {
     string line = "";
     int memOffset = 0; //memory offset from base address
     const string whitespace = " \n\r\t\f\v";

     while (line.compare(".text") != 0) { //loop until ".text" segment is found
         getline(inStream, line);
         line = trimString(line, whitespace);
     }
     if (inStream.eof()) { //".text" section was not found in file
         cout << "End of file reached. .DATA section not found.";
     } else { //.data segment found
         outStream << decToHex(Text, 4) + "\n";
         getline(inStream, line);
         line = trimString(line, whitespace);
         while (line.compare("main:") != 0) { //loop until "main:" code segment is found
             getline(inStream, line);
             line = trimString(line, whitespace);
         }
         getline(inStream, line);
         line = trimString(line, whitespace);
         while (line.compare("END") != 0) { //loop through ".text" segment until END is reached
             if (line.compare("") != 0) {
                 string encodedData;
                 string instruction = line.substr(0, line.find_first_of(whitespace));
                 string label = line.substr(line.find_first_of(whitespace) + 1);
                 line = trimString(line, whitespace);
                 if (instruction.compare("load") == 0) { //if instruction is "load"
                     encodedData = prependZeros(to_string(Load), 1) + sizeMap[label] + addMap[label];
                 } else if (instruction.compare("mult") == 0) { //if instruction is "mult"
                     encodedData = prependZeros(to_string(Mult), 1) + sizeMap[label] + addMap[label];
                 } else if (instruction.compare("add") == 0) { //if instruction is "add"
                     encodedData = prependZeros(to_string(Add), 1) + sizeMap[label] + addMap[label];
                 } else { //if instruction is "sore"
                     encodedData = prependZeros(to_string(Store), 1) + sizeMap[label] + addMap[label];
                 }
                 outStream << encodedData + "\n";
                 memOffset += 1;
             }
             getline(inStream, line);
             line = trimString(line, whitespace);
         }
         outStream << prependZeros(to_string(End), 1) + "0000000000" + "\n";
     }
 }

//***Executes code using accumulator implementation
//***PARAMS: Instruction_Count - holds number of instructions to execute
//           *dataArrayPointer - references data memory segment array
//           *textArrayPointer - references text memory segment array
//           &accumulator - holds the value of the accumulator
 void executeCode(int Instruction_Count, string *dataArrayPointer, string *textArrayPointer, int& accumulator) {
     Instruction_Pointer pc = 0;
     
     const int Mem_Offset_Opcode = 5;       //the offset of opcode byte from instruction pointer (pc)
     const int Mem_Offset_Size = 4;         //the offset of size byte from instruction pointer (pc)
     const int Mem_Offset_Base_Address = 3; //the offset of base address from instruction pointer (pc)
     const int Mem_Offset_Offset = 1;       //the offset of memory offset from instruction pointer (pc)

     int opCode;
     int byteSize;
     int baseAddress;
     int offset;
     int value;
     
     while (Instruction_Count != 0) {
         opCode = stoi(hexToDec(textArrayPointer[pc + Mem_Offset_Opcode]));
         byteSize = stoi(hexToDec(textArrayPointer[pc + Mem_Offset_Size]));
         baseAddress = stoi(hexToDec(textArrayPointer[pc + Mem_Offset_Base_Address] + textArrayPointer[pc + (Mem_Offset_Base_Address - 1)]));
         offset = stoi(hexToDec(textArrayPointer[pc + Mem_Offset_Offset] + textArrayPointer[pc + (Mem_Offset_Offset - 1)]));

         //Decode instruction opcode
         switch (opCode) {
         case Load:
             if (baseAddress == Data) { //check if base address equals the data segment base address constant
                 load(dataArrayPointer, offset, byteSize, accumulator);
                 value = stoi(hexToDec(readFromMem(offset, dataArrayPointer, byteSize)));
                 cout << "Loaded " << value << " into accumulator.\n" << endl;
             } else { //otherwise, illegal memory segment request
                 //illegal
             }             
             break;
         case Store:
            if (baseAddress == Data) { //check if base address equals the data segment base address constant
                 store(dataArrayPointer, offset, byteSize, accumulator);
                 value = stoi(hexToDec(readFromMem(offset, dataArrayPointer, byteSize)));
                 cout << "Stored " << value << " into memory." << "\n" << endl;
             } else { //otherwise, illegal memory segment request
                 //illegal
             }
             break;
         case Mult:
            if (baseAddress == Data) { //check if base address equals the stack segment base address constant
                 mult(dataArrayPointer, offset, byteSize, accumulator);
                 value = stoi(hexToDec(readFromMem(offset, dataArrayPointer, byteSize)));
                 cout << "Multiplied " << value << " by value in accumulator - stored product in accumulator.\n" << endl;
             } else { //otherwise, illegal memory segment request
                 //illegal
             }
            break;
         case Add:
            if (baseAddress == Data) { //check if base address equals the stack segment base address constant
                 add(dataArrayPointer, offset, byteSize, accumulator);
                 value = stoi(hexToDec(readFromMem(offset, dataArrayPointer, byteSize)));
                 cout << "Added " << value << " to value in accumulator - stored sum in accumulator.\n" << endl;
             } else { //otherwise, illegal memory segment request
                 //illegal
             }
            break;
         default:
             break;
         }

         pc += Instruction_Size;
         Instruction_Count--;
     }
 }

//----------------------------------------Instruction Methods----------------------------------------//

//***Loads data at specified address into accumulator
void load(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator) {
    int memData = stoi(hexToDec(readFromMem(offset, dataArrayPointer, sizeInBytes)));
    accumulator = memData;
}

//***Stores value of accumuator into the specified address
void store(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator) {
    string hexValAcc = decToHex(accumulator, sizeInBytes * Hex_Digits_In_Byte);
    writeDataToMem(offset, hexValAcc, dataArrayPointer, sizeInBytes);
}

//***Multiplies value at specified address by the accumulator - stores product in accumulator
void mult(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator) {
    int memData = stoi(hexToDec(readFromMem(offset, dataArrayPointer, sizeInBytes)));
    accumulator *= memData;
}

//***Adds value at specified address by the accumulator - stores sum in accumulator
void add(string *dataArrayPointer, int offset, int sizeInBytes, int& accumulator) {
    int memData = stoi(hexToDec(readFromMem(offset, dataArrayPointer, sizeInBytes)));
    accumulator += memData;
}

//----------------------------------------Helper Methods----------------------------------------//

//***Returns string that has been trimmed of whitespace
 string trimString(string stringToTrim, string charOfChoice) {
     stringToTrim.erase(0, stringToTrim.find_first_not_of(charOfChoice));
     stringToTrim.erase(stringToTrim.find_last_not_of(charOfChoice) + 1);
     return stringToTrim;
 }

//***Converst hexadecimal value to decimal
 string hexToDec(string hexVal) {
    std::stringstream stream;
    int decValue = 0;
    stream << hexVal;
    stream >> std::hex >> decValue;
    return to_string(decValue);
}

//***Converts decimal value to hexadecimal and prepends zeros based on specified hexValSize
string decToHex(int decVal, int hexValSize) {
    std::stringstream stream;
    stream << std::hex << decVal;
    string rando = stream.str();
    return prependZeros(stream.str(), hexValSize - stream.str().length());
}

//***Prepends zero to a string
string prependZeros(string value, int numToPrepend) {
    string zeros = "";
    for (int i = 0; i < numToPrepend; i++) {
        zeros += "0";
    }
    return value.insert(0, zeros);
}