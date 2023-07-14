#include <memory>
#include <vector>
#include <iterator>

template <typename T, typename A = std::allocator<T>>
class List {
 private:
  struct BaseNode {
    BaseNode* from = nullptr;
    BaseNode* to = nullptr;
  };

  struct Node : BaseNode {
    T value;

    Node() = default;
    Node(const T& value) : value(value) {}
  };

  using base_node_alloc = typename std::allocator_traits<A>::template rebind_alloc<BaseNode>;
  using base_node_traits = typename std::allocator_traits<A>::template rebind_traits<BaseNode>;
  using node_alloc = typename std::allocator_traits<A>::template rebind_alloc<Node>;
  using node_traits = typename std::allocator_traits<A>::template rebind_traits<Node>;
  using traits = std::allocator_traits<A>;
  A alloc_;

  size_t size_;
  BaseNode* empty_node_;

  void construct_empty_node() {
    base_node_alloc allocator = alloc_;
    empty_node_ = base_node_traits::allocate(allocator, 1);
    empty_node_->from = empty_node_;
    empty_node_->to = empty_node_;
  }

  void swap(List<T, A>& other) {
    std::swap(alloc_, other.alloc_);
    std::swap(size_, other.size_);
    std::swap(empty_node_, other.empty_node_);
  }

 public:
  A& get_allocator() { return alloc_; }

  size_t size() const { return size_; }

  List() : alloc_(A()), size_(0) {
    construct_empty_node();
  }

  List(size_t n, const T& value, const A& alloc) : alloc_(alloc), size_(0) {
    construct_empty_node();
    while (size_ < n) push_front(value);
  }

  List(size_t n, const A& alloc) : alloc_(alloc) {
    size_ = 0;
    construct_empty_node();
    Node* tmp = empty_node_;

    node_alloc allocator = alloc_;
    while (size_ < n) {
      tmp->to = allocator.allocate(1);
      node_traits::construct(allocator, tmp->to);
      tmp->to->from = tmp;
      tmp = tmp->to;
      size_++;
    }
    empty_node_->from = tmp;
    tmp->to = empty_node_;
  }

  List(const A& alloc) : alloc_(traits::select_on_container_copy_construction(alloc)) {
    size_ = 0;
    construct_empty_node();
  }

  List(const List<T, A>& other) :
          alloc_(traits::select_on_container_copy_construction(other.alloc_)) {
    size_ = 0;
    construct_empty_node();
    for (auto& iter : other) {
      push_back(iter);
    }
  }

  List(List&& other) : List() {
    if (this != &other) swap(other);
  }

  List& operator=(List<T, A>&& other) {
    if (this != &other) {
      while (size_ > 0) erase(cbegin());
      swap(other);
    }
    return *this;
  }

  List& operator=(const List<T, A>& other) {
    if (this != &other) {
      while (size_ > 0) erase(cbegin());
      if (node_traits::propagate_on_container_copy_assignment::value) {
        alloc_ = other.alloc_;
      }
      construct_empty_node();
      for (auto& iter : other) {
        push_back(iter);
      }
    }
    return *this;
  }

  ~List() {
    while (size_ > 0) erase(cbegin());
    base_node_alloc allocator = alloc_;
    base_node_traits::deallocate(allocator, empty_node_, 1);
  }

  template <bool is_const>
  struct Iterator {
   private:
    BaseNode* node_;

   public:
    using value_type = std::conditional_t<is_const, const T, T>;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = long long;

    Iterator() = default;

    Iterator(const Iterator<is_const> &iter) {
      node_ = iter.get_node_ptr();
    }

    Iterator(BaseNode* node) {
      node_ = node;
    }

    Iterator<is_const> &operator=(const Iterator<is_const> &iter) {
      node_ = iter.get_node_ptr();
      return *this;
    }

    reference operator*() { return static_cast<Node*>(node_)->value; }

    pointer operator->() { return &(operator*()); }

    Iterator<is_const> &operator++() {
      node_ = node_->to;
      return *this;
    }

    Iterator<is_const> operator++(int) {
      Iterator<is_const> tmp(*this);
      ++(*this);
      return tmp;
    }

    Iterator<is_const> &operator--() {
      node_ = node_->from;
      return *this;
    }

    Iterator<is_const> operator--(int) {
      Iterator<is_const> tmp(*this);
      --(*this);
      return tmp;
    }

    bool operator==(const Iterator& iter) const {
      return node_ == iter.get_node_ptr();
    }

    bool operator!=(const Iterator& iter) const {
      return node_ != iter.get_node_ptr();
    }

    operator bool() const {
      return node_ != nullptr;
    }

