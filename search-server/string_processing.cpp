#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
    std::vector<std::string_view> result;

    text.remove_prefix(std::min(text.find_first_not_of(" "s), text.size()));

   /* while (text.size() > 0) {

        int64_t space = text.find(' ');
        result.push_back(static_cast<size_t>(space) == text.npos ? text.substr(0, text.npos) : text.substr(0, space));
        if (static_cast<size_t>(space) == text.npos) {
            break;
        }
        //str.remove_prefix(space);
        text.remove_prefix(space);
        text.remove_prefix(std::min(text.find_first_not_of(" "s), text.size()));
    }*/
    int64_t pos = text.find_first_not_of(" ");
    // 2
    const int64_t pos_end = text.npos;
    // 3
    while (pos != pos_end) {
        // 4
        int64_t space = text.find(' ', pos);
        // 5
        result.push_back(space == pos_end ? text.substr(pos) : text.substr(pos, space - pos));
        // 6
        pos = text.find_first_not_of(" ", space);
    }

    return result;

}