#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(std::string_view(stop_words_text))
{
}

SearchServer::SearchServer(std::string_view stop_words_text)
    : SearchServer(
        SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
    const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);
    std::vector<std::string> string_words;

    for (const std::string_view word : words) {
        //std::string new_word = { word.begin(), word.end() };
        string_words.push_back(std::string{ word.begin(), word.end() });
    }

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : string_words) {
        words_.push_back(word);

        word_to_document_freqs_[std::string_view(words_.back())][document_id] += inv_word_count;
        word_freq_[document_id][std::string_view(words_.back())] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
    int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string_view> matched_words;

    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { matched_words, documents_.at(document_id).status };
        }
    }

    for (const std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back((word_to_document_freqs_.find(word))->first);
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::sequenced_policy,
    std::string_view raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy& policy,
    std::string_view raw_query, int document_id) const {
    if (document_ids_.count(document_id) == 0) {
        throw std::out_of_range("out of range"s);
    }
    auto query = ParseQuery(raw_query, false);

    std::vector<std::string_view> matched_words;
    matched_words.reserve(query.plus_words.size());

    if (std::any_of(policy, query.minus_words.begin(), query.minus_words.end(),
        [&](const std::string_view minus_word) {
            return word_to_document_freqs_.at(minus_word).count(document_id);
        }
    )) {
        matched_words.clear();
        return { matched_words, documents_.at(document_id).status };
    }

    auto it = std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
        [&](const std::string_view& word) {
            return word_to_document_freqs_.at(word).count(document_id);
        }
    );

    matched_words.erase(it, matched_words.end());

    std::sort(policy, matched_words.begin(), matched_words.end());
    auto plus_last = std::unique(policy, matched_words.begin(), matched_words.end());
    matched_words.erase(plus_last, matched_words.end());

    return { matched_words, documents_.at(document_id).status };
}


const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    auto it = word_freq_.find(document_id);
    if (it != word_freq_.end()) {
        return it->second;
    }
    return empty_word_freq_;
}

void SearchServer::RemoveDocument(int document_id) {
    if (word_freq_.count(document_id) > 0) {
        for (auto [word, freq] : word_freq_.at(document_id)) {
            word_to_document_freqs_.at(word).erase(document_id);
        }
    }

    documents_.erase(document_id);
    document_ids_.erase(document_id);
    word_freq_.erase(document_id);
}

void SearchServer::RemoveDocument(std::execution::sequenced_policy, int document_id) {
    SearchServer::RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy& policy, int document_id) {
    if (word_freq_.count(document_id) > 0) {

        size_t size = word_freq_.at(document_id).size();
        std::vector<std::string_view> words(size);

        std::transform(std::execution::par, word_freq_.at(document_id).begin(), word_freq_.at(document_id).end(), words.begin(),
            [](auto a) {
                return a.first;
            });

        std::for_each(std::execution::par, words.begin(), words.end(),
            [&](std::string_view word) {
                word_to_document_freqs_.at(word).erase(document_id);
            }

        );
    }

    documents_.erase(document_id);
    document_ids_.erase(document_id);
    word_freq_.erase(document_id);
}



bool SearchServer::IsStopWord(std::string_view word) const {
    auto it = find(stop_words_.begin(), stop_words_.end(), word);
    return (it == stop_words_.end() ? false : true);
}

bool IsValidWord(const std::string_view word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> words;
    for (std::string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    // std::string_view word = text;
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Query word is invalid");
    }
    return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text, bool to_sort) const {
    Query result;

    for (std::string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            query_word.is_minus ? result.minus_words.push_back(query_word.data) : result.plus_words.push_back(query_word.data);
        }
    }
    if (to_sort) {
        std::sort(result.plus_words.begin(), result.plus_words.end());
        auto plus_last = std::unique(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(plus_last, result.plus_words.end());
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
 }