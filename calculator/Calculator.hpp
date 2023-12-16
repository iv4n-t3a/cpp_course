#ifndef CALCULATOR
#define CALCULATOR

#include "ExprInPolishNotation.hpp"

template <typename T>
class Calculator {
 public:
  static T CalculateExpr(const std::string& strexpr);

 private:
  static void CalculateLastToken(std::stack<AbstractToken*>& tokens);
  static AbstractToken* ExtractLastToken(std::stack<AbstractToken*>& tokens);
};

template <typename T>
T Calculator<T>::CalculateExpr(const std::string& strexpr) {
  ExprInPolishNotation<T> polexpr(strexpr);

  std::stack<AbstractToken*> tokens;
  try {
    tokens = polexpr.GetTokens();
  } catch (InvalidExpr exc) {
    throw exc;
  }

  while (tokens.size() != 1) {
    CalculateLastToken(tokens);
  }

  auto* last_token = static_cast<OperandToken<T>*>(tokens.top());
  T res = last_token->GetValue();
  delete last_token;
  return res;
}

template <typename T>
void Calculator<T>::CalculateLastToken(std::stack<AbstractToken*>& tokens) {
  auto* token = tokens.top();
  tokens.pop();
  if (typeid(*token) == typeid(UnaryOperatorToken<T>)) {
    auto* opr = static_cast<UnaryOperatorToken<T>*>(token);
    auto* operand = static_cast<OperandToken<T>*>(ExtractLastToken(tokens));

    tokens.push(opr->Calculate(operand));

    delete opr;
    delete operand;
  } else if (typeid(*token) == typeid(BinaryOperatorToken<T>)) {
    auto* opr = static_cast<BinaryOperatorToken<T>*>(token);
    auto* lhs = static_cast<OperandToken<T>*>(ExtractLastToken(tokens));
    auto* rhs = static_cast<OperandToken<T>*>(ExtractLastToken(tokens));

    tokens.push(opr->Calculate(rhs, lhs));

    delete opr;
    delete lhs;
    delete rhs;
  } else if (typeid(*token) == typeid(OperandToken<T>)) {
    tokens.push(token);
  }
}

template <typename T>
AbstractToken* Calculator<T>::ExtractLastToken(
    std::stack<AbstractToken*>& tokens) {
  CalculateLastToken(tokens);
  AbstractToken* res = tokens.top();
  tokens.pop();
  return res;
}

#endif  // #ifndef CALCULATOR
