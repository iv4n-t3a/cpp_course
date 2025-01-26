#ifndef IOSTREAM
#define IOSTREAM

#include <string>

namespace stdlike {

const std::size_t kDefaultBufferSize = 1'000;
const int kDefaultPrecision = 9;

class ostream;

class istream {
public:
  istream(int fd = 0, std::size_t buff_cap = kDefaultBufferSize,
          ostream *to_tie = nullptr);
  ~istream();

  istream &operator>>(char &num);
  istream &operator>>(std::string &);

  istream &operator>>(bool &num);

  istream &operator>>(short &);
  istream &operator>>(unsigned short &);
  istream &operator>>(int &);
  istream &operator>>(unsigned int &);
  istream &operator>>(long int &);
  istream &operator>>(unsigned long int &);
  istream &operator>>(long long int &);
  istream &operator>>(unsigned long long int &);

  istream &operator>>(float &num);
  istream &operator>>(double &num);

  char get();
  char peek();

  bool fail() const;
  bool bad() const;

  void tie(ostream *);

private:
  template <typename T> istream &InputInt(T &);

  template <typename T> istream &InputFloat(T &);

  void skip_spaces();
  void skip_until_space();
  void readline();
  void init_buff();

  int fd_ = 0;
  std::size_t buff_sz_ = 0;
  std::size_t buff_cap_ = 0;
  std::size_t buff_read_ = 0;
  bool is_peeked_ = false;
  char peeked_;
  char *buff_ = nullptr;
  bool failbit_ = false;
  bool badbit_ = false;
  ostream *tied_ = nullptr;
};

class ostream {
public:
  ostream(int fd = 1, std::size_t buff_cap = kDefaultBufferSize);
  ~ostream();

  ostream &operator<<(const char *);
  ostream &operator<<(const void *);

  ostream &operator<<(short);
  ostream &operator<<(unsigned short);
  ostream &operator<<(int);
  ostream &operator<<(unsigned int);
  ostream &operator<<(long int);
  ostream &operator<<(unsigned long int);
  ostream &operator<<(long long int);
  ostream &operator<<(unsigned long long int);

  ostream &operator<<(float);
  ostream &operator<<(double);

  ostream &operator<<(char);

  void put(char);
  void flush();

  bool fail() const;
  bool bad() const;

private:
  template <typename T> ostream &PrintInt(T);

  template <typename T> ostream &PrintFloat(T);

  void init_buff();

  int fd_ = 1;
  int precision_ = kDefaultPrecision;
  std::size_t buff_cap_ = kDefaultBufferSize;
  std::size_t buff_used_ = 0;
  char *buff_ = nullptr;
  bool failbit_ = false;
  bool badbit_ = false;
};

extern istream cin;
extern ostream cout;

} // namespace stdlike

#endif // #ifndef IOSTREAM
