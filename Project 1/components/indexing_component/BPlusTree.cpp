#include "BPlusTree.h"
#include "../../components/storage_component/Block.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>

// Constructor
BPlusTree::BPlusTree(const std::string& file) : filename(file), rootNode(nullptr) {}

// Get the root node of the B+ Tree
BPlusTreeNode* BPlusTree::getRootNode() {
    return this->rootNode;
}

// Search for a record by key
std::tuple<NBARecords*, int> BPlusTree::searchRecord(float key) {
    if (rootNode == nullptr) {
        return std::make_tuple(nullptr, 0);
    }

    BPlusTreeNode* currentNode = rootNode;
    int nodesAccessed = 0;

    // Traverse to the leaf node
    while (!currentNode->isLeafNode) {
        nodesAccessed++;
        for (int i = 0; i < currentNode->nodeKeys.size(); i++) {
            if (key < currentNode->nodeKeys[i]) {
                currentNode = currentNode->childPointers[i];
                break;
            }
            if (i == currentNode->nodeKeys.size() - 1) {
                currentNode = currentNode->childPointers[i + 1];
                break;
            }
        }
    }

    // Search for the key in the leaf node
    for (int i = 0; i < currentNode->nodeKeys.size(); i++) {
        if (key == currentNode->nodeKeys[i]) {
            return std::make_tuple(currentNode->recordPointers[i], nodesAccessed + 1);
        }
    }

    return std::make_tuple(nullptr, nodesAccessed + 1);
}

// Search for records within a range of keys
std::tuple<NBARecords*, int, int> BPlusTree::searchRangedRecord(float startKey, float endKey) {
    if (rootNode == nullptr) {
        return std::make_tuple(nullptr, 0, 0);
    }

    BPlusTreeNode* currentNode = rootNode;
    int nodesAccessed = 0;
    int blocksAccessed = 0;

    // Traverse to the leaf node containing the start key
    while (!currentNode->isLeafNode) {
        nodesAccessed++;
        auto it = std::lower_bound(currentNode->nodeKeys.begin(), currentNode->nodeKeys.end(), startKey);
        int index = it - currentNode->nodeKeys.begin();
        currentNode = currentNode->childPointers[index];
    }

    // Collect records within the range
    auto* resultRecords = new NBARecords();
    while (currentNode != nullptr) {
        nodesAccessed++;
        blocksAccessed++; // Each leaf node corresponds to a block
        for (int i = 0; i < currentNode->nodeKeys.size(); i++) {
            if (currentNode->nodeKeys[i] > endKey) {
                // Stop traversing once we exceed the upper bound
                currentNode = nullptr;
                break;
            }
            if (currentNode->nodeKeys[i] >= startKey && currentNode->nodeKeys[i] <= endKey) {
                // Add all records for this key
                for (Record* record : currentNode->recordPointers[i]->records) {
                    resultRecords->records.push_back(record);
                }
            }
        }
        if (currentNode != nullptr) {
            currentNode = currentNode->getNextLeafNode();
        }
    }

    return std::make_tuple(resultRecords, nodesAccessed, blocksAccessed);
}

// Bulk load the B+ Tree
void BPlusTree::bulkLoad(const std::vector<Record*>& sortedRecords) {
    if (sortedRecords.empty()) return;

    // Create the initial leaf node
    BPlusTreeNode* leafNode = new BPlusTreeNode(true);
    rootNode = leafNode;

    for (Record* record : sortedRecords) {
        if (leafNode->nodeKeys.size() >= leafNode->maxCapacity) {
            // Split the leaf node
            BPlusTreeNode* newLeafNode = new BPlusTreeNode(true);
            int splitIdx = leafNode->nodeKeys.size() / 2;

            // Move half the keys and records to the new leaf node
            newLeafNode->nodeKeys.assign(leafNode->nodeKeys.begin() + splitIdx, leafNode->nodeKeys.end());
            newLeafNode->recordPointers.assign(leafNode->recordPointers.begin() + splitIdx, leafNode->recordPointers.end());

            // Remove the moved keys and records from the current leaf node
            leafNode->nodeKeys.erase(leafNode->nodeKeys.begin() + splitIdx, leafNode->nodeKeys.end());
            leafNode->recordPointers.erase(leafNode->recordPointers.begin() + splitIdx, leafNode->recordPointers.end());

            // Update next leaf pointers
            newLeafNode->nextLeafNode = leafNode->nextLeafNode;
            leafNode->nextLeafNode = newLeafNode;

            // Promote the middle key to the parent
            float promoteKey = newLeafNode->nodeKeys[0];
            BPlusTreeNode* parent = leafNode->parent;

            if (parent == nullptr) {
                // Create a new root node
                parent = new BPlusTreeNode(false);
                rootNode = parent;
                parent->childPointers.push_back(leafNode);
            }

            // Insert the promoteKey into the parent
            auto it = std::lower_bound(parent->nodeKeys.begin(), parent->nodeKeys.end(), promoteKey);
            int index = it - parent->nodeKeys.begin();
            parent->nodeKeys.insert(it, promoteKey);
            parent->childPointers.insert(parent->childPointers.begin() + index + 1, newLeafNode);

            // Update parent pointers
            newLeafNode->parent = parent;

            // Check if the parent needs to split
            if (parent->nodeKeys.size() > parent->maxCapacity) {
                splitInternalNode(parent);
            }

            leafNode = newLeafNode;
        }

        // Insert the record into the current leaf node
        leafNode->nodeKeys.push_back(record->fg_pct_home);
        auto* recordVector = new NBARecords();
        recordVector->records.push_back(record);
        leafNode->recordPointers.push_back(recordVector);
    }
}

