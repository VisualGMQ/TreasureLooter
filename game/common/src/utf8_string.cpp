#include "common/utf8_string.hpp"
#include <algorithm>

UTF8String::UTF8String() = default;

UTF8String::UTF8String(const char* utf8)
    : UTF8String(std::string_view(utf8)) {}

UTF8String::UTF8String(char32_t cp) {
    push_back(cp);
}

UTF8String::UTF8String(std::string_view utf8) : m_data(utf8) {
    buildOffsets();
}

char32_t UTF8String::operator[](size_t index) const {
    return decode(&m_data[m_offsets[index]]);
}

char32_t UTF8String::at(size_t index) const {
    return (*this)[index];
}

char32_t UTF8String::front() const {
    return (*this)[0];
}

char32_t UTF8String::back() const {
    return (*this)[m_offsets.size() - 1];
}

size_t UTF8String::size() const {
    return m_offsets.size();
}

size_t UTF8String::length() const {
    return m_offsets.size();
}

bool UTF8String::empty() const {
    return m_offsets.empty();
}

size_t UTF8String::byte_size() const {
    return m_data.size();
}

const std::string& UTF8String::str() const {
    return m_data;
}

const char* UTF8String::c_str() const {
    return m_data.c_str();
}

std::string_view UTF8String::view() const {
    return m_data;
}

void UTF8String::push_back(char32_t cp) {
    m_offsets.push_back(m_data.size());
    char buf[4];
    int len = encode(cp, buf);
    m_data.append(buf, len);
}

void UTF8String::insert(size_t pos, char32_t cp) {
    if (pos > m_offsets.size()) return;

    char buf[4];
    int byte_len = encode(cp, buf);

    size_t byte_pos =
        (pos < m_offsets.size()) ? m_offsets[pos] : m_data.size();
    m_data.insert(byte_pos, buf, byte_len);

    m_offsets.insert(m_offsets.begin() + pos, byte_pos);
    for (size_t i = pos + 1; i < m_offsets.size(); i++) {
        m_offsets[i] += byte_len;
    }
}

void UTF8String::erase(size_t pos, size_t count) {
    if (pos >= m_offsets.size() || count == 0) return;
    count = std::min(count, m_offsets.size() - pos);

    size_t byte_start = m_offsets[pos];
    size_t byte_end = (pos + count < m_offsets.size())
                          ? m_offsets[pos + count]
                          : m_data.size();
    size_t byte_count = byte_end - byte_start;

    m_data.erase(byte_start, byte_count);
    m_offsets.erase(m_offsets.begin() + pos, m_offsets.begin() + pos + count);
    for (size_t i = pos; i < m_offsets.size(); i++) {
        m_offsets[i] -= byte_count;
    }
}

void UTF8String::clear() {
    m_data.clear();
    m_offsets.clear();
}

void UTF8String::pop_back() {
    size_t byte_start = m_offsets.back();
    m_data.resize(byte_start);
    m_offsets.pop_back();
}

UTF8String& UTF8String::operator+=(char32_t cp) {
    push_back(cp);
    return *this;
}

UTF8String& UTF8String::operator+=(const UTF8String& other) {
    size_t old_size = m_data.size();
    m_data += other.m_data;
    for (size_t off : other.m_offsets) {
        m_offsets.push_back(old_size + off);
    }
    return *this;
}

UTF8String& UTF8String::operator+=(std::string_view utf8) {
    return *this += UTF8String(utf8);
}

UTF8String UTF8String::substr(size_t pos, size_t count) const {
    if (pos >= m_offsets.size()) return UTF8String();
    count = std::min(count, m_offsets.size() - pos);

    size_t byte_start = m_offsets[pos];
    size_t byte_end = (pos + count < m_offsets.size())
                          ? m_offsets[pos + count]
                          : m_data.size();

    return UTF8String(
        std::string_view(m_data).substr(byte_start, byte_end - byte_start));
}

size_t UTF8String::find(char32_t cp, size_t pos) const {
    for (size_t i = pos; i < m_offsets.size(); i++) {
        if (decode(&m_data[m_offsets[i]]) == cp) return i;
    }
    return npos;
}

bool UTF8String::operator==(const UTF8String& other) const {
    return m_data == other.m_data;
}

bool UTF8String::operator!=(const UTF8String& other) const {
    return m_data != other.m_data;
}

UTF8String::iterator UTF8String::begin() const {
    return iterator(this, 0);
}

UTF8String::iterator UTF8String::end() const {
    return iterator(this, m_offsets.size());
}

int UTF8String::seqLen(unsigned char first_byte) {
    if (first_byte < 0x80) return 1;
    if (first_byte < 0xC0) return 0;
    if (first_byte < 0xE0) return 2;
    if (first_byte < 0xF0) return 3;
    if (first_byte < 0xF8) return 4;
    return 0;
}

int UTF8String::codepointBytes(char32_t cp) {
    if (cp < 0x80) return 1;
    if (cp < 0x800) return 2;
    if (cp < 0x10000) return 3;
    return 4;
}

char32_t UTF8String::decode(const char* p) {
    unsigned char c = static_cast<unsigned char>(*p);
    if (c < 0x80) return c;
    int len = seqLen(c);
    char32_t cp = c & (0x7F >> len);
    for (int i = 1; i < len; i++) {
        cp = (cp << 6) | (static_cast<unsigned char>(p[i]) & 0x3F);
    }
    return cp;
}

int UTF8String::encode(char32_t cp, char* out) {
    if (cp < 0x80) {
        out[0] = static_cast<char>(cp);
        return 1;
    }
    if (cp < 0x800) {
        out[0] = static_cast<char>(0xC0 | (cp >> 6));
        out[1] = static_cast<char>(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = static_cast<char>(0xE0 | (cp >> 12));
        out[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out[2] = static_cast<char>(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = static_cast<char>(0xF0 | (cp >> 18));
    out[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    out[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out[3] = static_cast<char>(0x80 | (cp & 0x3F));
    return 4;
}

void UTF8String::buildOffsets() {
    m_offsets.reserve(m_data.size());
    size_t offset = 0;
    while (offset < m_data.size()) {
        m_offsets.push_back(offset);
        int len = seqLen(static_cast<unsigned char>(m_data[offset]));
        if (len <= 0 || offset + len > m_data.size()) break;
        offset += len;
    }
}

UTF8String::iterator::iterator(const UTF8String* str, size_t index)
    : m_string(str), m_index(index) {}

char32_t UTF8String::iterator::operator*() const {
    return (*m_string)[m_index];
}

UTF8String::iterator& UTF8String::iterator::operator++() {
    m_index++;
    return *this;
}

UTF8String::iterator UTF8String::iterator::operator++(int) {
    iterator tmp = *this;
    m_index++;
    return tmp;
}

UTF8String::iterator& UTF8String::iterator::operator--() {
    m_index--;
    return *this;
}

UTF8String::iterator UTF8String::iterator::operator--(int) {
    iterator tmp = *this;
    m_index--;
    return tmp;
}

bool UTF8String::iterator::operator==(const iterator& other) const {
    return m_string == other.m_string && m_index == other.m_index;
}

bool UTF8String::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}
