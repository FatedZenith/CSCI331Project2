// In SourceFiles/readBinaryFile.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "HeaderBuffer.h"
#include "readBinaryFile.h"
#include "ZipCodeRecordBuffer.h"

using namespace std;

void readBinaryFile(const string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName, ios::binary);
    ofstream outputFile(outputFileName);

    if (!inputFile.is_open()) {
        cerr << "Error opening binary file!" << endl;
        return;
    }
    if (!outputFile.is_open()) {
        cerr << "Error opening CSV file!" << endl;
        return;
    }

    // Read header first
    HeaderRecordBuffer header;
    if (!header.readHeader(inputFile)) {
        cerr << "Failed to read header record!" << endl;
        return;
    }

    cout << "--- File Header Information ---" << endl;
    cout << "File Type:       " << header.fileStructureType << endl;
    cout << "Version:         " << header.version << endl;
    cout << "Record count:    " << header.recordCount << endl;
    cout << "Creation date:   " << header.creationDate << endl;
    cout << "Index File:      " << header.indexFileName << endl;
    cout << "--- Field Schema ---" << endl;
    for(size_t i = 0; i < header.fields.size(); ++i) {
        cout << "  - Field " << i << ": " << header.fields[i].fieldName;
        if (i == header.primaryKeyFieldIndex) cout << " (Primary Key)";
        cout << endl;
    }
    cout << "-----------------------------" << endl << endl;

    // Write CSV header row
    outputFile << "RecordLength,ZipCode,PlaceName,State,County,Latitude,Longitude\n";

    // Read all records according to header record count
    for (uint32_t i = 0; i < header.recordCount; ++i) {
        uint32_t recordLength;
        if (!inputFile.read(reinterpret_cast<char*>(&recordLength), sizeof(recordLength))) {
            cerr << "Error reading record length!" << endl;
            break;
        }

        string record(recordLength, '\0');
        if (!inputFile.read(&record[0], recordLength)) {
            cerr << "Error reading record data!" << endl;
            break;
        }

        ZipCodeRecordBuffer buffer;
        istringstream ss(record);
        if (buffer.ReadRecord(ss)) {
            outputFile << recordLength << ","
                << buffer.getZipCode() << ","
                << buffer.getPlaceName() << ","
                << buffer.getState() << ","
                << buffer.getCounty() << ","
                << buffer.getLatitude() << ","
                << buffer.getLongitude() << "\n";
        } else {
            std::string head = record;
            if (head.size() > 200) head.resize(200);
            if (head.find("ZipCode") != std::string::npos && head.find("PlaceName") != std::string::npos) {
                // header row - skip
                continue;
            }   
            std::cerr << "Error parsing record " << i << "! Raw record: '" << head << "'\n";
            continue;
         }
    }

    inputFile.close();
    outputFile.close();

    cout << "CSV file with lengths has been written successfully!" << endl;
}