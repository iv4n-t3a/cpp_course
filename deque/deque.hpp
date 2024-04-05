#ifndef DEQUE
#define DEQUE

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>

template <typename T>
class Deque {
  static const size_t kBucketSize = 16;

  template <bool IsConst>
  class Iterator;

  template <bool IsConst>
  friend class Iterator;

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator =
      std::reverse_iterator<const_iterator>::iterator_type;

  Deque() = default;
  Deque(size_t count) : Deque(count, T()) {}
  Deque(size_t count, const T& value);
  Deque(const Deque& other);
  Deque& operator=(const Deque& other);
  ~Deque() { clean(); }

  void swap(Deque<T> other);

  size_t size() const { return end_ - begin_; }
  bool empty() const { return size() == 0; }

  T& operator[](size_t idx) { return *(begin_ + idx); }
  const T& operator[](size_t idx) const { return *(begin_ + idx); }
  T& at(size_t idx);
  const T& at(size_t idx) const;

  void push_back(const T& elem);
  void pop_back();
  void push_front(const T& elem);
  void pop_front();

  iterator begin() { return begin_; }
  iterator end() { return end_; }
  const_iterator cbegin() const { return begin_; }
  const_iterator cend() const { return end_; }
  reverse_iterator rbegin() { return std::reverse_iterator(end_); }
  reverse_iterator rend() { return std::reverse_iterator(begin_); }
  const_reverse_iterator crbegin() const { return std::reverse_iterator(end_); }
  const_reverse_iterator crend() const { return std::reverse_iterator(begin_); }

  void insert(iterator iter, T value);
  void erase(iterator iter);

  bool operator==(Deque<T> other) const;
  bool operator!=(Deque<T> other) const { return not *this == other; }

 private:
  void expand_buffer();
  void realloc_buffer(size_t buckets, size_t offset);
  void clean();
  static T* new_bucket();
  static T** new_empty_buffer(size_t buckets);
  static void remove_buffer(T** buffer, size_t buckets);

  iterator begin_ = iterator();
  iterator end_ = iterator();
  size_t buckets_ = 0;
  T** buffer_ = nullptr;
};

template <typename T>
template <bool IsConst>
class Deque<T>::Iterator {
  friend Deque<T>;

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
  pointer operator->() { return &buffer_[bucket_][index_in_bucket_]; }
  reference operator*() const { return buffer_[bucket_][index_in_bucket_]; }
  pointer operator->() const { return &buffer_[bucket_][index_in_bucket_]; }

 protected:
  void replace_buffer(T** newbuffer) { buffer_ = newbuffer; }

 private:
  static const size_t kBucketSize = Deque<T>::kBucketSize;

  size_t bucket_ = 0;
  size_t index_in_bucket_ = 0;
  T** buffer_ = nullptr;
};

template <typename T>
Deque<T>::Deque(size_t count, const T& value) {
  realloc_buffer(count / kBucketSize + (count % kBucketSize == 0 ? 0 : 1), 0);
  for (size_t i = 0; i < count; ++i) {
    try {
      push_back(value);
    } catch (...) {
      clean();
      throw;
    }
  }
}

template <typename T>
Deque<T>::Deque(const Deque& other) {
  size_t count = other.size();
  realloc_buffer(count / kBucketSize + (count % kBucketSize == 0 ? 0 : 1), 0);
  for (size_t i = 0; i < count; ++i) {
    try {
      push_back(other[i]);
    } catch (...) {
      clean();
      throw;
    }
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  auto tmp(other);
  swap(tmp);
  return *this;
}

template <typename T>
void Deque<T>::swap(Deque<T> other) {
  std::swap(buffer_, other.buffer_);
  std::swap(begin_, other.begin_);
  std::swap(end_, other.end_);
  std::swap(buckets_, other.buckets_);
}

template <typename T>
T& Deque<T>::at(size_t idx) {
  if (idx >= size()) {
    throw std::out_of_range("out of range");
  }
  return operator[](idx);
}

template <typename T>
const T& Deque<T>::at(size_t idx) const {
  if (idx >= size()) {
    throw std::out_of_range("out of range");
  }
  return operator[](idx);
}

template <typename T>
void Deque<T>::push_back(const T& elem) {
  if (buffer_ == nullptr or
      (end_ - 1).bucket_ == buckets_ - 1 and
          (end_ - 1).index_in_bucket_ == kBucketSize - 1) {
    expand_buffer();
  }
  if (buffer_[end_.bucket_] == nullptr) {
    buffer_[end_.bucket_] = new_bucket();
  }

  try {
    new (buffer_[end_.bucket_] + end_.index_in_bucket_) T(elem);
    end_ += 1;
  } catch (...) {
    throw;
  }
}

template <typename T>
void Deque<T>::pop_back() {
  end_ -= 1;
  end_->~T();
}

template <typename T>
void Deque<T>::push_front(const T& elem) {
  if (begin_.bucket_ == 0 and begin_.index_in_bucket_ == 0) {
    expand_buffer();
  }

  begin_ -= 1;
  if (buffer_[begin_.bucket_] == nullptr) {
    buffer_[begin_.bucket_] = new_bucket();
  }

  try {
    new (buffer_[begin_.bucket_] + begin_.index_in_bucket_) T(elem);
  } catch (...) {
    begin_ += 1;
    throw;
  }
}

template <typename T>
void Deque<T>::pop_front() {
  begin_->~T();
  begin_ += 1;
}

template <typename T>
void Deque<T>::insert(iterator iter, T value) {
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
    *i = *(i - 1);
  }
  *(begin_ + index) = value;
}

