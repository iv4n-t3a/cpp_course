#ifndef FILESYSTEM
#define FILESYSTEM

#include <dirent.h>
#include <sys/stat.h>

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <stack>
#include <string>

namespace stdlike {

enum class directory_options {
  none = 0,
  follow_directory_symlink = 1,
  skip_permission_denied = 2,
  all = 3
};

class directory_entry {
public:
  friend class recursive_directory_iterator;

  std::string path() const;

  bool is_directory() const;
  bool is_symlink() const;
  bool is_regular_file() const;
  bool is_block_file() const;
  bool is_character_file() const;
  bool is_socket() const;
  bool is_fifo() const;

  size_t file_size() const;
  size_t hard_link_count() const;
  time_t last_write_time() const;
  mode_t permissions() const;

  uintmax_t owner_uid() const;
  uintmax_t owner_gid() const;

  bool can_read() const;

private:
  directory_entry() = default;

  std::string path_;
  struct stat stat_;
};

class recursive_directory_iterator {
public:
  using value_type = directory_entry;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using iterator_category = std::input_iterator_tag;

  recursive_directory_iterator(const char *);
  recursive_directory_iterator(const char *, directory_options);
  recursive_directory_iterator(const recursive_directory_iterator &) = delete;
  ~recursive_directory_iterator();

  recursive_directory_iterator &operator++();
  void operator++(int);

  bool operator==(const recursive_directory_iterator &) const;
  bool operator!=(const recursive_directory_iterator &) const;

  value_type operator*();
  pointer operator->();

  size_t depth() const;
  void pop();

  recursive_directory_iterator begin() const;
  recursive_directory_iterator end() const;

private:
  bool can_enter_entry();
  void enter_directory(std::string path);
  void leave_directory();
  bool move_to_next_direntry();
  void update_stat();
  void up_to_unprocessed();
  void reopen_curdir();
  void clear();

  void pushdir(std::string path);
  void popdir();

  std::string path_;
  std::stack<DIR *> dirs_;
  directory_entry entry_;
  directory_options opts_;
};

recursive_directory_iterator begin(const recursive_directory_iterator &);
recursive_directory_iterator end(const recursive_directory_iterator &);

} // namespace stdlike

#endif // #ifndef FILESYSTEM
