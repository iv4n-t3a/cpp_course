#include "rec_dir_it.hpp"

#include <alloca.h>
#include <cstdint>
#include <dirent.h>
#include <iostream>

std::string stdlike::directory_entry::path() const { return path_; }

bool stdlike::directory_entry::is_directory() const {
  return (stat_.st_mode & S_IFMT) == S_IFDIR;
}
bool stdlike::directory_entry::is_symlink() const {
  return (stat_.st_mode & S_IFMT) == S_IFLNK;
}
bool stdlike::directory_entry::is_regular_file() const {
  return (stat_.st_mode & S_IFMT) == S_IFREG;
}
bool stdlike::directory_entry::is_block_file() const {
  return (stat_.st_mode & S_IFMT) == S_IFBLK;
}
bool stdlike::directory_entry::is_character_file() const {
  return (stat_.st_mode & S_IFMT) == S_IFCHR;
}
bool stdlike::directory_entry::is_socket() const {
  return (stat_.st_mode & S_IFMT) == S_IFSOCK;
}
bool stdlike::directory_entry::is_fifo() const {
  return (stat_.st_mode & S_IFMT) == S_IFIFO;
}

size_t stdlike::directory_entry::file_size() const { return stat_.st_size; }
size_t stdlike::directory_entry::hard_link_count() const {
  return stat_.st_nlink;
}
time_t stdlike::directory_entry::last_write_time() const {
  return stat_.st_mtime;
}
mode_t stdlike::directory_entry::permissions() const { return stat_.st_mode; }

uintmax_t stdlike::directory_entry::owner_uid() const { return stat_.st_uid; }
uintmax_t stdlike::directory_entry::owner_gid() const { return stat_.st_gid; }

stdlike::recursive_directory_iterator::recursive_directory_iterator(
    const char *path)
    : stdlike::recursive_directory_iterator(path,
                                            stdlike::directory_options::none) {}

bool stdlike::directory_entry::can_read() const {
  return stat_.st_mode & S_IXUSR;
}

stdlike::recursive_directory_iterator::recursive_directory_iterator(
    const char *path, stdlike::directory_options opt)
    : opts_(opt) {
  if (path[0] == '\0') {
    return;
  }

  enter_directory(path);
}

stdlike::recursive_directory_iterator::~recursive_directory_iterator() {
  clear();
}

stdlike::recursive_directory_iterator &
stdlike::recursive_directory_iterator::operator++() {
  if (can_enter_entry()) {
    enter_directory(entry_.path_);
    return *this;
  }

  if (not dirs_.empty() and not move_to_next_direntry()) {
    up_to_unprocessed();
  }

  return *this;
}

void stdlike::recursive_directory_iterator::operator++(int) { operator++(); }

bool stdlike::recursive_directory_iterator::operator==(
    const stdlike::recursive_directory_iterator &other) const {
  return entry_.path_ == other.entry_.path_;
}
bool stdlike::recursive_directory_iterator::operator!=(
    const stdlike::recursive_directory_iterator &other) const {
  return not operator==(other);
}

stdlike::recursive_directory_iterator::value_type
stdlike::recursive_directory_iterator::operator*() {
  return entry_;
}
stdlike::recursive_directory_iterator::pointer
stdlike::recursive_directory_iterator::operator->() {
  return &entry_;
}

size_t stdlike::recursive_directory_iterator::depth() const {
  return dirs_.size() - 1;
}

void stdlike::recursive_directory_iterator::pop() {
  leave_directory();
  reopen_curdir();
}

stdlike::recursive_directory_iterator
stdlike::recursive_directory_iterator::begin() const {
  return stdlike::recursive_directory_iterator(path_.c_str(), opts_);
}
stdlike::recursive_directory_iterator
stdlike::recursive_directory_iterator::end() const {
  return stdlike::recursive_directory_iterator("", opts_);
}

bool stdlike::recursive_directory_iterator::can_enter_entry() {
  bool follow_symlink =
      static_cast<int>(opts_) &
      static_cast<int>(directory_options::follow_directory_symlink);
  bool skip_permission_denied =
      static_cast<int>(opts_) &
      static_cast<int>(directory_options::skip_permission_denied);
  bool can_read = not skip_permission_denied or entry_.can_read();

  if (not can_read) {
    return false;
  }

  if (follow_symlink and entry_.is_symlink()) {
    struct stat entry_stat;
    stat(entry_.path_.c_str(), &entry_stat);
    return entry_stat.st_mode & S_IFDIR;
  }

  return entry_.is_directory();
}
void stdlike::recursive_directory_iterator::enter_directory(std::string path) {
  path_ = path;

  if (path_.back() != '/') {
    path_.push_back('/');
  }

  pushdir(path_);

  if (not move_to_next_direntry()) {
    leave_directory();
  }
}
void stdlike::recursive_directory_iterator::leave_directory() {
  if (dirs_.size() <= 1) {
    clear();
    return;
  }

  do {
    path_.pop_back();
  } while (not path_.empty() and path_.back() != '/');
  popdir();
}
bool stdlike::recursive_directory_iterator::move_to_next_direntry() {
  dirent *entry = nullptr;

  do {
    entry = readdir(dirs_.top());
    if (entry == nullptr) {
      return false;
    }
  } while (entry->d_name[0] == '.');

  entry_.path_ = path_ + entry->d_name;
  update_stat();

  return true;
}
void stdlike::recursive_directory_iterator::update_stat() {
  lstat(entry_.path_.c_str(), &entry_.stat_);
}
void stdlike::recursive_directory_iterator::up_to_unprocessed() {
  do {
    leave_directory();
  } while (not dirs_.empty() and not move_to_next_direntry());
}
void stdlike::recursive_directory_iterator::reopen_curdir() {
  popdir();
  pushdir(path_);
  move_to_next_direntry();
}
void stdlike::recursive_directory_iterator::clear() {
  while (not dirs_.empty()) {
    popdir();
  }
  path_ = "";
  entry_.path_ = "";
}

void stdlike::recursive_directory_iterator::pushdir(std::string path) {
  DIR *dir = opendir(path.c_str());
  dirs_.push(dir);
}
void stdlike::recursive_directory_iterator::popdir() {
  closedir(dirs_.top());
  dirs_.pop();
}

stdlike::recursive_directory_iterator
stdlike::begin(const stdlike::recursive_directory_iterator &iter) {
  return iter.begin();
}
stdlike::recursive_directory_iterator
stdlike::end(const stdlike::recursive_directory_iterator &iter) {
  return iter.end();
}