template <typename T>
void Deque<T>::erase(iterator iter) {
  for (; iter != end_ - 1; ++iter) {
    *iter = *(iter + 1);
  }
  end_ -= 1;
}

template <typename T>
bool Deque<T>::operator==(Deque<T> other) const {
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

template <typename T>
void Deque<T>::expand_buffer() {
  if (buckets_ == 0) {
    realloc_buffer(3, 1);
  } else {
    realloc_buffer(buckets_ * 3, buckets_);
  }
}

template <typename T>
void Deque<T>::realloc_buffer(size_t buckets, size_t offset) {
  T** new_buffer = new_empty_buffer(buckets);

  if (buffer_ != nullptr) {
    std::copy(buffer_, buffer_ + buckets_, new_buffer + offset);
    delete[] buffer_;
  }

  buffer_ = new_buffer;

  buckets_ = buckets;
  begin_.replace_buffer(new_buffer);
  end_.replace_buffer(new_buffer);
  begin_ += kBucketSize * offset;
  end_ += kBucketSize * offset;
}

template <typename T>
void Deque<T>::clean() {
  for (auto iter = begin_; iter != end_; ++iter) {
    iter->~T();
  }
  remove_buffer(buffer_, buckets_);
}

template <typename T>
T* Deque<T>::new_bucket() {
  return reinterpret_cast<T*>(
      new std::byte[kBucketSize * sizeof(T) / sizeof(std::byte)]);
}

template <typename T>
T** Deque<T>::new_empty_buffer(size_t buckets) {
  T** buffer = new T*[buckets];
  for (size_t i = 0; i < buckets; ++i) {
    buffer[i] = nullptr;
  }
  return buffer;
}

template <typename T>
void Deque<T>::remove_buffer(T** buffer, size_t buckets) {
  if (buffer == nullptr) {
    return;
  }

  for (size_t i = 0; i < buckets; ++i) {
    if (buffer[i] == nullptr) {
      continue;
    }
    delete[] reinterpret_cast<int8_t*>(buffer[i]);
  }
  delete[] buffer;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst> Deque<T>::Iterator<IsConst>::operator++(int) {
  auto copy = *this;
  ++*this;
  return copy;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst> Deque<T>::Iterator<IsConst>::operator--(int) {
  auto copy = *this;
  --*this;
  return copy;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst>& Deque<T>::Iterator<IsConst>::operator+=(
    Deque<T>::Iterator<IsConst>::difference_type diff) {
  size_t index = index_in_bucket_ + bucket_ * kBucketSize + diff;
  bucket_ = index / kBucketSize;
  index_in_bucket_ = index % kBucketSize;
  return *this;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst>& Deque<T>::Iterator<IsConst>::operator-=(
    Deque<T>::Iterator<IsConst>::difference_type diff) {
  size_t index = index_in_bucket_ + bucket_ * kBucketSize - diff;
  bucket_ = index / kBucketSize;
  index_in_bucket_ = index % kBucketSize;
  return *this;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst> Deque<T>::Iterator<IsConst>::operator+(
    Deque<T>::Iterator<IsConst>::difference_type diff) const {
  auto copy = *this;
  copy += diff;
  return copy;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst> Deque<T>::Iterator<IsConst>::operator-(
    Deque<T>::Iterator<IsConst>::difference_type diff) const {
  auto copy = *this;
  copy -= diff;
  return copy;
}

template <typename T>
template <bool IsConst>
Deque<T>::Iterator<IsConst>::difference_type
Deque<T>::Iterator<IsConst>::operator-(Iterator other) const {
  return (bucket_ - other.bucket_) * kBucketSize + index_in_bucket_ -
         other.index_in_bucket_;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::Iterator<IsConst>::operator==(Iterator<IsConst> other) const {
  return bucket_ == other.bucket_ and
         index_in_bucket_ == other.index_in_bucket_;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::Iterator<IsConst>::operator<(Iterator<IsConst> other) const {
  if (bucket_ == other.bucket_) {
    return index_in_bucket_ < other.index_in_bucket_;
  }
  return bucket_ < other.bucket_;
}

#endif  // #ifndef DEQUE
