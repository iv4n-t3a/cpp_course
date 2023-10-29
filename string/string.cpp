#include "string.hpp"

#include <cstdlib>
#include <cstring>
#include <utility>

String::String() {
  str_ = static_cast<char*>(calloc(1, sizeof(char)));
  capacity_ = 1;
}
String::String(size_t size, char character) {
  size_ = size;
  capacity_ = size << 1;
  str_ = static_cast<char*>(calloc(capacity_, sizeof(char)));
  for (size_t i = 0; i < size; ++i) {
    str_[i] = character;
  }
}
String::String(char* str) {
  size_ = strlen(str);
  capacity_ = size_ << 1;
  str_ = static_cast<char*>(calloc(capacity_, sizeof(char)));
  memcpy(str_, str, sizeof(char) * size_);
}
String::String(String& other) {
  if (size_ < other.size_) {
    Resize(other.size_);
  }
  memcpy(str_, other.str_, other.size_);
}
String::~String() {
  free(str_);
}
String& String::operator=(String& other) {
  if (size_ < other.size_) {
    Resize(other.size_);
  }
  memcpy(str_, other.str_, other.size_);
  return *this;
}

void String::Clear() { size_ = 0; }

void String::PushBack(char character) {
  if (size_ == capacity_) {
    Reserve(size_ << 1);
  }
  str_[size_] = character;
  size_ += 1;
}
void String::PopBack() { size_ -= 1; }
void String::Resize(size_t size) {
  Reserve(size);
  size_ = size;
}
void String::Reserve(size_t capacity) {
  capacity_ = capacity;
  char* old_str = str_;
  str_ = static_cast<char*>(calloc(capacity_, sizeof(char)));
  memcpy(str_, old_str, sizeof(char) * size_);
  free(old_str);
}
void String::ShrinkToFit() {
  char* old_str = str_;
  str_ = static_cast<char*>(calloc(size_, sizeof(char)));
  memcpy(str_, old_str, sizeof(char) * size_);
  free(old_str);
  capacity_ = size_;
}

void String::Swap(String& other) {
  std::swap(str_, other.str_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}


String String::operator+(String other) {
  String buf = *this;
  buf += other;
  return buf;
}
String String::operator+=(String other) {
  if (other.size_ + size_ > capacity_) {
    Reserve((other.size_ + size_) << 1);
  }
  memcpy(str_ + other.size_, other.str_, other.size_ * sizeof(char));
  return *this;
}

String String::operator*(size_t mult) {

}
String String::operator*=(size_t mult) {

}
