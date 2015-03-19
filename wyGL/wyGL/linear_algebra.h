#ifndef linear_algebra_h__
#define linear_algebra_h__
#include <assert.h>
#include <math.h>
#include <string.h>
namespace wyGL
{
    template<typename Scalar, int DIM>
    class Matrix
    {
        //row major 
    public:
        Matrix() 
        {
            memset(data, 0, sizeof(data));
        }

        Matrix(const Scalar _data[DIM][DIM])
        {
            for(int i = 0; i < DIM; i++)
            {
                for (int j = 0; j < DIM; j++)
                {
                    data[i][j] = _data[i][j];
                }
            }
        }

        static int dimension() 
        {
            return DIM;
        }

        // read and write data
        Scalar& operator() (unsigned i,unsigned j)
        {
            return data[i][j];
        }

        //// only read data
        const Scalar& operator() (unsigned i,unsigned j) const
        {
            return data[i][j];
        }

        // this * _m
        Matrix rightMultiplyMatrix( Matrix& _m) const
        {
            Matrix result;
            for (int i = 0; i < DIM; i++)
            {
                for (int j = 0; j < DIM; j++)
                {
                    float temp = 0;
                    for (int k = 0;k < DIM; k++)
                    {
                        temp += data[i][k] * _m(k,j);
                    }
                    result(i,j) = temp;
                }
            }
            return result;
        }

        // _m * this
        Matrix leftMultiplyMatrix(const Matrix& _m) const
        {
            assert(false);
            // to add implementation
        }

        Matrix transpose()
        {
            float temp[DIM][DIM] = {0};
            for(int i = 0; i < DIM; i++)
            {
                for (int j = 0; j < DIM; j++)
                {
                    temp[i][j] = data[j][i];
                }
            }
            return Matrix(temp);
        }


        Matrix inverse()
        {
            assert(false);
            // to add implementation
        }

    private:
        //  row major.
        //  when DIM = 4:
        //	a00,  a01,  a02,  a03
        //	a10,  a11,  a12,  a13
        //	a20,  a21,  a22,  a23
        //	a30,  a31,  a32,  a33
        Scalar data[DIM][DIM];

    };


    template<typename Scalar,int DIM>
    class Vector
    {
    public:
        typedef Scalar value_type;
        Vector()
        {
            memset(data, 0, sizeof(data) );
        }

        Vector(const Scalar a, const Scalar b)
        {
            assert(2 == DIM);
            data[0] = a;
            data[1] = b;
        }

        Vector(const Scalar a, const Scalar b,const Scalar c)
        {
            assert(3 == DIM);
            data[0] = a;
            data[1] = b;
            data[2] = c;
        }

        Vector(const Scalar a, const Scalar b,const Scalar c, const Scalar d)
        {
            assert(4 == DIM);
            data[0] = a;
            data[1] = b;
            data[2] = c;
            data[3] = d;
        }

        const Scalar& operator[](unsigned int i) const
        {
            assert(i >= 0 && i < DIM);
            return data[i];
        }

        Scalar& operator[](unsigned int i)
        {
            assert(i >= 0 && i < DIM);
            return data[i];
        }

        Vector operator+(const Vector& _v) const
        {
            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                result[i] = data[i] + _v[i];
            }
            return result;
        }

        Vector operator-(const Vector& _v) const
        {
            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                result[i] = data[i] - _v[i];
            }
            return result;
        }

        Vector operator*(const Scalar _s)
        {
            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                result[i] = data[i] * _s;
            }
            return result;
        }

        Vector operator*(const Vector _v)
        {
            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                result[i] = data[i] * _v[i];
            }
            return result;
        }

        Vector operator / (const Scalar _s)
        {

            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                result[i] = data[i] / _s;
            }
            return result;
        }

        Scalar dotProduct(const Vector& _v) const
        {
            Scalar result = 0;
            for (int i = 0; i < DIM; i++)
            {
                result += data[i] * _v[i];
            }
            return result;
        }

        Vector crossProduct(const Vector& _v) const
        {
            assert(3 == DIM);
            return Vector(data[1] * _v[2] - data[2] * _v[1],
                data[2] * _v[0] - data[0] * _v[2],
                data[0] * _v[1] - data[1] * _v[0]);
        }

        Scalar squareNorm() const
        {
            Scalar value = 0;
            for (int i = 0; i < DIM; i++)
            {
                value = data[i] * data[i] + value;
            }
            return value;
        }

        Scalar norm() const
        {
            return sqrt(squareNorm());
        }

        void normalize()
        {
            Scalar n = norm();
            for (int i = 0; i < DIM; i++)
            {
                data[i] /= n;
            }
        }

        // _m * this
        Vector leftMatrixMultiply(const Matrix<Scalar, DIM>& _m) const
        {
            Vector result;
            for (int i = 0; i < DIM; i++)
            {
                for (int j = 0; j < DIM; j++)
                {
                    result[i] += _m(i,j) * data[j];
                }
            }
            return result;
        }

        //  this * _m
        Vector rightMatrixMultiply(const Matrix<Scalar,DIM>& _m) const
        {
            assert(false);
            // to add implementation
        }

        void maximize(const Vector& _v)
        {
            for (int i = 0; i < DIM; i++)
            {
                if (data[i] < _v[i])
                {
                    data[i] = _v[i];
                }
            }
        }

        void minimize(const Vector& _v)
        {
            for (int i = 0; i < DIM; i++)
            {
                if (data[i] > _v[i])
                {
                    data[i] = _v[i];
                }
            }
        }

        Scalar maxBit()
        {
            Scalar temp = data[0];
            for (int i = 1; i < DIM; i++)
            {
                if (temp < data[i])
                {
                    temp = data[i];
                }
            }
            return temp;
        }

    private:
        Scalar data[DIM];

    };

    typedef Matrix<double,3> matrix3d;
    typedef Matrix<double,4> matrix4d;
    typedef Matrix<float,3>  matrix3f;
    typedef Matrix<float,4>  matrix4f;

    typedef Vector<float,2>  vec2f;
    typedef Vector<float,3>  vec3f;
    typedef Vector<float,4>  vec4f;

    typedef Vector<double,2> vec2d;
    typedef Vector<double,3> vec3d;
    typedef Vector<double,4> vec4d;

    typedef Vector<int,2>    vec2i;
    typedef Vector<int,3>    vec3i;
    typedef Vector<int,4>    vec4i;
}
#endif // linear_algebra_h__