#ifndef ISOMODEL_MATHHELPERS_HPP
#define ISOMODEL_MATHHELPERS_HPP

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Constants.hpp"

namespace openstudio {

// ==========================================
// From Vector.hpp
// ==========================================

/// Standard vector typedef to replace boost::numeric::ublas::vector
using Vector = std::vector<double>;

// ==========================================
// From Matrix.hpp
// ==========================================

/// Lightweight Matrix class replacing boost::numeric::ublas::matrix
class Matrix {
public:
  Matrix() = default;

  // Defaulted Rule of Five for optimal move semantics
  Matrix(const Matrix &) = default;
  Matrix(Matrix &&) noexcept = default;
  Matrix &operator=(const Matrix &) = default;
  Matrix &operator=(Matrix &&) noexcept = default;
  ~Matrix() = default;

  Matrix(size_t r, size_t c, double val = 0.0)
      : m_rows(r), m_cols(c), m_data(r * c, val) {}

  [[nodiscard]] size_t size1() const noexcept { return m_rows; }
  [[nodiscard]] size_t size2() const noexcept { return m_cols; }

  // Resize method that clears data (consistent with original behavior)
  void resize(size_t r, size_t c) {
    m_rows = r;
    m_cols = c;
    m_data.assign(r * c, 0.0);
  }

  double &operator()(size_t r, size_t c) noexcept {
    return m_data[r * m_cols + c];
  }

  const double &operator()(size_t r, size_t c) const noexcept {
    return m_data[r * m_cols + c];
  }

private:
  size_t m_rows = 0;
  size_t m_cols = 0;
  std::vector<double> m_data;
};

// ==========================================
// Other Math Helpers (originally MathHelpers.hpp)
// ==========================================
} // namespace openstudio
namespace openstudio::isomodel {

// Note: Ensure DEBUG_ISO_MODEL_SIMULATION is defined before including this,
// or passed as a template/argument.
#ifndef DEBUG_ISO_MODEL_SIMULATION
#define DEBUG_ISO_MODEL_SIMULATION false
#endif

// --- Printing Utilities ---

inline void printVector(const char *vecName, const Vector &vec) noexcept {
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << vecName << "(" << vec.size() << ") = [";
    if (vec.size() > 0) {
      std::cout << vec[0];
      for (unsigned int i = 1; i < vec.size(); i++) {
        std::cout << ", " << vec[i];
      }
    }
    std::cout << "]" << std::endl;
  }
}

inline void printMatrix(const char *matName, const Matrix &mat) noexcept {
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << matName << "(" << mat.size1() << ", " << mat.size2()
              << "): " << std::endl
              << "\t";
    for (unsigned int j = 0; j < mat.size2(); j++) {
      std::cout << "," << j;
    }
    std::cout << std::endl;
    for (unsigned int i = 0; i < mat.size1(); i++) {
      std::cout << "\t" << i;
      for (unsigned int j = 0; j < mat.size2(); j++) {
        std::cout << "," << mat(i, j);
      }
      std::cout << std::endl;
    }
  }
}

// --- Vector/Matrix Initialization ---

inline void vectorInit(Vector &vec, double val) noexcept {
  std::fill(vec.begin(), vec.end(), val);
}

inline void zero(Vector &vec) noexcept { vectorInit(vec, 0); }

inline void one(Vector &vec) noexcept { vectorInit(vec, 1); }

// --- Matrix Math ---

[[nodiscard]] inline Matrix prod(const Matrix &lop, const Matrix &rop) {
  if (lop.size2() != rop.size1()) {
    return Matrix(0, 0);
  }
  Matrix result(lop.size1(), rop.size2());
  for (size_t i = 0; i < result.size1(); ++i) {
    for (size_t j = 0; j < result.size2(); ++j) {
      double cellSum = 0.0;
      for (size_t k = 0; k < lop.size2(); ++k) {
        cellSum += lop(i, k) * rop(k, j);
      }
      result(i, j) = cellSum;
    }
  }
  return result;
}

[[nodiscard]] inline Vector prod(const Matrix &m, const Vector &v) {
  if (m.size2() != v.size()) {
    return Vector();
  }
  Vector result(m.size1(), 0.0);
  for (size_t i = 0; i < m.size1(); ++i) {
    double cellSum = 0.0;
    for (size_t j = 0; j < m.size2(); ++j) {
      cellSum += m(i, j) * v[j];
    }
    result[i] = cellSum;
  }
  return result;
}

// --- Scalar/Vector Math ---

[[nodiscard]] inline Vector mult(const double *v1, const double s1,
                                 int size) noexcept {
  Vector vp(size);
  for (int i = 0; i < size; i++)
    vp[i] = v1[i] * s1;
  return vp;
}

template <size_t N>
[[nodiscard]] inline Vector mult(const std::array<double, N> &v1,
                                 const double s1, int size) noexcept {
  return mult(v1.data(), s1, size);
}

