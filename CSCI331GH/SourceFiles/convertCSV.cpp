#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "convertCSV.h"
#include "readBinaryFile.h"
#include "HeaderBuffer.h"
#include "IndexManager.h"
#include <algorithm>
#include <cctype>
using namespace std;

//Authors: Team 5
//Date: 2025-10-11
/*Purpose: This file contains the implementation of functions to read a CSV file and
 write its contents to a binary file with length-prefixed records. */

void processFile(string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName, ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening files in processFile." << endl;
        return;
    }

    HeaderRecordBuffer header;
    
    // Set all the metadata
    header.version = 2; 
    header.indexFileName = "Data/zip.idx";
    header.primaryKeyFieldIndex = 0; 
    
    
    header.fields.push_back({"ZipCode", DataType::STRING});
    header.fields.push_back({"PlaceName", DataType::STRING});
    header.fields.push_back({"State", DataType::STRING});
    header.fields.push_back({"County", DataType::STRING});
    header.fields.push_back({"Latitude", DataType::DOUBLE});
    header.fields.push_back({"Longitude", DataType::DOUBLE});

    uint32_t recordCount = 0;
    string tempLine;
    getline(inputFile, tempLine); 
    while (std::getline(inputFile, tempLine)) {
        if (!tempLine.empty()) {
            ++recordCount;
        }
    }
    header.recordCount = recordCount; // Set the count
 
    header.writeHeader(outputFile);

    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);
    getline(inputFile, tempLine); 

    string line;
    while (getline(inputFile, line)) {
        if (!line.empty()) {
            lenRead(outputFile, line); 
        }
    }
    

    IndexManager index;
    index.buildIndex(outputFileName); 
    index.writeIndex(header.indexFileName); // Use the filename from the header

    inputFile.close();
    outputFile.close();
    cout << "Binary file and index created successfully with new header format." << endl;
}


void lenRead(ofstream& output, const string& record) {
    uint32_t recordLength = static_cast<uint32_t>(record.length());
    output.write(reinterpret_cast<const char*>(&recordLength), sizeof(recordLength)); // length prefix (uint32_t)
    output.write(record.c_str(), recordLength);
}

void binaryToCSV() {
    string inputCSVFileName = "Data/us_postal_codes.csv";
    string binaryFile = "Data/newBinaryPCodes.dat";
	string outputCSVFile = "Data/converted_postal_codes.csv";

    processFile(inputCSVFileName, binaryFile);
	readBinaryFile(binaryFile, outputCSVFile);

}
