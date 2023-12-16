#ifndef ABSTRACT_TOKEN_HPP
#define ABSTRACT_TOKEN_HPP

#include <sstream>
#include <string>

class AbstractToken {
 public:
  AbstractToken(const std::string& token) : token_(token) {}
  const std::string& GetStringToken() { return token_; }
  virtual ~AbstractToken() {}

 private:
  std::string token_;
};

#endif  // #ifndef ABSTRACT_TOKEN_HPP
