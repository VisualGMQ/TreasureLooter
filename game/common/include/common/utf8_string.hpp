#pragma once
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

class UTF8String {
public:
    class iterator;

    UTF8String();
    explicit UTF8String(std::string_view utf8);
    explicit UTF8String(const char* utf8);
    explicit UTF8String(char32_t cp);

    char32_t operator[](size_t index) const;
    char32_t at(size_t index) const;
    char32_t front() const;
    char32_t back() const;
    size_t size() const;
    size_t length() const;
    bool empty() const;
    size_t byte_size() const;

    const std::string& str() const;
    const char* c_str() const;
    std::string_view view() const;

    void push_back(char32_t cp);
    void insert(size_t pos, char32_t cp);
    void erase(size_t pos, size_t count = 1);
    void clear();
    void pop_back();

    UTF8String& operator+=(char32_t cp);
    UTF8String& operator+=(const UTF8String& other);
    UTF8String& operator+=(std::string_view utf8);

    UTF8String substr(size_t pos, size_t count = npos) const;

    size_t find(char32_t cp, size_t pos = 0) const;

    bool operator==(const UTF8String& other) const;
    bool operator!=(const UTF8String& other) const;

    iterator begin() const;
    iterator end() const;

    static constexpr size_t npos = static_cast<size_t>(-1);

private:
    std::string m_data;
    std::vector<size_t> m_offsets;

    static int seqLen(unsigned char first_byte);
    static int codepointBytes(char32_t cp);
    static char32_t decode(const char* p);
    static int encode(char32_t cp, char* out);

    void buildOffsets();
};

class UTF8String::iterator {
    friend class UTF8String;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = char32_t;

    char32_t operator*() const;
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

private:
    const UTF8String* m_string;
    size_t m_index;

    iterator(const UTF8String* str, size_t index);
};
