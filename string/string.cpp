#include "string.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

String::String() { Reserve(1); }
String::String(size_t size, char character) {
  Resize(size);
  for (size_t i = 0; i < size; ++i) {
    str_[i] = character;
  }
}
String::String(char const* str) {
  if (strlen(str) != 0) {
    Resize(strlen(str));
    memcpy(str_, str, sizeof(char) * size_);
  } else {
    Reserve(1);
  }
}
String::String(String const& other) {
  if (size_ < other.size_) {
    Resize(other.size_);
  }
  memcpy(str_, other.str_, other.size_);
}
String& String::operator=(String const& other) {
  Resize(other.size_);
  memcpy(str_, other.str_, other.size_);
  return *this;
}
String::~String() { free(str_); }

String String::operator+(String const& other) const {
  String buf = *this;
  buf += other;
  return buf;
}
String& String::operator+=(String const& other) {
  if (other.size_ + size_ > capacity_) {
    Reserve(other.size_ + size_);
    memcpy(str_ + size_ * sizeof(char), other.str_, other.size_ * sizeof(char));
    Resize(size_ + other.size_);
  } else {
    memcpy(str_ + size_ * sizeof(char), other.str_, other.size_ * sizeof(char));
  }
  return *this;
}

String String::operator*(size_t mult) const {
  String buf = *this;
  buf *= mult;
  return buf;
}
String& String::operator*=(size_t mult) {
  if (mult == 0) {
    Clear();
  } else {
    Reserve(mult * size_);
    for (size_t i = 1; i < mult; ++i) {
      memcpy(str_ + i * size_ * sizeof(char), str_, size_ * sizeof(char));
    }
    Resize(mult * size_);
  }
  return *this;
}

bool String::operator==(String const& other) const {
  if (size_ != other.size_) {
    return false;
  }

  for (size_t i = 0; i < size_; ++i) {
    if (str_[i] != other.str_[i]) {
      return false;
    }
  }
  return true;
}
bool String::operator!=(String const& other) const {
  return not(other == *this);
}

bool String::operator<(String const& other) const {
  for (size_t i = 0; i < std::min(size_, other.size_); ++i) {
    if (str_[i] < other.str_[i]) {
      return true;
    }
  }
  return size_ < other.size_;
}
bool String::operator>(String const& other) const {
  for (size_t i = 0; i < std::min(size_, other.size_); ++i) {
    if (str_[i] > other.str_[i]) {
      return true;
    }
  }
  return size_ > other.size_;
}

bool String::operator<=(String const& other) const {
  for (size_t i = 0; i < std::min(size_, other.size_); ++i) {
    if (str_[i] < other.str_[i]) {
      return true;
    }
    if (str_[i] > other.str_[i]) {
      return false;
    }
  }
  return size_ <= other.size_;
}
bool String::operator>=(String const& other) const {
  for (size_t i = 0; i < std::min(size_, other.size_); ++i) {
    if (str_[i] > other.str_[i]) {
      return true;
    }
    if (str_[i] < other.str_[i]) {
      return false;
    }
  }
  return size_ >= other.size_;
}

std::vector<String> String::Split(String const& delim) const {
  std::vector<String> res;
  size_t to_compare = 0;
  String buff;

  for (size_t i = 0; i < size_; i++) {
    if (str_[i] == delim[to_compare]) {
      to_compare += 1;
    } else {
      to_compare = 0;
    }
    buff.PushBack(str_[i]);
    if (to_compare == delim.Size()) {
      buff.Resize(buff.Size() - delim.Size());
      res.push_back(buff);
      buff.Clear();
      to_compare = 0;
    }
  }
  res.push_back(buff);
  return res;
}

String String::Join(std::vector<String> const& strings) const {
  if (strings.empty()) {
    return "";
  }

  String res = strings[0];
  for (size_t i = 1; i < strings.size(); ++i) {
    res += *this;
    res += strings[i];
  }
  return res;
}

std::istream& operator>>(std::istream& in, String& str) {
  str.Clear();
  char next = ' ';

  while (next == ' ') {
    in.get(next);
  }

  str.PushBack(next);
  while (in.get(next)) {
    if (next == ' ') {
      break;
    }
    str.PushBack(next);
  }
  return in;
}
std::ostream& operator<<(std::ostream& out, String const& str) {
  for (size_t i = 0; i < str.size_; ++i) {
    out << str.str_[i];
  }
  return out;
}

void String::PushBack(char character) {
  if (size_ == capacity_) {
    Reserve(capacity_ << 1);
  }
  str_[size_] = character;
  size_ += 1;
}
void String::PopBack() {
  if (size_ == 0) {
    return;
  }
  size_ -= 1;
}

void String::Clear() { size_ = 0; }
void String::Resize(size_t size) {
  Reserve(size);
  size_ = size;
  str_[size_] = '\0';
}
void String::Resize(size_t size, char character) {
  Reserve(size);
  for (size_t i = size_; i < size; ++i) {
    str_[i] = character;
  }
  size_ = size;
  str_[size_] = '\0';
}
void String::Reserve(size_t capacity) {
  if (capacity_ >= capacity) {
    return;
  }
  capacity_ = capacity;
  char* old_str = str_;
  str_ = static_cast<char*>(calloc(capacity_ + 1, sizeof(char)));
  memcpy(str_, old_str, sizeof(char) * size_);
  free(old_str);
}
void String::ShrinkToFit() {
  if (size_ == capacity_) {
    return;
  }
  char* old_str = str_;
  str_ = static_cast<char*>(calloc(size_ + 1, sizeof(char)));
  memcpy(str_, old_str, sizeof(char) * size_);
  free(old_str);
  capacity_ = size_;
  str_[size_] = '\0';
}
void String::Swap(String& other) {
  std::swap(str_, other.str_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}
