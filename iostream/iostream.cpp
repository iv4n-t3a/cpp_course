#include "iostream.hpp"

#include <unistd.h>

#include <cstddef>

#include "iostream.hpp"
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

stdlike::ostream stdlike::cout =
    stdlike::ostream(1, stdlike::kDefaultBufferSize);
stdlike::istream stdlike::cin =
    stdlike::istream(0, stdlike::kDefaultBufferSize, &stdlike::cout);

stdlike::istream::istream(int fd, std::size_t buff_cap,
                          stdlike::ostream *to_tie)
    : fd_(fd), buff_cap_(buff_cap), tied_(to_tie) {}

stdlike::istream::~istream() { delete[] buff_; }

stdlike::istream &stdlike::istream::operator>>(char &chr) {
  if (tied_ != nullptr) {
    tied_->flush();
  }
  if (bad()) {
    return *this;
  }
  failbit_ = false;

  skip_spaces();
  chr = get();
  return *this;
}

stdlike::istream &stdlike::istream::operator>>(std::string &str) {
  if (tied_ != nullptr) {
    tied_->flush();
  }
  if (bad()) {
    return *this;
  }
  failbit_ = false;

  std::vector<char> read;
  skip_spaces();
  char c = get();

  while (c != '\n' and c != ' ') {
    read.push_back(c);
    c = get();
  }

  str = read.data();

  return *this;
}

stdlike::istream &stdlike::istream::operator>>(bool &bln) {
  if (tied_ != nullptr) {
    tied_->flush();
  }
  if (bad()) {
    return *this;
  }
  failbit_ = false;

  skip_spaces();
  bln = get() != '0';
  skip_until_space();

  return *this;
}

