#include <iostream>

template <typename T>
class Deque {
private:
    T** _buffer;
    size_t _block_number;
    size_t _front_capacity;
    size_t _back_capacity;
    static const size_t _block_size = 32;

    void allocate_memory(size_t cap) {
        _block_number = cap / _block_size + 1;
        _front_capacity = (_block_size * _block_number - cap) / 2;
        _back_capacity = _block_size * _block_number - cap - _front_capacity;
        try {
            _buffer = new T*[_block_number];
        } catch (...) {
            throw;
        }
        for (size_t i = 0; i < _block_number; ++i) {
            try {
                _buffer[i] = reinterpret_cast<T*>(new char[_block_size * sizeof(T)]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    delete[] reinterpret_cast<char*>(_buffer[j]);
                }
                delete[] _buffer;
                throw;
            }
        }
    }

    T** reserve(size_t new_block_number) {
        T** new_buffer = nullptr;
        try {
            new_buffer = new T*[new_block_number];
        } catch (...) {
            throw;
        }
        for (size_t i = 0; i < _block_number; ++i) {
            try {
                new_buffer[i] = reinterpret_cast<T*>(new char[_block_size * sizeof(T)]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    delete[] reinterpret_cast<char*>(new_buffer[j]);
                }
                delete[] new_buffer;
                throw;
            }
        }
        for (size_t i = _block_number; i < 2 * _block_number; ++i) {
            new_buffer[i] = _buffer[i - _block_number];
        }
        for (size_t i = 2 * _block_number; i < new_block_number; ++i) {
            try {
                new_buffer[i] = reinterpret_cast<T*>(new char[_block_size * sizeof(T)]);
            } catch (...) {
                for (size_t j = _block_number; j < i; ++j) {
                    delete[] reinterpret_cast<char*>(new_buffer[j]);
                }
                delete[] new_buffer;
                throw;
            }
        }
        T** old_buffer = _buffer;
        _buffer = new_buffer;
        _front_capacity += _block_number * _block_size;
        _back_capacity += _block_number * _block_size;
        _block_number = new_block_number;
        return old_buffer;
    }

public:
    Deque() { allocate_memory(0); }

    Deque(size_t cap, const T& value = T()) {
        allocate_memory(cap);
        for (base_iterator it = begin(); it < end(); ++it) {
            try {
                new (it.operator->()) T(value);
            } catch (...) {
                for (base_iterator sec_iter = begin(); sec_iter < it; ++sec_iter) {
                    it->~T();
                }
                for (size_t i = 0; i < _block_number; ++i) {
                    delete[] reinterpret_cast<char*>(_buffer[i]);
                }
                delete[] _buffer;
                throw;
            }
        }
    }

    Deque(const Deque& deq) {
        allocate_memory(deq.size());
        for (size_t i = 0; i < size(); ++i) {
            try {
                new (&(operator[](i))) T(deq[i]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (operator[](j)).~T();
                }
                for (size_t k = 0; k < _block_number; ++k) {
                    delete[] reinterpret_cast<char*>(_buffer[i]);
                }
                delete[] _buffer;
                throw;
            }
        }
    }

    Deque& operator=(const Deque& deq) {
        Deque copy = deq;
        std::swap(_buffer, copy._buffer);
        std::swap(_block_number, copy._block_number);
        std::swap(_front_capacity, copy._front_capacity);
        std::swap(_back_capacity, copy._back_capacity);
        return *this;
    }

    size_t size() const { return (_block_number * _block_size) - _back_capacity - _front_capacity; }

    T& operator[](size_t index) { return *(begin() + index); }

    const T& operator[](size_t index) const { return *(cbegin() + index); }

    T& at(size_t index) {
        if (index >= size() || index < 0) {
            throw std::out_of_range("You have made some cringe!");
        }
        return operator[](index);
    }

    const T& at(size_t index) const {
        if (index >= size() || index < 0) {
            throw std::out_of_range("You have made some cringe!");
        }
        return operator[](index);
    }

    void push_back(const T& value) {
        if (_back_capacity == 0){
            reserve(_block_number * 3);
        }
        try {
            new ((end()).operator->()) T(value);
            --_back_capacity;
        } catch(...) {
            throw;
        }
    }

    void push_front(const T& value) {
        if (_front_capacity == 0) {
            reserve(_block_number * 3);
        }
        try {
            new ((--begin()).operator->()) T(value);
            --_front_capacity;
        } catch (...) {
            throw;
        }
    }

