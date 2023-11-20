#ifndef MATRIX
#define MATRIX

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() {
    for (size_t i = 0; i < N * M; ++i) {
      matrix_[i] = T();
    }
  }
  Matrix(std::vector<std::vector<T>> matrix) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        operator()(i, j) = matrix[i][j];
      }
    }
  }
  Matrix(T const& elem) {
    for (size_t i = 0; i < N * M; ++i) {
      matrix_[i] = elem;
    }
  }

  T& operator()(size_t height, size_t width) {
    return matrix_[height + width * N];
  };
  T const& operator()(size_t height, size_t width) const {
    return matrix_[height + width * N];
  }

  Matrix<M, N, T> Transposed() {
    Matrix<M, N, T> transposed;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        transposed(j, i) = operator()(i, j);
      }
    }
    return transposed;
  }

  T Trace() {
    static_assert(N == M);
    T res;
    for (size_t i = 0; i < N; ++i) {
      res += operator()(i, i);
    }
    return res;
  }

  bool operator==(Matrix<N, M, T> other) {
    for (size_t i = 0; i < N * M; ++i) {
      if (matrix_[i] != other.matrix_[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(Matrix<N, M, T> other) { return not operator==(other); }

  Matrix<N, M, T> operator+=(Matrix<N, M, T> other) {
    for (size_t i = 0; i < N * M; ++i) {
      matrix_[i] += other.matrix_[i];
    }
    return *this;
  }
  Matrix<N, M, T> operator-=(Matrix<N, M, T> other) {
    for (size_t i = 0; i < N * M; ++i) {
      matrix_[i] -= other.matrix_[i];
    }
    return *this;
  }
  Matrix<N, M, T> operator*=(T const& mult) {
    for (size_t i = 0; i < N * M; ++i) {
      matrix_[i] *= mult;
    }
    return *this;
  }

  Matrix<N, M, T> operator+(Matrix<N, M, T> other) {
    other += *this;
    return other;
  }
  Matrix<N, M, T> operator-(Matrix<N, M, T> other) {
    other -= *this;
    return other;
  }
  Matrix<N, M, T> operator*(T const& mult) {
    Matrix<N, M, T> copy = *this;
    copy *= mult;
    return copy;
  }

  template <size_t K>
  Matrix<N, K, T> operator*(Matrix<M, K, T> other) {
    std::ignore = other;
    Matrix<N, K, T> res;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < K; ++j) {
        for (size_t k = 0; k < M; ++k) {
          res(i, j) += operator()(i, k) * other(k, j);
        }
      }
    }
    return res;
  }

 private:
  std::array<T, N * M> matrix_;
};

#endif  // #ifndef MATRIX
