#pragma once

#include <iostream>
using namespace std::literals;

struct Document {
    Document() = default;
    
    Document(int id_, double relevance_, int rating_);

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};