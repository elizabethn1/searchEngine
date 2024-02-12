
#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <cctype>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Cleans a specific token by:
 * - Removing leading and trailing punctuation
 * - Converting to lowercase
 * If the token has no letters, returns the empty string.
 */
string cleanToken(string s) {
    string tempToken = "";
    string cleanedToken = "";

    int indexA = -1;
    int indexB = -1;

    bool containsOneAlpha = false;

    // Find index of the first non-punct char
    for (int i = 0; i < s.size(); ++i) {
        if (!ispunct(s.at(i))) {
            indexA = i;
            break;
        }
    }

    // Find index of the last non-punct char
    for (int j = s.size()-1; j >= 0; --j) {
        if (!ispunct(s.at(j))) {
            indexB = j;
            break;
        }
    }

    // The string is either empty or only contains punct chars
    if ((indexA == -1) && (indexB == -1)) {
        return cleanedToken;
    }

    tempToken = s.substr(indexA, indexB - indexA + 1);

    // Make uppercase alpha chars become lowercase
    for (int k = 0; k < tempToken.size(); ++k) {
        if (isalpha(tempToken.at(k))) {
            cleanedToken += tolower(tempToken.at(k));
        }
        else {
            cleanedToken += tempToken.at(k);
        }
    }

    // Checks for a string that contains at least one alpha char
    for (int l = 0; l < cleanedToken.size(); ++l) {
        if (isalpha(cleanedToken.at(l))) {
            containsOneAlpha = true;
            break;
        }
    }

    if (containsOneAlpha == false) {
        cleanedToken = "";
    }

    return cleanedToken;
}

/**
 * Returns a set of the cleaned tokens in the given text.
 */
set<string> gatherTokens(string text) {
    set <string> uniqueTokens;
    
    string tempWord = "";

    for (int i = 0; i < text.size(); ++i) {
        if (text.at(i) != ' ') {
            tempWord += text.at(i);
        }

        if ((text.at(i) == ' ') && (tempWord.size() != 0)) {
            if (cleanToken(tempWord) != "") {
                uniqueTokens.insert(cleanToken(tempWord));
            }
            tempWord = "";
        }
    }

    // The cleaned word is not empty, so it can be added to the uniqueTokens set
    if (cleanToken(tempWord) != "") {
        uniqueTokens.insert(cleanToken(tempWord));
    }

    return uniqueTokens;
}

/**
 * Builds the inverted index from the given data file. The existing contents
 * of `index` are left in place.
 */
int buildIndex(string filename, map<string, set<string>>& index) {
    int numURLS = 0; // Keeps track of the number of URLs that are present in the file

    string website = "";
    string text = "";

    ifstream inFS;

    inFS.open(filename);

    if (!inFS.is_open()) {
        return numURLS;
    }

    while (!inFS.eof()) {
        getline(inFS, website);

        // Indicates that the end of the file has been reached
        if (website == "") {
            break;
        }

        getline(inFS, text);

        numURLS += 1;

        // These are the keys of the map (index)
        set <string> tokensSet = gatherTokens(text);

        // Assign each key to a value, which is a set of URLs
        for (string a : tokensSet) {
            if (index.count(a) == 1) {
                set <string> temp = index.at(a);

                temp.insert(website);
                index.erase(a);
                index.emplace(a, temp);
            }
            else {
                set <string> webPages;
                webPages.insert(website);
                index.emplace(a, webPages);
            }
        }
    }

    inFS.close();

    return numURLS;
}

/**
 * Takes a string and separates the words of the string and stores them in a vector.
 */
void breakText(string line, vector <string> &list) {
    string tempWord = " ";
    int index = 0;
    int spacePosition = 0;

    // Loop through the indices of the line to find individual words.
    while (index < line.size()) {
        if (line.find(' ', index) == string::npos) { // Indicates that the last word has been reached.
            tempWord = line.substr(index);
            index = line.size();
        }
        else {
            spacePosition = line.find(' ', index);
            tempWord = line.substr(index, spacePosition - index);
            index = spacePosition + 1;

            while (line.at(index) == ' ') {
                index += 1;
            }
        }

        list.push_back(tempWord); // Add the identified word to the vector.
    }
}

