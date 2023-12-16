#ifndef OPERAND_TOKEN
#define OPERAND_TOKEN

#include "AbstractToken.hpp"

template <typename T>
class OperandToken : public AbstractToken {
 public:
  OperandToken(const T& value)
      : value_(value), AbstractToken(std::to_string(value)) {}

  const T& GetValue() { return value_; }

 private:
  T value_;
};

#endif  // #ifndef OPERAND_TOKEN
