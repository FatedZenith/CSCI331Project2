#include "LengthBuffer.h"
#include <iostream>
#include <cstdint>


// Authors: Team 5
// Date: 2025-10-11
/* Purpose: This file contains the implementation of the LengthBuffer class, which
   provides functions to write ZipCodeRecordBuffer objects to a binary file with 
   length-indicated fields and to read them back into memory. It supports writing
   each field with a 2-byte length prefix and reading fields safely into record objects.
*/

// Write a ZipCodeRecordBuffer to a length-indicated binary file
bool LengthBuffer::writeRecord(std::ofstream& out, const ZipCodeRecordBuffer& record) {
    if (!out) return false; // Check if the output stream is valid

    // Lambda to write a single field with a 2-byte length prefix
    auto writeField = [&](const std::string& field) {
        uint16_t len = static_cast<uint16_t>(field.size()); // Length of the field
        out.write(reinterpret_cast<char*>(&len), sizeof(len)); // Write length
        out.write(field.c_str(), len);                        // Write field data
    };

    // Write each field of the record
    writeField(record.getZipCode());
    writeField(record.getPlaceName());
    writeField(record.getState());
    writeField(record.getCounty());
    writeField(std::to_string(record.getLatitude()));
    writeField(std::to_string(record.getLongitude()));

    return true; 
}

// Read a ZipCodeRecordBuffer from a length-indicated binary file
bool LengthBuffer::readNextRecord(std::ifstream& in, ZipCodeRecordBuffer& record) {
    if (!in) return false; // Check if the input stream is valid

    // Lambda to read a single field with a 2-byte length prefix
    auto readField = [&](std::string& field) -> bool {
        uint16_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len)); // Read length
        if (in.eof() || in.fail()) return false;            // Check for errors

        char* buffer = new char[len];                      // Allocate buffer
        in.read(buffer, len);                               // Read field data
        if (in.eof() || in.fail()) {                        // Check for errors
            delete[] buffer;
            return false;
        }

        field = std::string(buffer, len); // Convert to string
        delete[] buffer;                  // Free buffer
        return true;
    };

    // Variables to hold each field
    std::string zip, place, state, county, latStr, lonStr;

    // Read fields one by one
    if (!readField(zip)) return false;
    if (!readField(place)) return false;
    if (!readField(state)) return false;
    if (!readField(county)) return false;
    if (!readField(latStr)) return false;
    if (!readField(lonStr)) return false;

    // Combine fields into a CSV string and unpack into the record
    record.unpackFromCSV(zip + "," + place + "," + state + "," + county + "," + latStr + "," + lonStr);
    return true; 
}