stdlike::istream &stdlike::istream::operator>>(short &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(unsigned short &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(int &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(unsigned int &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(long int &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(unsigned long int &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(long long int &num) {
  return InputInt(num);
}
stdlike::istream &stdlike::istream::operator>>(unsigned long long int &num) {
  return InputInt(num);
}

stdlike::istream &stdlike::istream::operator>>(float &num) {
  return InputFloat(num);
}
stdlike::istream &stdlike::istream::operator>>(double &num) {
  return InputFloat(num);
}

template <typename T> stdlike::istream &stdlike::istream::InputInt(T &num) {
  if (tied_ != nullptr) {
    tied_->flush();
  }
  if (bad()) {
    return *this;
  }
  failbit_ = false;

  skip_spaces();
  num = 0;
  bool neg = false;

  if (peek() == '-') {
    get();
    neg = true;
  }

  if (peek() > '9' or peek() < '0') {
    failbit_ = true;
    return *this;
  }

  for (char c = get(); c <= '9' and c >= '0'; c = get()) {
    num *= 10;
    num += neg ? -(c - '0') : c - '0';
  }

  return *this;
}

template <typename T> stdlike::istream &stdlike::istream::InputFloat(T &num) {
  if (tied_ != nullptr) {
    tied_->flush();
  }
  if (bad()) {
    return *this;
  }
  failbit_ = false;

  skip_spaces();
  num = 0;
  bool neg = false;

  if (peek() == '-') {
    get();
    neg = true;
  }

  if (peek() != '.' and (peek() > '9' or peek() < '0')) {
    failbit_ = true;
    return *this;
  }

  char c = get();

  for (; c <= '9' and c >= '0'; c = get()) {
    num *= 10;
    num += c - '0';
  }

  if (c != '.') {
    num = neg ? -num : num;
    return *this;
  }

  T mult = 1;

  while (peek() <= '9' and peek() >= '0') {
    c = get();
    mult /= 10;
    num += mult * static_cast<T>(c - '0');
  }

  num = neg ? -num : num;

  return *this;
}

char stdlike::istream::get() {
  if (bad()) {
    return 0;
  }

  if (buff_read_ == buff_sz_) {
    readline();
    if (buff_sz_ == 0) {
      badbit_ = true;
      return 0;
    }
  }
  char res = buff_[buff_read_];
  buff_read_ += 1;
  is_peeked_ = false;
  return res;
}

char stdlike::istream::peek() {
  if (bad()) {
    return 0;
  }

  if (is_peeked_) {
    return peeked_;
  }

  if (buff_read_ == buff_sz_) {
    readline();
    if (buff_sz_ == 0) {
      badbit_ = true;
      return 0;
    }
  }
  peeked_ = buff_[buff_read_];
  is_peeked_ = true;
  return peeked_;
}

bool stdlike::istream::fail() const { return failbit_ or badbit_; }
bool stdlike::istream::bad() const { return badbit_; }

void stdlike::istream::tie(stdlike::ostream *ostream) { tied_ = ostream; }

void stdlike::istream::skip_spaces() {
  while (peek() == ' ' or peek() == '\n') {
    get();
  }
}
void stdlike::istream::skip_until_space() {
  while (peek() != ' ' or peek() != '\n') {
    get();
  }
}

void stdlike::istream::readline() {
  if (buff_ == nullptr) {
    init_buff();
  }

  if (tied_ != nullptr) {
    tied_->flush();
  }

  buff_read_ = 0;
  ssize_t res = read(fd_, buff_, buff_cap_);

  if (res == -1) {
    badbit_ = true;
    buff_sz_ = 0;
  } else {
    buff_sz_ = res;
  }
}

void stdlike::istream::init_buff() { buff_ = new char[buff_cap_]; }

stdlike::ostream::ostream(int fd, std::size_t buff_cap)
    : fd_(fd), buff_cap_(buff_cap) {}

stdlike::ostream::~ostream() {
  flush();
  delete[] buff_;
}

stdlike::ostream &stdlike::ostream::operator<<(const char *str) {
  if (bad()) {
    return *this;
  }

  for (std::size_t i = 0; str[i] != '\0'; i++) {
    put(str[i]);
  }
  return *this;
}

stdlike::ostream &stdlike::ostream::operator<<(const void *ptr) {
  union PtrRepr {
    const void *ptr;
    const uint8_t bytes[sizeof(void *)];
  };

  if (bad()) {
    return *this;
  }

  PtrRepr repr = {.ptr = ptr};

  put('0');
  put('x');

  for (size_t i = 0; i < sizeof(void *); ++i) {
    char digit1 = (repr.bytes[i] & 0xF0) >> 4;
    char digit2 = (repr.bytes[i] & 0x0F);
    put(digit1 <= 9 ? digit1 + '0' : digit1 + 'a' - 10);
    put(digit2 <= 9 ? digit2 + '0' : digit2 + 'a' - 10);
  }

  return *this;
}

stdlike::ostream &stdlike::ostream::operator<<(short num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(int num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(long int num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(unsigned short num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(unsigned int num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(unsigned long int num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(long long int num) {
  return PrintInt(num);
}
stdlike::ostream &stdlike::ostream::operator<<(unsigned long long int num) {
  return PrintInt(num);
}

stdlike::ostream &stdlike::ostream::operator<<(float num) {
  return PrintFloat(num);
}
stdlike::ostream &stdlike::ostream::operator<<(double num) {
  return PrintFloat(num);
}

template <typename T> stdlike::ostream &stdlike::ostream::PrintInt(T num) {
  if (bad()) {
    return *this;
  }

  if (num == 0) {
    put('0');
    return *this;
  }

  bool neg = num < 0;

  if (neg) {
    put('-');
  }

  size_t denum = neg ? -1 : 1;

  while (num / denum >= 10) {
    denum *= 10;
  }

  while (denum != 0) {
    put(num / denum % 10 + '0');
    denum /= 10;
  }

  return *this;
}

template <typename T> stdlike::ostream &stdlike::ostream::PrintFloat(T num) {
  if (bad()) {
    return *this;
  }

  if (num == std::numeric_limits<T>::quiet_NaN() or
      num == std::numeric_limits<T>::signaling_NaN()) {
    *this << "nan";
    return *this;
  }

  if (num < 0) {
    put('-');
    num = -num;
  }

  if (num == std::numeric_limits<T>::infinity()) {
    *this << "inf";
    return *this;
  }

  *this << static_cast<uint64_t>(num);

  if (precision_ == 0) {
    return *this;
  }

  put('.');

  for (int i = 0; i < precision_; ++i) {
    num *= 10;
    put(static_cast<uint64_t>(num) % 10 + '0');
  }

  return *this;
}

stdlike::ostream &stdlike::ostream::operator<<(char chr) {
  if (bad()) {
    return *this;
  }

  put(chr);
  return *this;
}

void stdlike::ostream::put(char chr) {
  if (bad()) {
    return;
  }

  if (buff_ == nullptr) {
    init_buff();
  }
  if (buff_used_ == buff_cap_) {
    flush();
  }

  buff_[buff_used_] = chr;
  buff_used_ += 1;
}

void stdlike::ostream::flush() {
  if (buff_ == nullptr) {
    return;
  }

  for (size_t i = buff_used_; i > 0;) {
    int res = write(fd_, buff_ + buff_used_ - i, i);

    if (res >= 0) {
      i -= res;
    } else {
      badbit_ = true;
      buff_used_ = 0;
      return;
    }
  }
  buff_used_ = 0;
}

bool stdlike::ostream::fail() const { return failbit_ or badbit_; }
bool stdlike::ostream::bad() const { return badbit_; }

void stdlike::ostream::init_buff() { buff_ = new char[buff_cap_]; }
