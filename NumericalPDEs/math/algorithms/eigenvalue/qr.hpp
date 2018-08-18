//
//  qr.hpp
//  NumericalPDEs
//
//  Created by Carlos David on 17/06/2018.
//  Copyright © 2018 cdalvaro. All rights reserved.
//

#pragma once

#include "../factorization/lu.hpp"
#include "../../containers.hpp"
#include "../../math.hpp"


#define CDA_QR_DEFAULT_ACCURACY 1E-06
#define CDA_QR_DEFAULT_MAX_ITERATIONS 1E+06


namespace cda {
    namespace math {
        namespace algorithms {
            namespace eigenvalue {
                
                template <typename T>
                class QR {
                public:
                    
                    QR(const containers::Matrix<T> &matrix,
                       const double &accuracy = CDA_QR_DEFAULT_ACCURACY,
                       const size_t &max_iterations = CDA_QR_DEFAULT_MAX_ITERATIONS) :
                    max_iterations(max_iterations), accuracy(accuracy) {
                        
                        if (!matrix.IsSquared()) {
                            throw std::logic_error("Matrix must be squared to compute its eigenvalues.");
                        }
                        
                        original = matrix;
                        rows = matrix.Rows();
                    }
                    
                    virtual ~QR() = default;
                    
                    const containers::Matrix<T> &Q() {
                        if (q.IsNull()) {
                            ComputeQR(original);
                        }
                        return q;
                    }
                    
                    const containers::Matrix<T> &R() {
                        if (r.IsNull()) {
                            ComputeQR(original);
                        }
                        return r;
                    }
                    
                    const size_t &MaxIterations() const {
                        return max_iterations;
                    }
                    
                    void MaxIterations(const size_t &max_iterations) {
                        this->max_iterations = max_iterations;
                    }
                    
                    const double &Accuracy() const {
                        return accuracy;
                    }
                    
                    void Accuracy(const double &accuracy) {
                        this->accuracy = accuracy;
                    }
                    
                    containers::Vector<T> EigenValues() {
                        
                        auto matrix(original);
                        double squared_sum = 0;
                        T element = 0;
                        
                        for (size_t k = 0; k < max_iterations; ++k) {
                            
                            ComputeQR(matrix);
                            matrix = r * q;
                            
                            // Convergence test
                            for (size_t column = 0; column < rows - 1; ++column) {
                                for (size_t row = column + 1; row < rows; ++row) {
                                    element = matrix[row][column];
                                    squared_sum += element * element;
                                }
                            }
                            
                            if (std::sqrt(squared_sum) < accuracy) {
                                break;
                            }
                        }
                        
                        return matrix.GetDiagonal();
                    }
                    
//                    Matrix<T> eigenVectors(int maxIte, unsigned char opt);                    //  Calcula los autovectores a partir de los autovalores (QR).
//                    Matrix<T> eigenVectors(unsigned char opt);                                //  Calcula los autovectores a partir de los autovaleres (QR).
//                    Matrix<T> eigenVectors();                                                 //  Calcula los autovectores a partir de los autovalores (QR).
//                    Vector<T> eigenVector(T eigenValue, int maxIte, unsigned char opt);       //  Calcula el autovector correspondiente al autovector introducido.
//                    Vector<T> eigenVector(T eigenValue, unsigned char opt);                   //  Calcula el autovector correspondiente al autovector introducido.
//                    Vector<T> eigenVector(T eigenValue);                                      //  Calcula el autovector correspondiente al autovector introducido.
                    
                    containers::Vector<T> EigenVector(const T &eigenValue) const {
                        
                        int k;
                        T eVa, maxS = 0.0, fact = 1E-07;
                        
                        containers::Matrix<T> eVe(rows, 1);
                        
                        eVe.Ones();
                        
                        eVa = eigenValue+1.0;
                        k=1;
                        auto Ainv = original - eigenValue * (1.0 + fact) * containers::Matrix<T>::Identity(rows, rows);
                        Ainv = algorithms::factorization::LU<T>::InverseMatrix(Ainv);
                        
                        while ((eVa < eigenValue*(1.0-fact) || eVa > eigenValue*(1.0+fact)) && k < max_iterations) {
                            eVe = Ainv * eVe;
                            maxS = eVe.AbsoluteMaximumElementWithSign();
                            eVa = 1/maxS + eigenValue*(1.0+fact);
                            eVe /= maxS;
                            k++;
                            
                            if ((eVa < eigenValue*(1.0-fact) || eVa > eigenValue*(1.0+fact)) && k == max_iterations) {
                                eVe.Ones();
                                eVe[0][0] *= 2.0;
                                eVa = eigenValue+1.0;
                                k=1;
//                                Ainv = original - eigenValue*(1.0+fact)*containers::Matrix<T>::Identity(rows, rows);
//                                Ainv = algorithms::factorization::LU<T>::InverseMatrix(Ainv);
                                
                                while (1/eVa != eigenValue && k < max_iterations) {
                                    eVe = Ainv * eVe;
                                    maxS = eVe.AbsoluteMaximumElementWithSign();
                                    eVa = 1/maxS + eigenValue*(1.0+fact);
                                    eVe /= maxS;
                                    k++;
                                }
                            }
                        }
                        
                        return eVe.GetColumnAsVector(0);
                    }
                    
                private:
                    
                    containers::Matrix<T> original;
                    containers::Matrix<T> q;
                    containers::Matrix<T> r;
                    
                    size_t rows;
                    size_t max_iterations;
                    double accuracy;
                    
                    void ComputeQR(const containers::Matrix<T> &matrix) {
                        
                        const auto I = containers::Matrix<T>::Identity(rows, rows);
                        
                        //  First column
                        auto c = matrix.GetColumnAsVector(0);
                        auto vt = c + I.GetRowAsVector(0) * signum(c[0]) * c.Norm();
                        
                        q = I - 2.0 * (containers::Transpose(vt) * vt) / vt.SquaredNorm();
                        r = q * matrix;
                        for (auto it_r = r.Begin() + rows; it_r != r.End(); it_r += rows) {
                            *it_r = 0;
                        }
                        
                        //  Other columns
                        containers::Matrix<T> h;
                        const auto last_row = rows - 1;
                        
                        for (size_t row = 1; row < last_row; ++row) {
                            c = r.GetColumnAsVector(row).Get(row);
                            
                            if (c.IsNull()) {
                                h = containers::Matrix<T>::Identity(rows - row, rows - row);
                            } else {
                                vt = c + I.GetRowAsVector(row, row) * signum(c[0]) * c.Norm();
                                h = I.GetMatrix(row, row) - 2.0 * (containers::Transpose(vt) * vt) / vt.SquaredNorm();
                                
                                auto A(I);
                                A.SetMatrix(row, row, h);
                                h = std::move(A);
                            }
                            
                            q = h * q;
                            r = h * r;
                            
                            for (size_t column = 0; column < row; ++column) {
                                r[row][column] = 0;
                            }
                        }
                        
                        q = std::move(q.Transpose());
                        
                        for (size_t row = 1; row < rows; ++row) {
                            for (size_t column = 0; column < row; ++column) {
                                r[row][column] = 0;
                            }
                        }
                        
                    }
                    
                };
                
            } /* namespace eigenvalue */
        } /* namespace algorithms */
    } /* namespace math */
} /* namespace cda */
