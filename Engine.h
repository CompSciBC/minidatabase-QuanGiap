#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>   
#include <vector>     
#include "BST.h"      
#include "Record.h"
//add header files as needed
#include <algorithm>

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine {
    vector<Record> heap;                  // the main data store (simulates a heap file)
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name (can have duplicates)

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn) {
        //TODO
        // check if record exist base on id
        int* indexRecord = idIndex.find(recIn.id);
        if(indexRecord != nullptr){
            return -1; // record exist
        }
        heap.push_back(recIn);
        int rid = heap.size() - 1; // get the record id
        idIndex.insert(recIn.id, rid); // insert into id index
        string lastName = toLower(recIn.last);
        vector<int>* ridList = lastIndex.find(lastName);
        if (ridList == nullptr) {
            // no entry for this last name yet
            vector<int> newList = {rid};
            lastIndex.insert(lastName, newList);
        } else {
            // append to existing list
            ridList->push_back(rid);
        }
        return rid;
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        //TODO
        int* ridPtr = idIndex.find(id);
        if (ridPtr == nullptr) {
            return false; // record not found
        }
        int rid = *ridPtr;
        Record &rec = heap[rid];
        rec.deleted = true;
        idIndex.erase(id); // remove from id index
        string lastName = toLower(rec.last);
        vector<int>* ridList = lastIndex.find(lastName);
        if (ridList != nullptr) {
            // remove rid from the list
            ridList->erase(remove(ridList->begin(), ridList->end(), rid), ridList->end());
            if (ridList->empty()) {
                lastIndex.erase(lastName); // remove last name entry if list is empty
            }
        }
        return true;
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut) {
        //reset comparison count
        idIndex.resetMetrics();
        int* ridPtr = idIndex.find(id);
        cmpOut = idIndex.comparisons;
        if(ridPtr == nullptr){
            return nullptr; // not found
        }
        int rid = *ridPtr;
        return &heap[rid];
    }

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut) {
        //TODO
        // reset comparison count
        idIndex.resetMetrics();
        vector<const Record*> results;
        // create lambda function to collect results
        auto rangeFn = [&](int key, int rid) {
            results.push_back(&heap[rid]);
        };
        idIndex.rangeApply(lo, hi, rangeFn);
        cmpOut = idIndex.comparisons;
        return results;
    }

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut) {
        // reset comparison count
        lastIndex.resetMetrics();
        string lowerPrefix = toLower(prefix);
        vector<const Record*> results;
        vector<int>* ridList = lastIndex.find(lowerPrefix);
        if (ridList != nullptr) {
            for(int i=0; i < ridList->size(); i++){
                int index = (*ridList)[i];
                results.push_back(&heap[index]);
            }
        }
        cmpOut = lastIndex.comparisons;
        return results;
    }
};

#endif
