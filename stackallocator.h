#include <cstddef>
#include <iostream>
#include <memory>

template <size_t N>
class StackStorage {
 private:
  void *storage_;
  size_t begin_;

 public:
  StackStorage() {
    storage_ = malloc(N);
    begin_ = 0;
  }

  ~StackStorage() { free(storage_); }

  StackStorage(const StackStorage &other) = delete;

  void *allocate(size_t n, size_t align) {
    if (begin_ % align) {
      begin_ += align - (begin_ % align);
    }
    begin_ += n;
    return reinterpret_cast<char *>(storage_) + begin_ - n;
  }

  void deallocate(void *, size_t) {}
};

template <typename T, size_t N>
class StackAllocator {
 private:
  StackStorage<N> *stack_storage_;

 public:
  using value_type = T;

  template <typename A>
  struct rebind {
    using other = StackAllocator<A, N>;
  };

  StackStorage<N> *get_storage() const { return stack_storage_; }

  StackAllocator() : stack_storage_() {}

  StackAllocator(StackStorage<N> &storage_) : stack_storage_(&storage_) {}

  template <typename A>
  StackAllocator(const StackAllocator<A, N> &other) : stack_storage_(other.get_storage()) {}

  template <typename A>
  StackAllocator &operator=(const StackAllocator<A, N> &other) {
    stack_storage_ = other.get_storage();
    return *this;
  }

  bool operator==(StackAllocator<T, N> &other) { return stack_storage_ == other.get_storage(); }

  bool operator!=(StackAllocator<T, N> &other) { return stack_storage_ != other.get_storage(); }

  value_type *allocate(size_t n) {
    return reinterpret_cast<value_type *>(
        stack_storage_->allocate(n * sizeof(value_type), sizeof(value_type)));
  }

  void deallocate(value_type *ptr, size_t n) {
    std::ignore = ptr;
    std::ignore = n;
  }
};

template <class T, class A = std::allocator<T>>
class List {
 private:
  struct BaseNode {
    BaseNode *prev = nullptr;
    BaseNode *next = nullptr;

    BaseNode(BaseNode *prev_old, BaseNode *next_old): prev(prev_old), next(next_old) {}
    BaseNode(): prev(nullptr), next(nullptr) {}

  };

  struct Node: BaseNode {
    T value;

    Node(BaseNode *prev_old, BaseNode *next_old) : BaseNode(prev_old, next_old) {}
    Node(const T &value_old, BaseNode *prev_old, BaseNode *next_old) : BaseNode(prev_old, next_old), value(value_old) {}
  };

  using node_alloc = typename std::allocator_traits<A>::template rebind_alloc<Node>;
  using alloc_traits = std::allocator_traits<node_alloc>;
  node_alloc allocator_;
  BaseNode empty_node_;
  size_t size_;

  template <bool is_const>
  class Iterator {
   private:
    BaseNode *node_;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional_t<is_const, const T, T>;
    using difference_type = long long;
    using pointer = value_type *;
    using reference = value_type &;

    Iterator() = default;

    Iterator(const Iterator<is_const> &iter) noexcept : node_(iter.node_) {}

    operator Iterator<true>() const noexcept {
      Iterator<true> const_iter(node_);
      return const_iter;
    }

    explicit Iterator(BaseNode *new_node) noexcept : node_(new_node) {}
    explicit Iterator(const BaseNode *new_node) noexcept : node_(const_cast<BaseNode*>(new_node)) {}

    BaseNode *get_node() const noexcept { return node_; }

    Iterator<is_const> &operator++() noexcept {
      node_ = node_->next;
      return *this;
    }

    Iterator<is_const> operator++(int) noexcept {
      Iterator<is_const> tmp(*this);
      ++(*this);
      return tmp;
    }

    Iterator<is_const> &operator--() noexcept {
      node_ = node_->prev;
      return *this;
    }

    Iterator<is_const> operator--(int) noexcept {
      Iterator<is_const> tmp(*this);
      --(*this);
      return tmp;
    }

    Iterator<is_const> &operator=(const Iterator<is_const> &iter) noexcept {
      node_ = iter.node_;
      return *this;
    }