[[nodiscard]] inline Vector mult(const Vector &v1, const double s1) noexcept {
  Vector vp(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vp[i] = v1[i] * s1;
  return vp;
}

[[nodiscard]] inline Vector mult(const Vector &v1, const double *v2) noexcept {
  Vector vp(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vp[i] = v1[i] * v2[i];
  return vp;
}

template <size_t N>
[[nodiscard]] inline Vector mult(const Vector &v1,
                                 const std::array<double, N> &v2) noexcept {
  return mult(v1, v2.data());
}
[[nodiscard]] inline Vector mult(const Vector &v1, const Vector &v2) noexcept {
  Vector vp(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vp[i] = v1[i] * v2[i];
  return vp;
}

[[nodiscard]] inline Vector div(const Vector &v1, const double s1) noexcept {
  Vector vp(v1.size());
  if (s1 == 0) {
    std::fill(vp.begin(), vp.end(), std::numeric_limits<double>::infinity());
  } else {
    for (size_t i = 0; i < v1.size(); i++)
      vp[i] = v1[i] / s1;
  }
  return vp;
}

[[nodiscard]] inline Vector div(const double s1, const Vector &v1) noexcept {
  Vector vp(v1.size());
  for (size_t i = 0; i < v1.size(); i++) {
    vp[i] = (std::fabs(v1[i]) < SAFE_EPSILON)
                ? std::numeric_limits<double>::infinity()
                : (s1 / v1[i]);
  }
  return vp;
}

[[nodiscard]] inline Vector div(const Vector &v1, const Vector &v2) noexcept {
  Vector vp(v1.size());
  for (size_t i = 0; i < v1.size(); i++) {
    vp[i] = (std::fabs(v2[i]) < SAFE_EPSILON)
                ? std::numeric_limits<double>::infinity()
                : (v1[i] / v2[i]);
  }
  return vp;
}

// Variadic sum for vectors (C++17 fold expression)
template <typename... Args>
[[nodiscard]] inline Vector sum(const Vector &v1, const Vector &v2, const Args&... args) noexcept {
  Vector vs(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vs[i] = v1[i] + v2[i] + (args[i] + ... + 0.0);
  return vs;
}

inline double sum(const Vector &v1) {
  double s = 0;
  for (double val : v1)
    s += val;
  return s;
}

[[nodiscard]] inline Vector sum(const Vector &v1, const double v2) noexcept {
  Vector vs(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vs[i] = v1[i] + v2;
  return vs;
}

[[nodiscard]] inline Vector dif(const Vector &v1, const Vector &v2) noexcept {
  Vector vd(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vd[i] = v1[i] - v2[i];
  return vd;
}

[[nodiscard]] inline Vector dif(const Vector &v1, const double v2) noexcept {
  Vector vd(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vd[i] = v1[i] - v2;
  return vd;
}

[[nodiscard]] inline Vector dif(const double v1, const Vector &v2) noexcept {
  Vector vd(v2.size());
  for (size_t i = 0; i < v2.size(); i++)
    vd[i] = v1 - v2[i];
  return vd;
}

[[nodiscard]] inline double maximum(const Vector &v1) noexcept {
  double max_val = std::numeric_limits<double>::lowest();
  for (double val : v1)
    if (val > max_val)
      max_val = val;
  return max_val;
}

[[nodiscard]] inline Vector maximum(const Vector &v1,
                                    const Vector &v2) noexcept {
  Vector vx(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vx[i] = std::max(v1[i], v2[i]);
  return vx;
}

[[nodiscard]] inline Vector maximum(const Vector &v1, double val) noexcept {
  Vector vx(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vx[i] = std::max(v1[i], val);
  return vx;
}

[[nodiscard]] inline double minimum(const Vector &v1) noexcept {
  double min_val = std::numeric_limits<double>::max();
  for (double val : v1)
    if (val < min_val)
      min_val = val;
  return min_val;
}

[[nodiscard]] inline Vector minimum(const Vector &v1, double val) noexcept {
  Vector vn(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    vn[i] = std::min(v1[i], val);
  return vn;
}

[[nodiscard]] inline Vector abs(const Vector &v1) noexcept {
  Vector va(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    va[i] = std::fabs(v1[i]);
  return va;
}

[[nodiscard]] inline Vector pow(const Vector &v1, const double xp) noexcept {
  Vector va(v1.size());
  for (size_t i = 0; i < v1.size(); i++)
    va[i] = std::pow(v1[i], xp);
  return va;
}

[[nodiscard]] inline double fastPow23(double x) noexcept {
  return std::cbrt(x * x);
}

[[nodiscard]] inline Matrix
toMatrix(const std::vector<std::vector<double>> &source, size_t rows,
         size_t cols) noexcept {
  Matrix mat(rows, cols);
  for (size_t r = 0; r < rows; ++r) {
    for (size_t c = 0; c < cols; ++c) {
      // Check bounds to be safe
      if (r < source.size() && c < source[r].size()) {
        mat(r, c) = source[r][c];
      } else {
        mat(r, c) = 0.0;
      }
    }
  }
  return mat;
}

} // namespace openstudio::isomodel

#endif // ISOMODEL_MATHHELPERS_HPP
