#ifndef EXPR_IN_POLISH_NOTATION_HPP
#define EXPR_IN_POLISH_NOTATION_HPP

#include <sstream>
#include <stack>
#include <stdexcept>

#include "AbstractToken.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

struct InvalidExpr : public std::exception {
  const char* what() const noexcept override { return "Invalid expression!"; }
};

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& expr);
  std::stack<AbstractToken*> GetTokens() { return tokens_; }

 private:
  void ConvertInlineBrackets(std::stringstream& sstream);
  bool CheckBrackets(const std::string& expr);
  void ReadOperand(std::stringstream& sstream, int operands);
  void UnloadOperators(std::stack<char>& operators, int operands);
  void Clear();

  std::stack<AbstractToken*> tokens_;
};

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(const std::string& expr) {
  if (not CheckBrackets(expr)) {
    throw InvalidExpr();
  }
  std::stringstream sstream;
  sstream << expr;
  ConvertInlineBrackets(sstream);
}

template <typename T>
bool ExprInPolishNotation<T>::CheckBrackets(const std::string& expr) {
  int balance = 0;
  for (char chr : expr) {
    if (chr == '(') {
      balance += 1;
    } else if (chr == ')') {
      balance -= 1;
    }
    if (balance < 0) {
      return false;
    }
  }
  return balance == 0;
}

template <typename T>
void ExprInPolishNotation<T>::ConvertInlineBrackets(
    std::stringstream& sstream) {
  std::stack<char> operators;
  int operands = 0;
  char chr;
  while (sstream >> chr) {
    switch (chr) {
      case '-':
      case '+':
      case '/':
      case '*':
        operators.push(chr);
        break;
      case ')':
        return;
      case '(':
        ConvertInlineBrackets(sstream);
        UnloadOperators(operators, operands + 1);
        operands = 1;
        break;
      default:
        sstream.putback(chr);
        T operand;
        sstream >> operand;
        auto* token = new OperandToken<T>(operand);
        tokens_.push(token);
        UnloadOperators(operators, operands + 1);
        operands = 1;
        break;
    }
  }
}

template <typename T>
void ExprInPolishNotation<T>::UnloadOperators(std::stack<char>& operators,
                                              int operands) {
  if (operators.empty()) {
    if (operands != 1) {
      Clear();
      throw InvalidExpr();
    }
    return;
  }

  while (operators.size() > 1) {
    auto op_str = std::string(1, operators.top());
    tokens_.push(new UnaryOperatorToken<T>(op_str));
    operators.pop();
  }

  auto op_str = std::string(1, operators.top());
  if (operands == 2) {
    tokens_.push(new BinaryOperatorToken<T>(op_str));
  } else {
    tokens_.push(new UnaryOperatorToken<T>(op_str));
  }
  operators.pop();
}

template <typename T>
void ExprInPolishNotation<T>::Clear() {
  while (not tokens_.empty()) {
    delete tokens_.top();
    tokens_.pop();
  }
}

#endif  // #ifndef EXPR_IN_POLISH_NOTATION_HPP
