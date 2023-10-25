#include <cstddef>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity) {
  }

  size_t Size() const {
    return 0;
  }

  bool Empty() const {
    return false;
  }

  bool TryPush(int element) {
    return true;
  }

  bool TryPop(int* element) {
    return true;
  }
};
