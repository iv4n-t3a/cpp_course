#include <cstdlib>

class String {
  public:
    String();
    String(size_t size, char character);
    String(char*);

    String(String&);
    ~String();
    String& operator=(String&);


    void Clear();
    void PushBack(char);
    void PopBack();
    void Resize(size_t);
    void Reserve(size_t);
    void ShrinkToFit();
    void Swap(String&);

    char& operator[](size_t idx) { return str_[idx]; }
    char operator[](size_t idx) const { return str_[idx]; }
    char& Front() { return str_[0]; }
    char& Back() { return str_[size_ - 1]; }
    char Front() const { return str_[0]; }
    char Back() const { return str_[size_ - 1]; }

    bool Empty() const { return size_ == 0; }
    size_t Size() const { return size_; }
    size_t Capacity() const { return capacity_; }
    char* Data() const { return str_; }

    String operator+(String);
    String operator+=(String);

    String operator*(size_t);
    String operator*=(size_t);

  private:
    char* str_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
