#ifndef MATRIX
#define MATRIX

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() = default;

  explicit Matrix(std::vector<std::vector<T>> matrix);
  explicit Matrix(const T& elem) { matrix_.fill(elem); }

  T& operator()(size_t height, size_t width) {
    return matrix_[height + width * N];
  };
  const T& operator()(size_t height, size_t width) const {
    return matrix_[height + width * N];
  }

  Matrix<M, N, T> Transposed();
  T Trace();

  bool operator==(Matrix<N, M, T> other);
  bool operator!=(Matrix<N, M, T> other) { return not operator==(other); }

  Matrix<N, M, T> operator+=(Matrix<N, M, T> other);
  Matrix<N, M, T> operator-=(Matrix<N, M, T> other);
  Matrix<N, M, T> operator*=(const T& mult);

  template <size_t K>
  Matrix<N, K, T> operator*(Matrix<M, K, T> other);

  Matrix<N, M, T> operator+(Matrix<N, M, T> other) {
    other += *this;
    return other;
  }
  Matrix<N, M, T> operator-(Matrix<N, M, T> other) {
    other -= *this;
    return other;
  }
  Matrix<N, M, T> operator*(const T& mult) {
    Matrix<N, M, T> copy = *this;
    copy *= mult;
    return copy;
  }

 private:
  std::array<T, N * M> matrix_{};
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(std::vector<std::vector<T>> matrix) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      operator()(i, j) = matrix[i][j];
    }
  }
}

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> transposed;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      transposed(j, i) = operator()(i, j);
    }
  }
  return transposed;
}

template <size_t N, size_t M, typename T>
T Matrix<N, M, T>::Trace() {
  static_assert(N == M);
  T res;
  for (size_t i = 0; i < N; ++i) {
    res += operator()(i, i);
  }
  return res;
}

template <size_t N, size_t M, typename T>
bool Matrix<N, M, T>::operator==(Matrix<N, M, T> other) {
  for (size_t i = 0; i < N * M; ++i) {
    if (matrix_[i] != other.matrix_[i]) {
      return false;
    }
  }
  return true;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator+=(Matrix<N, M, T> other) {
  for (size_t i = 0; i < N * M; ++i) {
    matrix_[i] += other.matrix_[i];
  }
  return *this;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator-=(Matrix<N, M, T> other) {
  for (size_t i = 0; i < N * M; ++i) {
    matrix_[i] -= other.matrix_[i];
  }
  return *this;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator*=(const T& mult) {
  for (size_t i = 0; i < N * M; ++i) {
    matrix_[i] *= mult;
  }
  return *this;
}

template <size_t N, size_t M, typename T>
template <size_t K>
Matrix<N, K, T> Matrix<N, M, T>::operator*(Matrix<M, K, T> other) {
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

#endif  // #ifndef MATRIX
