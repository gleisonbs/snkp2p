#include <sstream>

#include <parser.hpp>

using std::getline;
using std::string;
using std::stringstream;
using std::vector;

vector<string> Parser::parse(const string& txt, const char delim) {
    vector<string> tokens;
    stringstream ss(txt);
    string token;
    while (getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}