    value_type &operator*() const noexcept { return reinterpret_cast<Node*>(node_)->value; }

    value_type *operator->() const noexcept { return &(reinterpret_cast<Node*>(node_)->value); }

    bool operator==(const Iterator<is_const> &iter) const noexcept { return node_ == iter.node_; }

    bool operator!=(const Iterator<is_const> &iter) const noexcept { return !(*this == iter); }

    ~Iterator() = default;
  };

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  A get_allocator() const noexcept { return allocator_; }

  size_t size() const noexcept { return size_; }

  void add_elements(const List &other) {
    for (auto it = other.begin(); it != other.end(); ++it) {
      insert(end(), *it);
    }
  }

  List &operator=(const List& other_list) {
    auto tmp = other_list;
    if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
      allocator_ = other_list.get_allocator();
    }
    std::swap(tmp.empty_node_, empty_node_);
    std::swap((&tmp.empty_node_)->prev->next, (&empty_node_)->prev->next);
    std::swap((&tmp.empty_node_)->next->prev, (&empty_node_)->next->prev);
    std::swap(tmp.size_, size_);
    return *this;
  }

  void insert(const_iterator iter, const T &value) {
    BaseNode *next_node = iter.get_node();
    BaseNode *prev_node = next_node->prev;
    BaseNode *new_node;
    new_node = alloc_traits::allocate(allocator_, 1);
    try {
      alloc_traits::construct(allocator_, reinterpret_cast<Node*>(new_node), value, next_node, prev_node);
    } catch (...) {
      alloc_traits::deallocate(allocator_, reinterpret_cast<Node*>(new_node), 1);
      throw;
    }
    new_node->prev = prev_node;
    prev_node->next = new_node;
    new_node->next = next_node;
    next_node->prev = new_node;
    ++size_;
  }

  void erase(const_iterator iter) {
    BaseNode *tmp = iter.get_node();
    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
    alloc_traits::destroy(allocator_, static_cast<Node*>(tmp));
    alloc_traits::deallocate(allocator_, static_cast<Node*>(tmp), 1);
    --size_;
  }

  void push_back(const T &item) { insert(end(), item); }

  void push_front(const T &item) { insert(begin(), item); }

  void pop_back() { erase(std::prev(end())); }

  void pop_front() { erase(begin()); }

  iterator begin() noexcept { return iterator((&empty_node_)->next); }

  iterator end() noexcept { return iterator(&empty_node_); }

  const_iterator begin() const noexcept { return cbegin(); }

  const_iterator end() const noexcept { return cend(); }

  const_iterator cbegin() const noexcept { return const_iterator((&empty_node_)->next); }

  const_iterator cend() const noexcept { return const_iterator(&empty_node_); }

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cend()); }

  const_reverse_iterator crend() const { return std::make_reverse_iterator(cbegin()); }

  const_reverse_iterator rbegin() const noexcept { return crbegin(); }

  const_reverse_iterator rend() const noexcept { return crend(); }

  List(const A &alloc = A()) : allocator_(alloc), empty_node_(&empty_node_, &empty_node_), size_(0) {

  }

  List(size_t n, const T &value, const A &alloc) : List(alloc) {
    for (size_t i = 0; i < n; ++i) {
      insert(end(), value);
    }
  }

  List(size_t n, const A &alloc = A()) : List(alloc) {
    BaseNode *tmp = &empty_node_;
    BaseNode *new_node;
    for (size_t i = 0; i < n; ++i) {
      new_node = alloc_traits::allocate(allocator_, 1);
      try {
        alloc_traits::construct(allocator_, reinterpret_cast<Node*>(new_node), new_node, new_node);
      } catch (...) {
        alloc_traits::deallocate(allocator_, reinterpret_cast<Node*>(new_node), 1);
        throw;
      }
      tmp->next = new_node;
      new_node->prev = tmp;
      tmp = new_node;
      ++size_;
    }
    tmp->next = &empty_node_;
    (&empty_node_)->prev = tmp;
  }

  List(const List &other_list)
      : List(std::allocator_traits<A>::select_on_container_copy_construction(
      other_list.get_allocator())) {
    add_elements(other_list);
  }

  ~List() {
    while (size_ > 0) pop_front();
  }
};
