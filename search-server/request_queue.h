#pragma once

#include <deque>
#include <vector>
#include <string>

#include "search_server.h"

class RequestQueue {
public:
    RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    
    int GetNoResultRequests() const;
    
private:
    
    struct QueryResult {
        // определите, что должно быть в структуре
        std::string query;
        int docs;
    };
    
    std::deque<QueryResult> requests_;
    const int min_in_day_ = 1440;
    const SearchServer& server;
    // возможно, здесь вам понадобится что-то ещё
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    QueryResult result = { raw_query, static_cast<int>(server.FindTopDocuments(raw_query, document_predicate).size()) };
    requests_.pop_front();
    requests_.push_back(result);

    return server.FindTopDocuments(raw_query);
}