    operator Iterator<true>() const {
      Iterator<true> const_iter(node_);
      return const_iter;
    }

    BaseNode* get_node_ptr() const { return node_; }
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() noexcept {
    return iterator(empty_node_->to);
  }

  const_iterator begin() const noexcept {
    return cbegin();
  }

  const_iterator cbegin() const noexcept {
    return const_iterator(empty_node_->to);
  }

  reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const noexcept {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator rbegin() const noexcept {
    return crbegin();
  }

  iterator end() noexcept {
    return iterator(empty_node_);
  }

  const_iterator end() const noexcept {
    return cend();
  }

  const_iterator cend() const noexcept {
    return const_iterator(empty_node_);
  }

  reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator rend() const noexcept {
    return crend();
  }

  template <typename... Args>
  void emplace(const_iterator iter, Args&&... args) {
    node_alloc allocator = alloc_;
    Node* node = static_cast<Node*>(iter.get_node_ptr());
    Node* new_node = node_traits::allocate(allocator, 1);
    traits::construct(alloc_, &(new_node->value), std::forward<Args>(args)...);
    new_node->to = node;
    new_node->from = node->from;
    node->from->to = new_node;
    node->from = new_node;
    ++size_;
  }

  void insert(const_iterator iter, const T& value) {
    emplace(iter, value);
  }

  void erase(const_iterator iter) {
    node_alloc allocator = alloc_;
    Node* tmp = static_cast<Node*>(iter.get_node_ptr());
    tmp->to->from = tmp->from;
    tmp->from->to = tmp->to;
    traits::destroy(alloc_, tmp);
    node_traits::deallocate(allocator, tmp, 1);
    --size_;
  }

  void push_back(const T &value) {
    insert(end(), value);
  }

  void push_front(const T &value) {
    insert(begin(), value);
  }

  void pop_back() {
    erase(--cend());
  }

  void pop_front() {
    erase(cbegin());
  }
};

template<
        typename Key,
        typename Value,
        typename Hash = std::hash<Key>,
        typename Equal = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, Value>>
>
class UnorderedMap {
 private:
  using NodeType = std::pair<const Key, Value>;
  using AllocTraits = std::allocator_traits<Alloc>;
  using ListType = List<NodeType, typename AllocTraits::template rebind_alloc<NodeType>>;
  using ListIterator = typename ListType::iterator;
  using ListConstIterator = typename ListType::const_iterator;
  using ListIteratorAlloc = typename AllocTraits::template rebind_alloc<ListIterator>;
  using iterator = typename ListType::template Iterator<false>;
  using const_iterator = typename ListType::template Iterator<true>;

  static constexpr size_t START_BUCKET_COUNT = 64;
  static constexpr size_t MAX_SEARCH_DIST = 16;
  static constexpr float DEFAULT_MAX_LOAD_FACTOR = 0.8f;

  Hash hash_;
  Equal equal_;
  Alloc alloc_;
  std::vector<ListIterator, ListIteratorAlloc> hash_table_;
  ListType list_;

  void rehash() {
    bool tmp = true;
    const size_t count = hash_table_.size();
    hash_table_.assign(count, nullptr);
    for (auto iter = list_.begin(); iter != list_.end(); ++iter) {
      const size_t hash_id = hash_(iter->first) % count;
      const size_t end = std::min(count, hash_id + MAX_SEARCH_DIST);
      for (size_t i = hash_id; i < end; ++i) {
        if (!hash_table_[i % count]) {
          hash_table_[i % count] = iter;
          break;
        } else if (i == end - 1) {
          tmp = false;
        }
      }
      if (!tmp) {
        break;
      }
    }
    if (!tmp) {
      hash_table_.assign(count, nullptr);
      for (auto iter = list_.begin(); iter != list_.end(); ++iter) {
        const size_t hash_id = hash_(iter->first) % count;
        if (!hash_table_[hash_id]) {
          hash_table_[hash_id] = iter;
        } else {
          const size_t end = std::min(count, hash_id + MAX_SEARCH_DIST);
          size_t i;
          for (i = hash_id + 1; i < end && hash_table_[i % count]; ++i) {}
          hash_table_[i % count] = iter;
        }
      }
    }
  }

  float load_factor() const {
    return static_cast<float>(list_.size()) / hash_table_.size();
  }

  void swap(const UnorderedMap& other) {
    std::swap(hash_table_, other.hash_table);
    std::swap(list_, other.list_);
    std::swap(hash_, other.hash_);
    std::swap(equal_, other.equal_);
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }
  }

 public:
  size_t size() const { return list_.size(); }

