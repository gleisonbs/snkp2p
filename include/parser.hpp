#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>

class Parser {
public:
    Parser() {};
    std::vector<std::string> parse(const std::string&, const char);
};


#endif
