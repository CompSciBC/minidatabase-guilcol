#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <vector>
#include "BST.h"
#include "Record.h"
#include <algorithm>

// add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s)
{
	for (char &c : s)
		c = (char)tolower((unsigned char)c);
	return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine
{
	vector<Record> heap;				// the main data store (simulates a heap file)
	BST<int, int> idIndex;				// index by student ID
	BST<string, vector<int>> lastIndex; // index by last name (can have duplicates)

	// Inserts a new record and updates both indexes.
	// Returns the record ID (RID) in the heap.
	int insertRecord(const Record &recIn)
	{
		// Check if a record with the same ID already exists
		int *existingIndex = idIndex.find(recIn.id);
		int rid = heap.size(); // might as well register this here because we need it anyway

		// In case NO record exists...
		if (!existingIndex)
		{
			heap.push_back(recIn);						// Add to heap
			idIndex.insert(recIn.id, rid);				// Add RID to idIndex
			string lastNameLower = toLower(recIn.last); // Convert last name to lowercase for indexing
			// A node for this last name might already exist
			// Because two separate records would be stored in the same last name
			// So check that first
			vector<int> *lastNameVector = lastIndex.find(lastNameLower);
			if (!lastNameVector)
			{
				// It doesn't exist!
				// Create a new vector with this RID and insert, so simple
				vector<int> newVector = {rid};
				lastIndex.insert(lastNameLower, newVector);
			}
			else
			{
				// iT does exist!
				// Just append this RID to the existing vector
				lastNameVector->push_back(rid);
			}
			// Return the new heap index (RID)
			return rid;
		}
		else
		{
			// In case a record with the same ID already exists...
			// We need to "delete" the old record and update both indexes
			int oldIndex = *existingIndex;
			heap[oldIndex].deleted = true;

			// Old last name may differ from new last name, so update lastIndex accordingly
			string oldLastNameLower = toLower(heap[oldIndex].last);
			vector<int> *oldLastNameVector = lastIndex.find(oldLastNameLower);
			if (oldLastNameVector)
			{
				// Remove the old index from the vector in lastIndex
				oldLastNameVector->erase(
					remove(oldLastNameVector->begin(),
						   oldLastNameVector->end(),
						   oldIndex),
					oldLastNameVector->end());
			}

			// Add new record to the heap
			heap.push_back(recIn);
			int newRid = heap.size() - 1;

			// Erase old id from idIndex and add new one
			// idIndex shouldn't care about deleted records
			idIndex.erase(recIn.id);
			idIndex.insert(recIn.id, newRid);

			// Insert new last name into lastIndex
			string newLastNameLower = toLower(recIn.last);
			vector<int> *newLastNameVector = lastIndex.find(newLastNameLower);
			if (!newLastNameVector)
			{
				lastIndex.insert(newLastNameLower, vector<int>{newRid});
			}
			else
			{
				newLastNameVector->push_back(newRid);
			}

			return rid;
		}
	}

	// Deletes a record logically (marks as deleted and updates indexes)
	// Returns true if deletion succeeded.
	bool deleteById(int id)
	{
		// Find the record in idIndex
		int *heapIndex = idIndex.find(id);

		if (!heapIndex) // Handle doesn't exist
			return false;

		// Exists, "delete" it softly
		heap[*heapIndex].deleted = true;
		idIndex.erase(id); // Remove from idIndex
		string lastNameLower = toLower(heap[*heapIndex].last);
		// Find key for lastIndex
		vector<int> *lastNameVector = lastIndex.find(lastNameLower);

		if (lastNameVector)
		{
			lastNameVector->erase(
				remove(lastNameVector->begin(), lastNameVector->end(), *heapIndex),
				lastNameVector->end());
		}

		return true;
	}

	// Finds a record by student ID.
	// Returns a pointer to the record, or nullptr if not found.
	// Outputs the number of comparisons made in the search.
	const Record *findById(int id, int &cmpOut)
	{
		// Clear comparison counter
		idIndex.resetMetrics();
		int *heapIndex = idIndex.find(id);

		// Set cpmOut to number of comparisons made
		cmpOut = idIndex.comparisons;

		// Return the proper pointer
		if (!heapIndex)
			return nullptr;
		return &heap[*heapIndex];
	}

	// Returns all records with ID in the range [lo, hi].
	// Also reports the number of key comparisons performed.
	vector<const Record *> rangeById(int lo, int hi, int &cmpOut)
	{
		idIndex.resetMetrics();
		vector<const Record *> rangeResults;

		// Build a LAMBDA to use as callback
		auto callback = [&](int /*key*/, int rid)
		{
			//  Only keep it if the record isn’t deleted
			if (!heap[rid].deleted)
			{
				rangeResults.push_back(&heap[rid]);
			}
		};

		// Run the rangeApply with the LAMBDA above
		idIndex.rangeApply(lo, hi, callback);

		cmpOut = idIndex.comparisons;
		return rangeResults;
	}

	// Returns all records whose last name begins with a given prefix.
	// Case-insensitive using lowercase comparison.
	vector<const Record *> prefixByLast(const string &prefix, int &cmpOut)
	{

		lastIndex.resetMetrics();
		vector<const Record *> rangeResults;

		// Define lower and upper bounds for the prefix search
		// '{' is the next ASCII value after z, it turns higherBound into the perfect upper limit
		string lowerBound = toLower(prefix);
		string higherBound = lowerBound + "{";

		lastIndex.resetMetrics();

		// Build a LAMBDA to use as callback like before
		auto callback = [&](const string &, vector<int> &ridList)
		{
			for (int rid : ridList)
			{
				if (!heap[rid].deleted)
				{
					rangeResults.push_back(&heap[rid]);
				}
			}
		};

		lastIndex.rangeApply(lowerBound, higherBound, callback);

		cmpOut = lastIndex.comparisons;
		return rangeResults;
	}
};

#endif
