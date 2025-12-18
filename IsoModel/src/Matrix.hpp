
// update to remove boost library dependence
//#ifndef ISOMODEL_MATRIX_HPP
//#define ISOMODEL_MATRIX_HPP
//
//#include <vector>
//#include <boost/numeric/ublas/matrix.hpp>
//
//namespace openstudio {
///// Matrix 
//typedef boost::numeric::ublas::matrix<double> Matrix;
//}
//#endif

#ifndef ISOMODEL_MATRIX_HPP
#define ISOMODEL_MATRIX_HPP

#include <vector>
#include <stdexcept>

namespace openstudio {

    /// Lightweight Matrix class to replace boost::numeric::ublas::matrix
    class Matrix {
    public:
        Matrix() : m_rows(0), m_cols(0) {}

        Matrix(size_t r, size_t c, double val = 0.0)
            : m_rows(r), m_cols(c), m_data(r* c, val) {
        }

        size_t size1() const { return m_rows; }
        size_t size2() const { return m_cols; }

        // Resize method that clears data (sufficient for this codebase's usage)
        void resize(size_t r, size_t c) {
            m_rows = r;
            m_cols = c;
            m_data.assign(r * c, 0.0);
        }

        double& operator()(size_t r, size_t c) {
            return m_data[r * m_cols + c];
        }

        const double& operator()(size_t r, size_t c) const {
            return m_data[r * m_cols + c];
        }

    private:
        size_t m_rows;
        size_t m_cols;
        std::vector<double> m_data;
    };

} // namespace openstudio
#endif // ISOMODEL_MATRIX_HPP