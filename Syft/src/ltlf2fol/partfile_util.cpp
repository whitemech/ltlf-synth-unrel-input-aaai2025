#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;

std::set<std::string> get_unobservables(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return {};
    }

    std::set<std::string> unobservables;
    std::string line;

    while (getline(file, line)) {
        // Check the line prefix and process accordingly
        if (line.find(".unobservables:") == 0) {
            vector<string> tokens;
            // Split and skip the first token which is "unobservables"
            split(tokens, line, is_any_of(" "), token_compress_on);
            // Insert the tokens into the set skipping the first one
            unobservables.insert(tokens.begin() + 1, tokens.end());
        }
    }

    file.close();
    return unobservables;
}