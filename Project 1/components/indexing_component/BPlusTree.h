#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "BPlusTreeNode.h"
#include <tuple>
#include <queue>
#include <set>
#include <string>

class BPlusTree {
private:
    BPlusTreeNode* rootNode;
    std::string filename;

    // Helper methods
    BPlusTreeNode* insertAndSplit(float key, Record* recordData, BPlusTreeNode* currentNode);
    bool removeRecordHelper(BPlusTreeNode* currentNode, float keyToRemove);
    void balanceWithLeftSibling(BPlusTreeNode* parentNode, int leftIndex, int rightIndex);
    void balanceWithRightSibling(BPlusTreeNode* parentNode, int leftIndex, int rightIndex);
    void mergeNodes(BPlusTreeNode* parentNode, int currentIndex, int leftIndex, int rightIndex);

    // Helper method to split internal nodes during bulk loading
    void splitInternalNode(BPlusTreeNode* node);

    // Helper method to recursively delete the tree
    void deleteTree(BPlusTreeNode* node);

    // Disk storage methods
    void saveNode(BPlusTreeNode* node, std::ofstream& outFile);
    BPlusTreeNode* loadNode(std::ifstream& inFile);

    // Statistics methods
    int countNodes(BPlusTreeNode* node) const;
    int countLevels(BPlusTreeNode* node) const;

public:
    // Constructor
    BPlusTree(const std::string& file);

    // Bulk load the B+ Tree
    void bulkLoad(const std::vector<Record*>& sortedRecords);

    // Insert a record into the B+ Tree
    void insertRecord(float key, Record* recordData);

    // Delete a record from the B+ Tree
    void deleteRecord(float keyToRemove);

    // Search for a record by key
    std::tuple<NBARecords*, int> searchRecord(float key);

    // Search for records within a range of keys
    std::tuple<NBARecords*, int, int> searchRangedRecord(float startKey, float endKey);

    // Search for unique keys within a range
    std::set<float> searchRangedKeys(float startKey, float endKey);

    // Display the B+ Tree structure
    void displayTree(BPlusTreeNode* currentNode);

    // Display the keys in the root node
    void displayRootNode();

    // Get the size parameter (n) of the B+ Tree
    void getNodeSize();

    // Calculate and display tree statistics
    void calculateStatistics(BPlusTreeNode* currentNode, int insertFlag);

    // Get the root node of the B+ Tree
    BPlusTreeNode* getRootNode();

    // Save the B+ Tree to disk
    void saveToDisk();

    // Load the B+ Tree from disk
    void loadFromDisk();

    // Get the number of nodes in the B+ Tree
    int getNumberOfNodes() const;

    // Get the number of levels in the B+ Tree
    int getNumberOfLevels() const;

    // Destructor
    ~BPlusTree();
};

#endif // BPLUSTREE_H