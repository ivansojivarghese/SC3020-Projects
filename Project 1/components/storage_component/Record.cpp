#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "Record.h"

// Serialize the record into a byte stream
std::vector<char> Record::serialize() const {
    std::vector<char> buffer(RECORD_SIZE);
    std::memcpy(buffer.data(), this, RECORD_SIZE);
    return buffer;
}

// Deserialize a byte stream into a record
Record Record::deserialize(const std::vector<char>& buffer) {
    Record record;
    std::memcpy(&record, buffer.data(), RECORD_SIZE);
    return record;
}

// Read game records from a file
std::vector<Record> Record::readGamesFile(const std::string& filename) {
    std::vector<Record> records;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return records;
    }

    std::string line;
    std::getline(file, line); // Skip the header line
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Record record;

        // Parse the line into the Record object
        iss >> record.game_date_est >> record.team_id_home >> record.pts_home >>
            record.fg_pct_home >> record.ft_pct_home >> record.fg3_pct_home >>
            record.ast_home >> record.reb_home >> record.home_team_wins;

        records.push_back(record);
    }

    file.close();
    return records;
}