/**
 * Runs a search query on the provided index, and returns a set of results.
 *
 * Search terms are processed using set operations (union, intersection, difference).
 */
set<string> findQueryMatches(map<string, set<string>>& index, string sentence) {
    set <string> results; // Empty 

    vector <string> words;
    vector <string> cleanedWords;

    // Inititates a vector to only contain the words of the sentence (input)
    if (sentence.find(" ") != string::npos) {
        breakText(sentence, words);
    }
    else {
        words.push_back(sentence);
    }

    // Initiates a vector that contains cleaned words of sentence
    for (int i = 0; i < words.size(); ++i) {
        cleanedWords.push_back(cleanToken(words.at(i)));
    }

    for (int j = 0; j < cleanedWords.size(); ++j) {
        set <string> setA = results; 
        set <string> setB; 

        string tempStr = cleanedWords.at(j);

        // Assigns setB with a value (a set) that's based on whether the key is in the map (index) or not
        if (index.count(cleanedWords.at(j)) == 1) {
            setB = index.at(tempStr);
        }
        else {
            setB = {};
        }

        // The set contains one element
        if ((cleanedWords.size() == 1) && (index.count(tempStr) == 1)) { // The element is in the index
            results = index.at(tempStr);
            return results;
        }
        else if ((cleanedWords.size() == 1) && (index.count(tempStr) == 0)) { // The element is not in the index
            results = {};
            return results;
        }

        // Set operations for sets that contains more than one element
        if ((words.at(j).at(0) != '+') && (words.at(j).at(0) != '-')) { // Union
            set_union(setA.begin(), setA.end(), setB.begin(), setB.end(), inserter(results, results.begin()));
        }
        else if (words.at(j).at(0) == '+') { // Intersection
            results = {};
            set_intersection(setA.begin(), setA.end(), setB.begin(), setB.end(), inserter(results, results.begin()));
        }
        else if (words.at(j).at(0) == '-') { // Set difference
            results = {};
            set_difference(setA.begin(), setA.end(), setB.begin(), setB.end(), inserter(results, results.begin()));
        }
    }
    return results;
}

/**
 * Runs the main command loop for the search program
 */
void searchEngine(string filename) {
    ifstream inFS;

    inFS.open(filename);

    if (!inFS.is_open()) {
        cout << "Invalid filename." << endl;
    }

    inFS.close();

    cout << "Stand by while building index..." << endl;

    map<string, set<string>> myIndex;

    int numWebPages = buildIndex(filename, myIndex);
    int numTerms = 0;

    // Count the number of terms in the index
    for (auto[key, value] : myIndex) {
        numTerms += 1;
    }

    if (numWebPages == 0) {
        cout << "Indexed 0 pages containing 0 unique terms" << endl;
    }
    else {
        cout << "Indexed " << numWebPages << " pages containing " << numTerms << " unique terms" << endl;
    }

    cout << endl;

    string userQuery = "";

    do {
        int numMatchingPages = 0;

        cout << "Enter query sentence (press enter to quit): ";
        getline(cin, userQuery);

        // The user wants to quit
        if (userQuery == "") {
            break;
        }

        set <string> myResults = findQueryMatches(myIndex, userQuery); 

        // Count the number of matching pages that were found 
        for (string a : myResults) {
            numMatchingPages += 1;
        }

        cout << "Found " << numMatchingPages << " matching pages" << endl;

        // Display URLs
        for (string b : myResults) {
            cout << b << endl;
        }

        cout << endl;
    } while (userQuery != "");

    cout << "Thank you for searching!" << endl;
}