  UnorderedMap() : alloc_(Alloc()), hash_table_(alloc_), list_(alloc_) {
    reserve(START_BUCKET_COUNT);
  }

  UnorderedMap(const UnorderedMap& other) :
          alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)),
          hash_table_(alloc_),
          list_(other.list_) {
    reserve(other.hash_table_.size());
    rehash();
  }

  UnorderedMap(UnorderedMap&& other) :
          alloc_(std::move(other.alloc_)),
          hash_table_(std::move(other.hash_table_)),
          list_(std::move(other.list_)) {
    reserve(other.hash_table_.size());
    rehash();
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    if (this != &other) {
      UnorderedMap tmp(other);
      swap(tmp);
    }
    return *this;
  }

  UnorderedMap& operator=(UnorderedMap&& other) {
    if (this != &other) {
      hash_table_ = std::move(other.hash_table_);
      list_ = std::move(other.list_);
      hash_ = std::move(other.hash_);
      equal_ = std::move(other.equal_);
      alloc_ = std::move(other.alloc_);
    }
    return *this;
  }

  iterator begin() { return list_.begin(); }

  const_iterator begin() const { return cbegin(); }

  const_iterator cbegin() const { return list_.cbegin(); }

  iterator end() { return list_.end(); }

  const_iterator end() const { return cend(); }

  const_iterator cend() const { return list_.cend(); }

  void reserve(size_t n) {
    if (n > hash_table_.size()) {
      hash_table_.resize(n);
      rehash();
    }
  }

  iterator find(const Key& key) {
    size_t hash_id = hash_(key) % hash_table_.size();
    size_t i = 0;
    for (;;) {
      const size_t index = (hash_id + i) % hash_table_.size();
      if (!hash_table_[index]) {
        return list_.end();
      }
      if (equal_(key, hash_table_[index]->first)) {
        return hash_table_[index];
      }
      ++i;
      if (i == MAX_SEARCH_DIST) {
        return list_.end();
      }
    }
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    list_.emplace(list_.end(), std::forward<Args>(args)...);
    iterator element = --list_.end();
    iterator iter = find(element->first);
    if (iter != list_.end() && equal_(element->first, iter->first) && iter == element) {
      return std::make_pair(iter, true);
    } else if (iter != list_.end() && equal_(element->first, iter->first)) {
      list_.pop_back();
      return std::make_pair(iter, false);
    }
    while (true) {
      size_t sz = hash_table_.size();
      size_t hash_id = hash_(element->first) % sz;
      size_t i;
      const size_t end = std::min(sz, hash_id + MAX_SEARCH_DIST);
      for (i = hash_id; i < end && hash_table_[i % sz]; ++i) {}
      if (!hash_table_[i % sz]) {
        hash_table_[i % sz] = element;
        break;
      } else {
        reserve(sz * 2);
      }
    }
    if (load_factor() > DEFAULT_MAX_LOAD_FACTOR) {
      reserve(hash_table_.size() * 2);
    }
    return std::make_pair(element, true);
  }

  void insert(const NodeType& node) {
    emplace(node);
  }

  void insert(NodeType&& node) {
    if (load_factor() >= DEFAULT_MAX_LOAD_FACTOR) {
      reserve(hash_table_.size() * 2);
    }
    emplace(std::move(const_cast<Key&>(node.first)), std::move(node.second));
  }


  template <typename InputIterator>
  void insert(const InputIterator& first, const InputIterator& second) {
    for (auto iter = first; iter != second; ++iter) {
      emplace(*iter);
    }
  }

  void erase(iterator iter) {
    size_t hash_id = hash_(iter->first) % hash_table_.size();
    for (size_t i = 0; i < MAX_SEARCH_DIST; ++i) {
      if (hash_table_[(hash_id + i) % hash_table_.size()] == iter) {
        hash_table_[(hash_id + i) % hash_table_.size()] = iterator();
        break;
      }
    }
    list_.erase(iter);
  }

  void erase(const Key& key) {
    iterator iter = find(key);
    if (iter == list_.end()) {
      return;
    }
    erase(iter);
  }

  void erase(iterator first, iterator second) {
    while (first != second) {
      iterator iter = first++;
      erase(iter);
    }
  }

  Value& operator[](const Key& key) {
    iterator iter = find(key);
    if (iter == list_.end()) {
      return emplace(key, Value()).first->second;
    } else {
      return iter->second;
    }
  }

  Value& at(const Key& key) {
    iterator iter = find(key);
    if (iter != end()) {
      return iter->second;
    }
    throw std::out_of_range("You have made some cringe!");
  }
};
