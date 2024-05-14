#ifndef DEQUE
#define DEQUE

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
  static const size_t kBucketSize = 16;

  template <bool IsConst>
  class Iterator;

  template <bool IsConst>
  friend class Iterator;

 public:
  using allocator_type = Allocator;
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator =
      std::reverse_iterator<const_iterator>::iterator_type;

  Deque() = default;
  Deque(const Allocator& alloc) : alloc_(alloc) {}

  Deque(size_t count, const Allocator& alloc = Allocator());
  Deque(size_t count, const T& value, const Allocator& alloc = Allocator());

  Deque(std::initializer_list<T> list, const Allocator& alloc = Allocator());
  Deque(const Deque& other);
  Deque(Deque&& other);
  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other);

  ~Deque() { clean(); }

  void swap(Deque<T, Allocator>& other);

  size_t size() const { return end_ - begin_; }
  bool empty() const { return size() == 0; }

  T& operator[](size_t idx) { return *(begin_ + idx); }
  const T& operator[](size_t idx) const { return *(begin_ + idx); }
  T& at(size_t idx);
  const T& at(size_t idx) const;

  void push_back(const T& elem);
  void push_front(const T& elem);
  void push_back(T&& elem);
  void push_front(T&& elem);
  void pop_back();
  void pop_front();
  void emplace_back(auto&&... args);
  void emplace_front(auto&&... args);

  iterator begin() { return begin_; }
  iterator end() { return end_; }
  const_iterator begin() const { return begin_; }
  const_iterator end() const { return end_; }
  const_iterator cbegin() const { return begin_; }
  const_iterator cend() const { return end_; }
  reverse_iterator rbegin() { return std::reverse_iterator(end_); }
  reverse_iterator rend() { return std::reverse_iterator(begin_); }
  const_reverse_iterator crbegin() const { return std::reverse_iterator(end_); }
  const_reverse_iterator crend() const { return std::reverse_iterator(begin_); }

  void insert(iterator iter, T value);
  void erase(iterator iter);

  bool operator==(Deque<T, Allocator> other) const;
  bool operator!=(Deque<T, Allocator> other) const {
    return not *this == other;
  }

  const allocator_type& get_allocator() { return alloc_; }

 private:
  using alloc_traits = std::allocator_traits<allocator_type>;
  using buffer_alloc = typename alloc_traits::template rebind_alloc<T*>;
  using buffer_alloc_traits = std::allocator_traits<buffer_alloc>;

  void move_fields_from_other(Deque<T, Allocator>&& other);
  void swap_fields_without_alloc(Deque<T, Allocator>& other);

  void expand_buffer();
  void realloc_buffer(size_t buckets, size_t offset);
  void clean();
  void destroy_elements();

  void destroy(iterator iter);
  void construct(iterator iter, auto&&... args);

  T* allocate_bucket();
  T** allocate_buffer(size_t buckets);
  void deallocate_buffer(T** buffer, size_t buckets);

  iterator begin_ = iterator();
  iterator end_ = iterator();
  size_t buckets_ = 0;
  T** buffer_ = nullptr;
  allocator_type alloc_;
  buffer_alloc buff_alloc_;
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
  friend Deque<T, Allocator>;

 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;

  Iterator() = default;
  Iterator(size_t index, T** buffer)
      : bucket_(index / kBucketSize),
        index_in_bucket_(index % kBucketSize),
        buffer_(buffer) {}
  Iterator(size_t bucket, size_t index_in_bucket, T** buffer)
      : bucket_(bucket), index_in_bucket_(index_in_bucket), buffer_(buffer) {}
  Iterator(const Iterator&) = default;
  Iterator& operator=(const Iterator&) = default;
  operator Iterator<true>() const {
    return Iterator<true>(bucket_, index_in_bucket_, buffer_);
  }

  Iterator& operator++() { return *this += 1; }
  Iterator& operator--() { return *this -= 1; }
  Iterator operator++(int);
  Iterator operator--(int);

  Iterator operator+(difference_type diff) const;
  Iterator operator-(difference_type diff) const;
  difference_type operator-(Iterator other) const;

  Iterator& operator+=(difference_type diff);
  Iterator& operator-=(difference_type diff);

  bool operator==(Iterator<IsConst> other) const;
  bool operator<(Iterator<IsConst> other) const;
  bool operator!=(Iterator<IsConst> other) const { return not(*this == other); }
  bool operator>(Iterator<IsConst> other) const { return other < *this; }
  bool operator<=(Iterator<IsConst> other) const { return not(*this < other); }
  bool operator>=(Iterator<IsConst> other) const { return not(*this > other); }

  reference operator*() { return buffer_[bucket_][index_in_bucket_]; }
  pointer operator->() { return buffer_[bucket_] + index_in_bucket_; }
  reference operator*() const { return buffer_[bucket_][index_in_bucket_]; }
  pointer operator->() const { return buffer_[bucket_] + index_in_bucket_; }

 protected:
  void replace_buffer(T** buffer) { buffer_ = buffer; }

 private:
  static const size_t kBucketSize = Deque<T, Allocator>::kBucketSize;

  size_t bucket_ = 0;
  size_t index_in_bucket_ = 0;
  T** buffer_ = nullptr;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : alloc_(alloc), buff_alloc_(alloc) {
  realloc_buffer(count / kBucketSize + (count % kBucketSize == 0 ? 0 : 1), 0);
  try {
    for (size_t i = 0; i < count; ++i) {
      emplace_back();
    }
  } catch (...) {
    clean();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc), buff_alloc_(alloc) {
  realloc_buffer(count / kBucketSize + (count % kBucketSize == 0 ? 0 : 1), 0);
  try {
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  } catch (...) {
    clean();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> list,
                           const Allocator& alloc)
    : alloc_(alloc), buff_alloc_(alloc) {
  realloc_buffer(
      list.size() / kBucketSize + (list.size() % kBucketSize == 0 ? 0 : 1), 0);
  try {
    for (const T& value : list) {
      push_back(value);
    }
  } catch (...) {
    clean();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)),
      buff_alloc_(alloc_) {
  size_t count = other.size();
  realloc_buffer(count / kBucketSize + (count % kBucketSize == 0 ? 0 : 1), 0);
  try {
    for (auto& val : other) {
      push_back(val);
    }
  } catch (...) {
    clean();
    throw;
  }
}
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other)
    : alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)),
      buff_alloc_(alloc_) {
  move_fields_from_other(std::move(other));
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    const Deque<T, Allocator>& other) {
  if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
    auto tmp(other);
    swap_fields_without_alloc(tmp);
    alloc_ = other.alloc_;
    buff_alloc_ = other.buff_alloc_;
  } else if (alloc_ == other.alloc_) {
    auto tmp(other);
    swap_fields_without_alloc(tmp);
  } else {
    auto copy(std::move(*this));

    try {
      for (auto& val : other) {
        emplace_back(val);
      }
    } catch (...) {
      clean();
      *this = std::move(copy);
    }
  }

  return *this;
}
template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    Deque<T, Allocator>&& other) {
  if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
    move_fields_from_other(std::move(other));
    alloc_ = std::move(other.alloc_);
    buff_alloc_ = std::move(other.buff_alloc_);
  } else if (alloc_ == other.alloc_) {
    move_fields_from_other(other);
  } else {
    destroy_elements();
    for (auto& val : other) {
      emplace_back(std::move(val));
    }
  }

  return *this;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap(Deque<T, Allocator>& other) {
  swap_fields_without_alloc(other);
  if constexpr (alloc_traits::propagate_on_container_swap::value) {
    std::swap(alloc_, other.alloc_);
    std::swap(buff_alloc_, other.buff_alloc_);
  }
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t idx) {
  if (idx >= size()) {
    throw std::out_of_range("out of range");
  }
  return operator[](idx);
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t idx) const {
  if (idx >= size()) {
    throw std::out_of_range("out of range");
  }
  return operator[](idx);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& elem) {
  emplace_back(elem);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& elem) {
  emplace_front(elem);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& elem) {
  emplace_back(std::forward<T>(elem));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& elem) {
  emplace_front(std::forward<T>(elem));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  end_ -= 1;
  destroy(end_);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  destroy(begin_);
  begin_ += 1;
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (buffer_ == nullptr or
      (end_ - 1).bucket_ == buckets_ - 1 and
          (end_ - 1).index_in_bucket_ == kBucketSize - 1) {
    expand_buffer();
  }
  if (buffer_[end_.bucket_] == nullptr) {
    buffer_[end_.bucket_] = allocate_bucket();
  }
  construct(end_, std::forward<Args>(args)...);
  end_ += 1;
}
template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (begin_.bucket_ == 0 and begin_.index_in_bucket_ == 0) {
    expand_buffer();
  }

  begin_ -= 1;
  if (buffer_[begin_.bucket_] == nullptr) {
    buffer_[begin_.bucket_] = allocate_bucket();
  }

  try {
    construct(begin_, std::forward<Args>(args)...);
  } catch (...) {
    begin_ += 1;
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(iterator iter, T value) {
  if (iter == begin_) {
    push_front(value);
    return;
  }
  if (iter == end_) {
    push_back(value);
    return;
  }

  size_t index = iter - begin_;
  push_back(*(end_ - 1));

  for (auto i = end_ - 1; i != begin_ + index + 1; --i) {
    *i = std::move(*(i - 1));
  }
  *(begin_ + index) = value;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(iterator iter) {
  for (; iter != end_ - 1; ++iter) {
    *iter = std::move(*(iter + 1));
  }
  end_ -= 1;
}

template <typename T, typename Allocator>
bool Deque<T, Allocator>::operator==(Deque<T, Allocator> other) const {
  if (size() != other.size()) {
    return false;
  }

  for (size_t i = 0; i < size(); ++i) {
    if (operator[](i) != other[i]) {
      return false;
    }
  }
  return true;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::move_fields_from_other(Deque<T, Allocator>&& other) {
  buffer_ = other.buffer_;
  begin_ = other.begin_;
  end_ = other.end_;
  buckets_ = other.buckets_;

  other.buffer_ = nullptr;
  other.begin_ = iterator();
  other.end_ = iterator();
  other.buckets_ = 0;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap_fields_without_alloc(
    Deque<T, Allocator>& other) {
  std::swap(buffer_, other.buffer_);
  std::swap(begin_, other.begin_);
  std::swap(end_, other.end_);
  std::swap(buckets_, other.buckets_);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::expand_buffer() {
  if (buckets_ == 0) {
    realloc_buffer(3, 1);
  } else {
    realloc_buffer(buckets_ * 3, buckets_);
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::realloc_buffer(size_t buckets, size_t offset) {
  T** allocated_buffer = allocate_buffer(buckets);

  if (buffer_ != nullptr) {
    std::copy(buffer_, buffer_ + buckets_, allocated_buffer + offset);
    buffer_alloc_traits::deallocate(buff_alloc_, buffer_, buckets_);
  }

  buffer_ = allocated_buffer;

  buckets_ = buckets;
  begin_.replace_buffer(allocated_buffer);
  end_.replace_buffer(allocated_buffer);
  begin_ += kBucketSize * offset;
  end_ += kBucketSize * offset;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::clean() {
  destroy_elements();
  deallocate_buffer(buffer_, buckets_);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::destroy_elements() {
  for (auto iter = begin_; iter != end_; ++iter) {
    destroy(iter);
  }

  begin_ = iterator(buckets_ / 2, buffer_);
  end_ = iterator(buckets_ / 2, buffer_);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::destroy(iterator iter) {
  alloc_traits::destroy(alloc_, buffer_[iter.bucket_] + iter.index_in_bucket_);
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::construct(iterator iter, Args&&... args) {
  alloc_traits::construct(alloc_, buffer_[iter.bucket_] + iter.index_in_bucket_,
                          std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
T* Deque<T, Allocator>::allocate_bucket() {
  return alloc_traits::allocate(alloc_, kBucketSize);
}

template <typename T, typename Allocator>
T** Deque<T, Allocator>::allocate_buffer(size_t buckets) {
  T** buffer = buffer_alloc_traits::allocate(buff_alloc_, buckets);
  for (size_t i = 0; i < buckets; ++i) {
    buffer[i] = nullptr;
  }
  return buffer;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::deallocate_buffer(T** buffer, size_t buckets) {
  if (buffer == nullptr) {
    return;
  }

  for (size_t i = 0; i < buckets; ++i) {
    if (buffer[i] == nullptr) {
      continue;
    }
    alloc_traits::deallocate(alloc_, buffer[i], kBucketSize);
  }
  buffer_alloc_traits::deallocate(buff_alloc_, buffer, buckets);
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator++(int) {
  auto copy = *this;
  ++*this;
  return copy;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator--(int) {
  auto copy = *this;
  --*this;
  return copy;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>&
Deque<T, Allocator>::Iterator<IsConst>::operator+=(
    Deque<T, Allocator>::Iterator<IsConst>::difference_type diff) {
  size_t index = index_in_bucket_ + bucket_ * kBucketSize + diff;
  bucket_ = index / kBucketSize;
  index_in_bucket_ = index % kBucketSize;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>&
Deque<T, Allocator>::Iterator<IsConst>::operator-=(
    Deque<T, Allocator>::Iterator<IsConst>::difference_type diff) {
  size_t index = index_in_bucket_ + bucket_ * kBucketSize - diff;
  bucket_ = index / kBucketSize;
  index_in_bucket_ = index % kBucketSize;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator+(
    Deque<T, Allocator>::Iterator<IsConst>::difference_type diff) const {
  auto copy = *this;
  copy += diff;
  return copy;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator-(
    Deque<T, Allocator>::Iterator<IsConst>::difference_type diff) const {
  auto copy = *this;
  copy -= diff;
  return copy;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>::difference_type
Deque<T, Allocator>::Iterator<IsConst>::operator-(Iterator other) const {
  return (bucket_ - other.bucket_) * kBucketSize + index_in_bucket_ -
         other.index_in_bucket_;
}

template <typename T, typename Allocator>
template <bool IsConst>
bool Deque<T, Allocator>::Iterator<IsConst>::operator==(
    Iterator<IsConst> other) const {
  return bucket_ == other.bucket_ and
         index_in_bucket_ == other.index_in_bucket_;
}

template <typename T, typename Allocator>
template <bool IsConst>
bool Deque<T, Allocator>::Iterator<IsConst>::operator<(
    Iterator<IsConst> other) const {
  if (bucket_ == other.bucket_) {
    return index_in_bucket_ < other.index_in_bucket_;
  }
  return bucket_ < other.bucket_;
}

#endif  // #ifndef DEQUE