// Split internal nodes during bulk loading
void BPlusTree::splitInternalNode(BPlusTreeNode* node) {
    int splitIdx = node->nodeKeys.size() / 2;
    float promoteKey = node->nodeKeys[splitIdx];

    // Create a new internal node
    BPlusTreeNode* newNode = new BPlusTreeNode(false);
    newNode->nodeKeys.assign(node->nodeKeys.begin() + splitIdx + 1, node->nodeKeys.end());
    newNode->childPointers.assign(node->childPointers.begin() + splitIdx + 1, node->childPointers.end());

    // Remove the moved keys and pointers from the current node
    node->nodeKeys.erase(node->nodeKeys.begin() + splitIdx, node->nodeKeys.end());
    node->childPointers.erase(node->childPointers.begin() + splitIdx + 1, node->childPointers.end());

    // Promote the middle key to the parent
    BPlusTreeNode* parent = node->parent;
    if (parent == nullptr) {
        // Create a new root node
        parent = new BPlusTreeNode(false);
        rootNode = parent;
        parent->childPointers.push_back(node);
    }

    // Insert the promoteKey into the parent
    auto it = std::lower_bound(parent->nodeKeys.begin(), parent->nodeKeys.end(), promoteKey);
    int index = it - parent->nodeKeys.begin();
    parent->nodeKeys.insert(it, promoteKey);
    parent->childPointers.insert(parent->childPointers.begin() + index + 1, newNode);

    // Update parent pointers
    newNode->parent = parent;

    // Check if the parent needs to split
    if (parent->nodeKeys.size() > parent->maxCapacity) {
        splitInternalNode(parent);
    }
}

// Save a node to disk
void BPlusTree::saveNode(BPlusTreeNode* node, std::ofstream& outFile) {
    std::vector<char> buffer = node->serialize();
    outFile.write(buffer.data(), buffer.size());
}

// Load a node from disk
BPlusTreeNode* BPlusTree::loadNode(std::ifstream& inFile) {
    std::vector<char> buffer(BLOCK_SIZE);
    inFile.read(buffer.data(), buffer.size());
    return BPlusTreeNode::deserialize(buffer, /* isLeafNode */ true);
}

// Save the entire B+ Tree to disk
void BPlusTree::saveToDisk() {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }
    saveNode(rootNode, outFile);
    outFile.close();
}

// Load the entire B+ Tree from disk
void BPlusTree::loadFromDisk() {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }
    rootNode = loadNode(inFile);
    inFile.close();
}

// Count the number of nodes in the tree
int BPlusTree::countNodes(BPlusTreeNode* node) const {
    if (node == nullptr) return 0;
    int count = 1;
    if (!node->isLeafNode) {
        for (BPlusTreeNode* child : node->childPointers) {
            count += countNodes(child);
        }
    }
    return count;
}

// Count the number of levels in the tree
int BPlusTree::countLevels(BPlusTreeNode* node) const {
    if (node == nullptr) return 0;
    int levels = 1;
    if (!node->isLeafNode) {
        levels += countLevels(node->childPointers[0]);
    }
    return levels;
}

// Get the number of nodes in the B+ Tree
int BPlusTree::getNumberOfNodes() const {
    return countNodes(rootNode);
}

// Get the number of levels in the B+ Tree
int BPlusTree::getNumberOfLevels() const {
    return countLevels(rootNode);
}

// Destructor
BPlusTree::~BPlusTree() {
    deleteTree(rootNode);
}

// Recursively delete the tree
void BPlusTree::deleteTree(BPlusTreeNode* node) {
    if (node == nullptr) return;

    if (!node->isLeafNode) {
        for (BPlusTreeNode* child : node->childPointers) {
            deleteTree(child);
        }
    }

    // Delete record pointers in leaf nodes
    if (node->isLeafNode) {
        for (NBARecords* records : node->recordPointers) {
            delete records;
        }
    }

    delete node;
}