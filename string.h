#include <algorithm>
#include <cstring>
#include <iostream>

class String {
private:
    size_t size_;
    size_t capacity_;
    char* string_;

    void realloc(size_t cap) {
        char* new_string = new char[cap]{};
        memcpy(new_string, string_, size_);
        delete[] string_;
        string_ = new_string;
        capacity_ = cap;
    }

    void swap(String& second) {
        std::swap(size_, second.size_);
        std::swap(capacity_, second.capacity_);
        std::swap(string_, second.string_);
    }

public:
    String();
    String(const char* c_string);
    String(size_t size, char symbol);
    String(const String& other);
    ~String();

    String& operator=(String other);
    String& operator+=(const String& other);
    String& operator+=(char other);

    size_t length() const { return size_; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_ - 1; }
    void push_back(char symbol);
    void pop_back() { size_--; }

    const char& operator[](size_t index) const { return string_[index]; }
    const char& front() const { return string_[0]; }
    const char& back() const { return string_[size_ - 1]; }
    char& operator[](size_t index) { return string_[index]; }
    char& front() { return string_[0]; }
    char& back() { return string_[size_ - 1]; }

    size_t find(const String& other) const;
    size_t rfind(const String& other) const;
    String substr(size_t index, size_t n) const;

    bool empty() const { return (size_ == 0); }
    void clear() { size_ = 0; }
    const char* data() const { return string_; }
    char* data() { return string_; }
    void shrink_to_fit() { realloc(size() + 1); }
};

bool operator<(const String& a, const String& b);
bool operator>(const String& a, const String& b);
bool operator<=(const String& a, const String& b);
bool operator>=(const String& a, const String& b);
bool operator==(const String& a, const String& b);
bool operator!=(const String& a, const String& b);

String operator+(const String& lhs, const String& rhs);
String operator+(char lhs, const String& rhs);
String operator+(const String& lhs, char rhs);

std::ostream& operator<<(std::ostream& os, const String& other);
std::istream& operator>>(std::istream& is, String& other);

String::String() : size_(0), capacity_(1), string_(nullptr) {}

String::String(const char* c_string)
        : size_(strlen(c_string)),
          capacity_(size_ + 1),
          string_(new char[capacity_]) {
    strcpy(string_, c_string);
    string_[size_] = '\0';
}

String::String(const size_t size, char symbol)
        : size_(size),
          capacity_(size + 1),
          string_(new char[capacity_]) {
    std::fill(string_, string_ + size, symbol);
    string_[size] = '\0';
}

String::String(const String& other)
        : size_(other.size_),
          capacity_(other.capacity_),
          string_(new char[capacity_]) {
    strcpy(string_, other.string_);
}

String::~String() { delete[] string_; }

String& String::operator=(String other) {
    swap(other);
    return *this;
}

bool operator<(const String& a, const String& b) {
    return (memcmp(a.data(), b.data(), std::max(a.size(), b.size())) == -1);
}

bool operator>(const String& a, const String& b) { return (b < a); }

bool operator<=(const String& a, const String& b) { return !(b < a); }

bool operator>=(const String& a, const String& b) { return !(a < b); }

bool operator==(const String& a, const String& b) {
    return (memcmp(a.data(), b.data(), std::max(a.size(), b.size())) == 0);
}

bool operator!=(const String& a, const String& b) { return !(a == b); }

void String::push_back(char symbol) {
    if (size_ >= capacity_ - 1) {
        realloc(capacity_ *= 2);
    }
    string_[size_++] = symbol;
    string_[size_] = '\0';
}

String& String::operator+=(const String& other) {
    realloc(other.length() + size_ + 1);
    std::copy(other.string_, other.string_ + other.length(), string_ + size_);
    size_ = strlen(string_);
    return *this;
}

String& String::operator+=(char other) {
    push_back(other);
    return *this;
}

String operator+(const String& lhs, const String& rhs) {
    String s = lhs;
    s += rhs;
    return s;
}

String operator+(char lhs, const String& rhs) {
    String s(1, lhs);
    s += rhs;
    return s;
}

String operator+(const String& lhs, char rhs) {
    String s = lhs;
    s += rhs;
    return s;
}

size_t String::find(const String& other) const {
    for (size_t i = 0; i < size_; i++) {
        if (memcmp(string_ + i, other.data(), other.size_) == 0) {
            return i;
        }
    }
    return size_;
}

size_t String::rfind(const String& other) const {
    size_t last_find = size_;
    for (size_t i = 0; i < size_; i++) {
        if (memcmp(string_ + i, other.data(), other.size_) == 0) {
            last_find = i;
        }
    }
    return last_find;
}

String String::substr(size_t index, size_t n) const {
    String sub(n, '0');
    memcpy(sub.string_, string_ + index, n);
    return sub;
}

std::ostream& operator<<(std::ostream& os, const String& other) {
    for (size_t i = 0; i < other.length(); ++i) {
        os << other[i];
    }
    return os;
}

std::istream& operator>>(std::istream& is, String& other) {
    char tmp;
    while (is.get(tmp) && !isspace(tmp)) {
        other.push_back(tmp);
    }
    return is;
}
