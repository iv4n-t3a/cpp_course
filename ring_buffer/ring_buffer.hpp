#include <cstddef>
#include <vector>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity) { buffer_.resize(capacity); }

  size_t Size() const { return size_; }
  bool Empty() const { return Size() == 0; }

  bool TryPush(int element) {
    if (IsOverflow()) {
      return false;
    }
    buffer_[end_] = element;
    end_ = NextIndex(end_);
    size_ += 1;
    return true;
  }

  bool TryPop(int* element) {
    if (Size() == 0) {
      return false;
    }
    *element = buffer_[begin_];
    begin_ = NextIndex(begin_);
    size_ -= 1;
    return true;
  }

 private:
  bool IsOverflow() const { return size_ == buffer_.size(); }
  size_t NextIndex(size_t index) { return (index + 1) % buffer_.size(); }

  size_t begin_ = 0;
  size_t end_ = 0;
  size_t size_ = 0;
  std::vector<int> buffer_;
};
