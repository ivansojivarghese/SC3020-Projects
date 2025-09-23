#ifndef RECORD_H
#define RECORD_H

#include <vector>
#include <cstring>
#include <string>

const int RECORD_SIZE = 40; // Size of a record in bytes

class Record {
public:
    char game_date_est[11]; // 11 bytes
    int team_id_home;       // 4 bytes
    int pts_home;           // 4 bytes
    float fg_pct_home;      // 4 bytes
    float ft_pct_home;      // 4 bytes
    float fg3_pct_home;     // 4 bytes
    int ast_home;           // 4 bytes
    int reb_home;           // 4 bytes
    bool home_team_wins;    // 1 byte

    // Serialize the record into a byte stream
    std::vector<char> serialize() const;

    // Deserialize a byte stream into a record
    static Record deserialize(const std::vector<char>& buffer);

    // Read game records from a file
    static std::vector<Record> readGamesFile(const std::string& filename);
};

// Represents a collection of NBA game records
struct NBARecords {
    std::vector<Record*> records; // Pointers to Record objects
};

#endif // RECORD_H