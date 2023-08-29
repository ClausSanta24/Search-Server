#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
    : server(search_server)
{
    RequestQueue::QueryResult empty_req = { ""s, {} };
        
    for (int i = 0; i < min_in_day_; ++i) {
        requests_.push_back(empty_req);
    }
}
    
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {

    return RequestQueue::AddFindRequest(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    int result = 0;
    for (RequestQueue::QueryResult a : requests_) {
        if (a.docs == 0) {
            ++result;
            }
        }
    return result;
}