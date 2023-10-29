#include <cstdlib>

class String {
 public:
  String();
  String(size_t size, char character);
  String(char*);

  void Clear();

  void PushBack(char);
  void PopBack();
  void Resize(size_t);
  void Reserve(size_t);
  void ShrinkToFit();

  char* Data() { return str_; }

 private:
  char* str_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
};
