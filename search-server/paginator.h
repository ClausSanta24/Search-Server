#pragma once

#include <iostream>
#include <vector>

using namespace std::literals;

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        :begin_(begin), end_(end)
    {
    }

    auto begin() const {
        return begin_;
    }
    auto end() const {
        return end_;
    }
    auto size() const {
        return end_ - begin_;
    }
private:
    Iterator begin_;
    Iterator end_;

};

std::ostream& operator<<(std::ostream& output, Document document) {
    output << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
    return output;
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, IteratorRange<Iterator> iterator_range) {
    for (auto it = iterator_range.begin(); it != iterator_range.end(); ++it) {
        output << *it;
    }
    return output;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        if (begin < end) {
            for (auto it = begin; it != end; ++it) {
                if (std::distance(it, end) > page_size) {
                    auto it1 = it;
                    std::advance(it, page_size);
                    pages_.push_back(IteratorRange<Iterator>(it1, it));
                    --it;
                    continue;
                }
                if (std::distance(it, end) <= page_size) {
                    pages_.push_back(IteratorRange<Iterator>(it, end));
                    break;
                }
            }
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    auto size() const {
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}