#include <map>
#include <vector>       // ✅ Added for std::vector
#include <iomanip>
#include <string>
#include "ZipCodeRecordBuffer.h"
#include <iostream>
#include <limits>       // for numeric_limits
#include "LengthBuffer.h"
#include <fstream>
#include <sstream>

using namespace std;

// Struct to hold extreme ZIP codes for a state
struct StateRecord {
    string easternmost_zip;
    double easternmost_lon = -numeric_limits<double>::max();
    string westernmost_zip;
    double westernmost_lon = numeric_limits<double>::max();
    string northernmost_zip;
    double northernmost_lat = -numeric_limits<double>::max();
    string southernmost_zip;
    double southernmost_lat = numeric_limits<double>::max();
};

int main() {
    map<string, StateRecord> all_states;
    vector<ZipCodeRecordBuffer> all_records;

    const string files[] = {"../Data/us_postal_codes.csv", "../Data/us_postal_rand.csv"};

    for (const auto& filename : files) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            continue;
        }

        string header;
        getline(file, header); // skip header

        ZipCodeRecordBuffer buffer;
        int count = 0;
        while (buffer.ReadRecord(file)) {
            string state = buffer.getState();
            string zip = buffer.getZipCode();
            double latitude = buffer.getLatitude();
            double longitude = buffer.getLongitude();

            all_records.push_back(buffer);
            count++;

            // Initialize state if it doesn't exist
            if (all_states.find(state) == all_states.end()) {
                all_states[state] = StateRecord{};
            }

            StateRecord& record = all_states[state];

            // Update extremes
            if (longitude > record.easternmost_lon) {
                record.easternmost_lon = longitude;
                record.easternmost_zip = zip;
            }
            if (longitude < record.westernmost_lon) {
                record.westernmost_lon = longitude;
                record.westernmost_zip = zip;
            }
            if (latitude > record.northernmost_lat) {
                record.northernmost_lat = latitude;
                record.northernmost_zip = zip;
            }
            if (latitude < record.southernmost_lat) {
                record.southernmost_lat = latitude;
                record.southernmost_zip = zip;
            }
        }
        file.close();
        cout << "Read " << count << " records from " << filename << endl;
    }

    // Print header
    cout << left << setw(8) << "State"
         << setw(15) << "Easternmost"
         << setw(15) << "Westernmost"
         << setw(15) << "Northernmost"
         << setw(15) << "Southernmost"
         << "\n";
    cout << string(68, '-') << "\n";

    // Print extremes per state
    for (const auto& [state, record] : all_states) {
        cout << left << setw(8) << state
             << setw(15) << record.easternmost_zip
             << setw(15) << record.westernmost_zip
             << setw(15) << record.northernmost_zip
             << setw(15) << record.southernmost_zip
             << "\n";
    }

    // Write all records to a length-indicated binary file
    ofstream out("all_records.len", ios::binary); // Open the file for binary output
    if (!out.is_open()) {                         // Check if the file opened successfully
        cerr << "Error opening all_records.len for writing.\n";
        return 1;
    }

    int written = 0; // Counter for successfully written records

    // Loop through all records in memory
    for (const auto& rec : all_records) {
        // Attempt to write each record to the file
        // If successful, increment the counter
        if (LengthBuffer::writeRecord(out, rec)) written++;
    }

    out.close(); // Close the output file
    cout << "\n" << written << " records written to all_records.len\n"; // Report how many records were written


    return 0;
}
