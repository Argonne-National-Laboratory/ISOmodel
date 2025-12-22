#ifndef ISOMODEL_MONTHLYMODELHELPERS_HPP
#define ISOMODEL_MONTHLYMODELHELPERS_HPP

#include "Vector.hpp"
#include "Matrix.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cfloat>

namespace openstudio {
    namespace isomodel {

        // Note: Ensure DEBUG_ISO_MODEL_SIMULATION is defined before including this, 
        // or passed as a template/argument. For now, we assume it's visible or we check indentation.
        // If you strictly want this to rely on the define from MonthlyModel.hpp, 
        // ensure MonthlyModel.hpp is included before this file in the .cpp.

#ifndef DEBUG_ISO_MODEL_SIMULATION
#define DEBUG_ISO_MODEL_SIMULATION false
#endif

// --- Printing Utilities ---

        inline void printVector(const char* vecName, const Vector& vec)
        {
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

        inline void printMatrix(const char* matName, const Matrix& mat)
        {
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << matName << "(" << mat.size1() << ", " << mat.size2() << "): " << std::endl << "\t";
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

        inline void vectorInit(Vector& vec, double val) {
            std::fill(vec.begin(), vec.end(), val);
        }

        inline void zero(Vector& vec) {
            vectorInit(vec, 0);
        }

        inline void one(Vector& vec) {
            vectorInit(vec, 1);
        }

        // --- Matrix Math ---

        inline Matrix prod(const Matrix& lop, const Matrix& rop) {
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

        inline Vector prod(const Matrix& m, const Vector& v) {
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

        inline Vector mult(const double* v1, const double s1, int size) {
            Vector vp(size);
            for (int i = 0; i < size; i++) vp[i] = v1[i] * s1;
            return vp;
        }

        inline Vector mult(const Vector& v1, const double s1) {
            Vector vp(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vp[i] = v1[i] * s1;
            return vp;
        }

        inline Vector mult(const Vector& v1, const double* v2) {
            Vector vp(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vp[i] = v1[i] * v2[i];
            return vp;
        }

        inline Vector mult(const Vector& v1, const Vector& v2) {
            Vector vp(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vp[i] = v1[i] * v2[i];
            return vp;
        }

        inline Vector div(const Vector& v1, const double s1) {
            Vector vp(v1.size());
            if (s1 == 0) {
                std::fill(vp.begin(), vp.end(), DBL_MAX);
            }
            else {
                for (size_t i = 0; i < v1.size(); i++) vp[i] = v1[i] / s1;
            }
            return vp;
        }

        inline Vector div(const double s1, const Vector& v1) {
            Vector vp(v1.size());
            for (size_t i = 0; i < v1.size(); i++) {
                vp[i] = (v1[i] == 0) ? DBL_MAX : (s1 / v1[i]);
            }
            return vp;
        }

        inline Vector div(const Vector& v1, const Vector& v2) {
            Vector vp(v1.size());
            for (size_t i = 0; i < v1.size(); i++) {
                vp[i] = (v2[i] == 0) ? DBL_MAX : (v1[i] / v2[i]);
            }
            return vp;
        }

        inline Vector sum(const Vector& v1, const Vector& v2) {
            Vector vs(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vs[i] = v1[i] + v2[i];
            return vs;
        }

        inline double sum(const Vector& v1) {
            double s = 0;
            for (double val : v1) s += val;
            return s;
        }

        inline Vector sum(const Vector& v1, const double v2) {
            Vector vs(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vs[i] = v1[i] + v2;
            return vs;
        }

        inline Vector dif(const Vector& v1, const Vector& v2) {
            Vector vd(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vd[i] = v1[i] - v2[i];
            return vd;
        }

        inline Vector dif(const Vector& v1, const double v2) {
            Vector vd(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vd[i] = v1[i] - v2;
            return vd;
        }

        inline Vector dif(const double v1, const Vector& v2) {
            Vector vd(v2.size());
            for (size_t i = 0; i < v2.size(); i++) vd[i] = v1 - v2[i];
            return vd;
        }

        inline double maximum(const Vector& v1) {
            double max_val = -DBL_MAX;
            for (double val : v1) if (val > max_val) max_val = val;
            return max_val;
        }

        inline Vector maximum(const Vector& v1, const Vector& v2) {
            Vector vx(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vx[i] = std::max(v1[i], v2[i]);
            return vx;
        }

        inline Vector maximum(const Vector& v1, double val) {
            Vector vx(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vx[i] = std::max(v1[i], val);
            return vx;
        }

        inline double minimum(const Vector& v1) {
            double min_val = DBL_MAX;
            for (double val : v1) if (val < min_val) min_val = val;
            return min_val;
        }

        inline Vector minimum(const Vector& v1, double val) {
            Vector vn(v1.size());
            for (size_t i = 0; i < v1.size(); i++) vn[i] = std::min(v1[i], val);
            return vn;
        }

        inline Vector abs(const Vector& v1) {
            Vector va(v1.size());
            for (size_t i = 0; i < v1.size(); i++) va[i] = std::fabs(v1[i]);
            return va;
        }

        inline Vector pow(const Vector& v1, const double xp) {
            Vector va(v1.size());
            for (size_t i = 0; i < v1.size(); i++) va[i] = std::pow(v1[i], xp);
            return va;
        }

    } // namespace isomodel
} // namespace openstudio

#endif // ISOMODEL_MONTHLYMODELHELPERS_HPP