    void pop_front() {
        begin()->~T();
        ++_front_capacity;
    }

    void pop_back() {
        (--end())->~T();
        ++_back_capacity;
    }

    template <bool is_const>
    struct base_iterator {
    public:
        using value_type = std::conditional_t<is_const, const T, T>;
        using iterator_category = std::random_access_iterator_tag;
        using const_iterator = base_iterator<true>;

        T** _block_pointer;
        size_t _index;

        base_iterator(T** block_ptr, size_t index) : _block_pointer(block_ptr), _index(index) {}

        value_type& operator*() const { return (*_block_pointer)[_index]; }
        value_type* operator->() const { return *_block_pointer + _index; }

        operator const_iterator() { return const_iterator(_block_pointer, _index); }

        base_iterator& operator++() { return *this += 1; }

        base_iterator operator++(int) {
            base_iterator result = *this;
            ++*this;
            return result;
        }

        base_iterator& operator--() { return *this -= 1; }

        base_iterator operator--(int) {
            base_iterator result = *this;
            --*this;
            return result;
        }

        base_iterator& operator+=(int delta) {
            if (delta >= 0) {
                _block_pointer += (_index + delta) / _block_size;
                _index = (_index + delta) % _block_size;
                return *this;
            }
            return *this -= -delta;
        }

        base_iterator& operator-=(int delta) {
            if (delta >= 0) {
                _block_pointer -= (_block_size - _index + delta - 1) / _block_size;
                _index = _block_size - ((_block_size - _index + delta - 1) % _block_size) - 1;
                return *this;
            }
            return *this += -delta;
        }

        base_iterator operator+(int delta) const {
            base_iterator result = *this;
            result += delta;
            return result;
        }

        base_iterator operator-(int delta) const {
            base_iterator result = *this;
            result -= delta;
            return result;
        }

        int operator-(const base_iterator& other) const {
            return _block_size * (_block_pointer - other._block_pointer) + _index - other._index;
        }

        bool operator<(const base_iterator& other) const {
            if (_block_pointer < other._block_pointer) {
                return true;
            }
            if (_block_pointer > other._block_pointer) {
                return false;
            }
            return _index < other._index;
        }

        bool operator>(const base_iterator& other) const { return other < *this; }

        bool operator==(const base_iterator& other) const { return !(*this < other || other < *this); }

        bool operator!=(const base_iterator& other) const { return !(*this == other); }

        bool operator<=(const base_iterator& other) const { return !(other < *this); }

        bool operator>=(const base_iterator& other) const { return !(*this < other); }

    };

    using const_iterator = base_iterator<true>;
    using iterator = base_iterator<false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(_buffer + (_front_capacity / _block_size), _front_capacity % _block_size);
    }

    iterator end() {
        return iterator(_buffer + ((_front_capacity + size()) / _block_size), (_front_capacity + size()) % _block_size);
    }

    const_iterator begin() const { return cbegin(); }

    const_iterator end() const { return cend(); }

    const_iterator cbegin() const {
        return const_iterator(_buffer + (_front_capacity / _block_size), _front_capacity % _block_size);
    }

    const_iterator cend() const {
        return const_iterator(_buffer + ((_front_capacity + size()) / _block_size), (_front_capacity + size()) % _block_size);
    }

    reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

    reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cend()); }

    const_reverse_iterator crend() const { return std::make_reverse_iterator(cbegin()); }

    void insert(iterator it, const T& value) {
        if (it == end()) {
            push_back(value);
            return;
        }
        T* iter_pointer = it.operator->();
        push_back(value);
        for (iterator sec_it = end() - 1; sec_it.operator->() != iter_pointer; --sec_it) {
            std::swap(*sec_it, *(sec_it - 1));
        }
    }

    void erase(iterator it) {
        for (iterator sec_it = it + 1; sec_it != end(); ++sec_it) {
            std::swap(*sec_it, *(sec_it - 1));
        }
        pop_back();
    }

    ~Deque() {
        for (iterator it = begin(); it < end(); ++it) {
            try {
                it->~T();
            } catch (...) {
                continue;
            }
        }
        for (size_t i = 0; i < _block_number; ++i) {
            try {
                delete[] reinterpret_cast<char*>(_buffer[i]);
            } catch (...) {
                continue;
            }
        }
        try {
            delete[] _buffer;
        } catch (...) {}
    }
};
