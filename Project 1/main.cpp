#include "components/storage_component/Record.h"
#include "components/storage_component/Block.h"
#include "components/storage_component/Disk.h"
#include "components/indexing_component/BPlusTree.h"
#include <iostream>
#include <chrono>
#include <algorithm>

// Helper function to calculate the average FG_PCT_home of returned records
float calculateAverageFG_PCT_home(const NBARecords* records) {
    if (records->records.empty()) return 0.0f; // Avoid division by zero
    float sum = 0.0f;
    for (Record* record : records->records) {
        sum += record->fg_pct_home;
    }
    return sum / records->records.size();
}

int main() {
    // ==================================================
    // Task 1: Storage Component
    // ==================================================
    std::cout << "=== Task 1: Storage Component ===" << std::endl;

    // Step 1: Load NBA game data from file
    std::vector<Record> records = Record::readGamesFile("games.txt");
    std::cout << "Number of records: " << records.size() << std::endl;

    // Step 2: Store records in blocks on disk
    Disk disk("nba_data.bin");
    int blockId = 0;
    Block block(blockId);
    for (auto& record : records) {
        if (!block.addRecord(record)) {
            disk.writeBlock(block, blockId);
            blockId++;
            block = Block(blockId);
            block.addRecord(record);
        }
    }
    disk.writeBlock(block, blockId); // Write the last block
    std::cout << "Number of blocks: " << blockId + 1 << std::endl;

    // Report additional statistics for Task 1
    const int RECORD_SIZE = 40; // Size of a record in bytes
    const int BLOCK_SIZE = 4096; // Block size in bytes
    int recordsPerBlock = BLOCK_SIZE / RECORD_SIZE;
    std::cout << "Size of a record: " << RECORD_SIZE << " bytes" << std::endl;
    std::cout << "Number of records stored in a block: " << recordsPerBlock << std::endl;

    // ==================================================
    // Task 2: Indexing Component
    // ==================================================
    std::cout << "\n=== Task 2: Indexing Component ===" << std::endl;

    // Step 3: Build B+ Tree on "FG_PCT_home" using bulk loading
    BPlusTree bPlusTree("bplus_tree.bin");

    // Sort records by fg_pct_home
    std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
        return a.fg_pct_home < b.fg_pct_home;
    });

    // Convert records to pointers
    std::vector<Record*> recordPtrs;
    for (auto& record : records) {
        recordPtrs.push_back(&record);
    }

    // Bulk load the B+ Tree
    bPlusTree.bulkLoad(recordPtrs);

    // Save the B+ Tree to disk
    bPlusTree.saveToDisk();

    // Report statistics for Task 2
    BPlusTreeNode* root = bPlusTree.getRootNode();
    std::cout << "Parameter n of the B+ tree: " << root->maxCapacity << std::endl;
    std::cout << "Number of nodes in the B+ tree: " << bPlusTree.getNumberOfNodes() << std::endl;
    std::cout << "Number of levels in the B+ tree: " << bPlusTree.getNumberOfLevels() << std::endl;
    std::cout << "Content of the root node: ";
    for (float key : root->getNodeKeys()) {
        std::cout << key << " ";
    }
    std::cout << std::endl;

    // ==================================================
    // Task 3: Search and Comparison
    // ==================================================
    std::cout << "\n=== Task 3: Search and Comparison ===" << std::endl;

    // Step 4: Perform range query on "FG_PCT_home" (0.6 to 0.9) using B+ Tree
    int iterations = 100; // Number of iterations for both B+ Tree and linear scan
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        auto result = bPlusTree.searchRangedRecord(0.6f, 0.9f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Perform one additional search to get the result
    auto result = bPlusTree.searchRangedRecord(0.6f, 0.9f);
    NBARecords* recordsInRange = std::get<0>(result);
    int nodesAccessed = std::get<1>(result);
    int blocksAccessed = std::get<2>(result);

    std::cout << "Number of index nodes accessed: " << nodesAccessed << std::endl;
    std::cout << "Number of data blocks accessed: " << blocksAccessed << std::endl;
    std::cout << "Number of records in range of 0.6 - 0.9: " << recordsInRange->records.size() << std::endl;
    std::cout << "Average FG_PCT_home of returned records: " << calculateAverageFG_PCT_home(recordsInRange) << std::endl;
    std::cout << "Time taken for B+ Tree search (average over " << iterations << " iterations): " << (elapsed.count() / iterations) << " seconds" << std::endl;

    // Step 5: Brute-force linear scan for comparison
    start = std::chrono::high_resolution_clock::now();
    int totalRecordsInRange = 0; // Counter for records in range across all iterations
    for (int i = 0; i < iterations; i++) {
        int currentIterationCount = 0; // Counter for records in range for the current iteration
        for (int j = 0; j <= blockId; j++) {
            Block currentBlock = disk.readBlock(j);
            for (const auto& record : currentBlock.getRecords()) {
                if (record.fg_pct_home >= 0.6f && record.fg_pct_home <= 0.9f) {
                    currentIterationCount++; // Increment counter for the current iteration
                }
            }
        }
        totalRecordsInRange += currentIterationCount; // Accumulate the count across all iterations
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;

    std::cout << "Number of blocks accessed in linear scan: " << blockId + 1 << std::endl;
    std::cout << "Number of records in range of 0.6 - 0.9 (linear scan): " << totalRecordsInRange / iterations << std::endl;
    std::cout << "Time taken for linear scan (average over " << iterations << " iterations): " << (elapsed.count() / iterations) << " seconds" << std::endl;

    return 0;
}