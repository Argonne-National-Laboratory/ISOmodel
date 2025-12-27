#ifndef ISOMODEL_MATRIX_HPP
#define ISOMODEL_MATRIX_HPP

#include <vector>
#include <stdexcept>

namespace openstudio {

    /// Lightweight Matrix class replacing boost::numeric::ublas::matrix
    class Matrix {
    public:
        Matrix() = default;

        // Defaulted Rule of Five for optimal move semantics
        Matrix(const Matrix&) = default;
        Matrix(Matrix&&) noexcept = default;
        Matrix& operator=(const Matrix&) = default;
        Matrix& operator=(Matrix&&) noexcept = default;
        ~Matrix() = default;

        Matrix(size_t r, size_t c, double val = 0.0)
            : m_rows(r), m_cols(c), m_data(r* c, val) {
        }

        [[nodiscard]] size_t size1() const noexcept { return m_rows; }
        [[nodiscard]] size_t size2() const noexcept { return m_cols; }

        // Resize method that clears data (consistent with original behavior)
        void resize(size_t r, size_t c) {
            m_rows = r;
            m_cols = c;
            m_data.assign(r * c, 0.0);
        }

        double& operator()(size_t r, size_t c) noexcept {
            return m_data[r * m_cols + c];
        }

        const double& operator()(size_t r, size_t c) const noexcept {
            return m_data[r * m_cols + c];
        }

    private:
        size_t m_rows = 0;
        size_t m_cols = 0;
        std::vector<double> m_data;
    };

} // namespace openstudio
#endif // ISOMODEL_MATRIX_HPP