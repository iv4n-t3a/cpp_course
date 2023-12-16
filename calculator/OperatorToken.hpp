#ifndef OPERATOR_TOKEN
#define OPERATOR_TOKEN

#include <functional>
#include <unordered_map>

#include "AbstractToken.hpp"
#include "OperandToken.hpp"

template <typename T>
class OperatorToken : public AbstractToken {
 public:
  OperatorToken(const std::string& token) : AbstractToken(token) {}
  virtual bool IsBinary() = 0;
};

template <typename T>
class UnaryOperatorToken : public OperatorToken<T> {
 public:
  UnaryOperatorToken(const std::string& token) : OperatorToken<T>(token) {}
  bool IsBinary() override { return false; }

  OperandToken<T>* Calculate(OperandToken<T>* operand) {
    return new OperandToken<T>(
        functions[AbstractToken::GetStringToken()](operand->GetValue()));
  }

 private:
  static std::unordered_map<std::string, std::function<T(T)>> functions;
};

template <typename T>
std::unordered_map<std::string, std::function<T(T)>>
    UnaryOperatorToken<T>::functions = {
        {"-", [](T operand) { return -operand; }},
        {"+", [](T operand) { return operand; }},
};

template <typename T>
class BinaryOperatorToken : public OperatorToken<T> {
 public:
  BinaryOperatorToken(const std::string& token) : OperatorToken<T>(token) {}
  bool IsBinary() override { return true; }

  OperandToken<T>* Calculate(OperandToken<T>* lhs, OperandToken<T>* rhs) {
    return new OperandToken<T>(functions[AbstractToken::GetStringToken()](
        lhs->GetValue(), rhs->GetValue()));
  }

 private:
  static std::unordered_map<std::string, std::function<T(T, T)>> functions;
};

template <typename T>
std::unordered_map<std::string, std::function<T(T, T)>>
    BinaryOperatorToken<T>::functions = {
        {"+", [](T lhs, T rhs) { return lhs + rhs; }},
        {"-", [](T lhs, T rhs) { return lhs - rhs; }},
        {"*", [](T lhs, T rhs) { return lhs * rhs; }},
        {"/", [](T lhs, T rhs) { return lhs / rhs; }},
};

#endif  // #ifndef OPERATOR_TOKEN
