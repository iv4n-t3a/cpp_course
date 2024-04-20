#ifndef LIST
#define LIST

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  template <bool IsConst>
  class Iterator;

 public:
  using value_type = T;
  using allocator_type = Allocator;
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  List() = default;
  List(size_t cnt, const T& value, const Allocator& alloc = Allocator());
  explicit List(size_t cnt, const Allocator& alloc = Allocator());
  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  List(const List& other);
  List& operator=(const List& other);
  ~List() { clear(alloc_, fake_node_); }

  iterator begin() { return iterator(fake_node_.next); }
  iterator end() { return iterator(fake_node_.prev->next); }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }
  reverse_iterator rbegin() { return std::reverse_iterator<iterator>(end()); }
  reverse_iterator rend() { return std::reverse_iterator<iterator>(begin()); }
  const_iterator cbegin() const { return const_iterator(fake_node_.next); };
  const_iterator cend() const { return const_iterator(fake_node_.next->prev); }

  T& front() { return fake_node_.next->data; }
  const T& front() const { return fake_node_.next.data; }
  T& back() { return fake_node_.prev->data; }
  const T& back() const { return fake_node_.prev->data; }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  void push_back_range(auto begin, auto end);

  void push_back(const T& elem);
  void push_front(const T& elem);
  void pop_back();
  void pop_front();

  allocator_type get_allocator() { return alloc_; }

 private:
  struct Node;

  struct BaseNode {
    BaseNode() {
      prev = static_cast<Node*>(this);
      next = static_cast<Node*>(this);
    }

    Node* prev = nullptr;
    Node* next = nullptr;
  };

  struct Node : public BaseNode {
    Node() : data(), BaseNode() {}
    Node(const T& elem) : data(elem) {}
    template <typename... Args>
    Node(Args&&... args) : data(std::forward<Args>(args)...) {}

    T data;
  };

  using alloc_traits = std::allocator_traits<Allocator>;
  using node_alloc = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = std::allocator_traits<node_alloc>;

  static void clear(node_alloc& alloc, BaseNode& fake_node);
  static void insert_node(Node* prev, Node* node);
  static void remove_node(node_alloc& alloc, Node* node);
  template <typename... Args>
  static Node* construct_node(node_alloc& alloc, Args&&... args);
  static void destroy_node(node_alloc& alloc, Node* node);

  BaseNode fake_node_;
  node_alloc alloc_;
  size_t size_ = 0;
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::Iterator {
 public:
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using cond_type = std::conditional_t<IsConst, const T, T>;
  using value_type = cond_type;
  using pointer = cond_type*;
  using reference = cond_type&;

  Iterator(Node* node) : node_(node) {}
  Iterator(const Iterator&) = default;
  Iterator& operator=(const Iterator&) = default;
  operator Iterator<true>() const { return Iterator<true>(node_); }

  reference operator*() const { return node_->data; }
  pointer operator->() const { return (T*)((size_t)node_ + sizeof(BaseNode)); }
  Iterator& operator++();
  Iterator& operator--();
  Iterator operator++(int);
  Iterator operator--(int);

  bool operator==(const Iterator& other) const { return node_ == other.node_; }
  bool operator!=(const Iterator& other) const { return node_ != other.node_; }

 private:
  Node* node_;
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t cnt, const Allocator& alloc) : alloc_(alloc) {
  try {
    for (size_t i = 0; i < cnt; ++i) {
      Node* node = construct_node(alloc_);
      insert_node(fake_node_.prev, node);
      size_ += 1;
    }
  } catch (...) {
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t cnt, const T& value, const Allocator& alloc)
    : alloc_(alloc) {
  try {
    for (size_t i = 0; i < cnt; ++i) {
      push_back(value);
    }
  } catch (...) {
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : alloc_(alloc) {
  push_back_range(init.begin(), init.end());
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List<T, Allocator>& other)
    : alloc_(node_alloc_traits::select_on_container_copy_construction(
          other.alloc_)) {
  push_back_range(other.cbegin(), other.cend());
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    const List<T, Allocator>& other) {
  node_alloc old_alloc = alloc_;

  if constexpr (node_alloc_traits::propagate_on_container_copy_assignment::
                    value) {
    alloc_ = other.alloc_;
  }

  try {
    push_back_range(other.cbegin(), other.cend());
  } catch (...) {
    throw;
  }

  for (; size_ != other.size_; --size_) {
    remove_node(old_alloc, fake_node_.next);
  }

  return *this;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back_range(auto begin, auto end) {
  size_t copyed = 0;
  try {
    for (auto iter = begin; iter != end; ++iter) {
      push_back(*iter);
      copyed += 1;
    }
  } catch (...) {
    for (size_t i = 0; i < copyed; ++i) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& elem) {
  Node* node = construct_node(alloc_, elem);
  insert_node(fake_node_.prev, node);
  size_ += 1;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& elem) {
  Node* node = construct_node(alloc_, elem);
  insert_node(static_cast<Node*>(&fake_node_), node);
  size_ += 1;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  remove_node(alloc_, fake_node_.prev);
  size_ -= 1;
}
template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  remove_node(alloc_, fake_node_.next);
  size_ -= 1;
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear(node_alloc& alloc, BaseNode& fake_node) {
  while (fake_node.next != static_cast<Node*>(&fake_node)) {
    remove_node(alloc, fake_node.next);
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::insert_node(Node* prev, Node* node) {
  node->prev = prev;
  node->next = prev->next;
  prev->next = node;
  node->next->prev = node;
}

template <typename T, typename Allocator>
void List<T, Allocator>::remove_node(node_alloc& alloc, Node* node) {
  node->prev->next = node->next;
  node->next->prev = node->prev;
  destroy_node(alloc, node);
}

template <typename T, typename Allocator>
template <typename... Args>
List<T, Allocator>::Node* List<T, Allocator>::construct_node(node_alloc& alloc,
                                                             Args&&... args) {
  Node* node = node_alloc_traits::allocate(alloc, 1);
  try {
    node_alloc_traits::construct(alloc, node, std::forward<Args>(args)...);
  } catch (...) {
    node_alloc_traits::deallocate(alloc, node, 1);
    throw;
  }
  return node;
}

template <typename T, typename Allocator>
void List<T, Allocator>::destroy_node(node_alloc& alloc,
                                      List<T, Allocator>::Node* node) {
  node_alloc_traits::destroy(alloc, node);
  node_alloc_traits::deallocate(alloc, node, 1);
}

template <typename T, typename Allocator>
template <bool IsConst>
List<T, Allocator>::Iterator<IsConst>&
List<T, Allocator>::Iterator<IsConst>::operator++() {
  node_ = node_->next;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
List<T, Allocator>::Iterator<IsConst>&
List<T, Allocator>::Iterator<IsConst>::operator--() {
  node_ = node_->prev;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
List<T, Allocator>::Iterator<IsConst>
List<T, Allocator>::Iterator<IsConst>::operator++(int) {
  Iterator copy = *this;
  this->operator++();
  return copy;
}

template <typename T, typename Allocator>
template <bool IsConst>
List<T, Allocator>::Iterator<IsConst>
List<T, Allocator>::Iterator<IsConst>::operator--(int) {
  Iterator copy = *this;
  this->operator++();
  return copy;
}

#endif  // #ifndef LIST
