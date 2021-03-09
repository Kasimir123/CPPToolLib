#include "../stdafx/stdafx.h"

namespace geometry 
{
    // Value of PI
    #define PI 3.1415927f

    /// <summary>
    /// Vector 3 Class
    /// </summary>
    class vector3
    {
        public:
            float x = 0;
            float y = 0;
            float z = 0;

            vector3() {};

            vector3(float x, float y, float z)
            {
                this->x = x;
                this->y = y;
                this->z = z;
            };

            /// <summary>
            /// Subtracts two vectors
            /// </summary>
            /// <param name="src">vector 1</param>
            /// <param name="dst">vector 2</param>
            /// <returns>vector 1 - vector 2</returns>
            static vector3 Subtract(vector3 src, vector3 dst)
            {
                vector3 diff;
                diff.x = src.x - dst.x;
                diff.y = src.y - dst.y;
                diff.z = src.z - dst.z;
                return diff;
            }

            /// <summary>
            /// Divides a vector by a constant
            /// </summary>
            /// <param name="src">vector</param>
            /// <param name="num">constant</param>
            /// <returns>new vector</returns>
            static vector3 Divide(vector3 src, float num)
            {
                vector3 vec;
                vec.x = src.x / num;
                vec.y = src.y / num;
                vec.z = src.z / num;

                return vec;
            }

            /// <summary>
            /// Gets magnitude of current vector
            /// </summary>
            /// <returns>Magnitude of current vector</returns>
            float Magnitude()
            {
                return vector3::Magnitude(vector3(this->x, this->y, this->z));
            }

            /// <summary>
            /// Magnitude of vector
            /// </summary>
            /// <param name="vec">Vector to get the magnitude of</param>
            /// <returns>Returns the magnitude of the vector</returns>
            static float Magnitude(vector3 vec)
            {
                return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            }

            /// <summary>
            /// Dot Product of two Vector3s
            /// </summary>
            /// <param name="src">First vector</param>
            /// <param name="dst">Second vector</param>
            /// <returns></returns>
            static float DotProduct(vector3 src, vector3 dst)
            {
                return src.x * dst.x + src.y * dst.y + src.z * dst.z;
            }
    };

    /// <summary>
    /// Vector 4 Class
    /// </summary>
    class vector4
    {
        public:
            float x = 0;
            float y = 0;
            float z = 0;
            float w = 0;

            vector4() {};

            vector4(float x, float y, float z, float w)
            {
                this->x = x;
                this->y = y;
                this->z = z;
                this->w = w;
            };
    };
    

    
}