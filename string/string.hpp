#include <cstdlib>
#include <iostream>
#include <vector>

class String {
 public:
  String();
  String(size_t size, char character);
  String(char const*);
  String(String const&);
  String& operator=(String const&);
  ~String();

  char& operator[](size_t idx) { return *(str_ + idx); }
  char operator[](size_t idx) const { return str_[idx]; }
  char& Front() { return str_[0]; }
  char Front() const { return str_[0]; }
  char& Back() { return str_[size_ - 1]; }
  char Back() const { return str_[size_ - 1]; }

  bool Empty() const { return size_ == 0; }
  size_t Size() const { return size_; }
  size_t Capacity() const { return capacity_; }
  char* Data() { return str_; }
  char const* Data() const { return str_; }

  String operator+(String const&) const;
  String& operator+=(String const&);

  String operator*(size_t) const;
  String& operator*=(size_t);

  bool operator<(String const&) const;
  bool operator>(String const&) const;
  bool operator<=(String const&) const;
  bool operator>=(String const&) const;
  bool operator==(String const&) const;
  bool operator!=(String const&) const;

  std::vector<String> Split(String const& = " ") const;
  String Join(std::vector<String> const& strings) const;

  friend std::istream& operator>>(std::istream&, String&);
  friend std::ostream& operator<<(std::ostream&, String const&);

  void PushBack(char);
  void PopBack();

  void Clear();
  void Resize(size_t);
  void Resize(size_t, char);
  void Reserve(size_t);
  void ShrinkToFit();
  void Swap(String&);

 private:
  char* str_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
};

std::istream& operator>>(std::istream&, String&);
std::ostream& operator<<(std::ostream&, String const&);
