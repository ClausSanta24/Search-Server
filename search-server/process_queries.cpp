#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> vec(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), vec.begin(),
        [&search_server](std::string a) {
            return search_server.FindTopDocuments(a);
        });
    return vec;
}

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    // std::vector<std::vector<Document>> documents = ProcessQueries(search_server, queries);

    std::list<Document> vec;
    for (const auto& a : ProcessQueries(search_server, queries)) {
        for (const auto& b : a) {
            vec.push_back(b);
        }
    }
    return vec;
}