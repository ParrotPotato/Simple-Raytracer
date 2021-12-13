#ifndef VECTOR_HH
#define VECTOR_HH

#include <cmath>
#include <cstdio> // for snprintf'ing things ;P

#include "types.hh"

// TODO(nitesh): Make the vector component x, y, z, w interchangable with the color specific
// counter part like r, g, b, a

// vector2 defination

struct v2
{
    r32 x, y;

    v2(void) noexcept {x = y = 0.0f;}
    v2(r32 a) noexcept {x = y = a;}
    v2(r32 x_, r32 y_) noexcept {x = x_; y = y_;}

    inline r32 length_squared (void)  const noexcept
    {
        r32 result = (x * x + y * y);
        return result; 
    }

    inline r32 length (void) const noexcept
    {
        r32 result = sqrt(length_squared());
        return result;
    }

    inline v2 normalise(void)
    {
        r32 len = length();

        if(len == 0.0f) 
        {
            *this = v2(0.0);
            return *this;
        }

        this->x = this->x / len;
        this->y = this->y / len;

        return *this;
    }
    

    void to_string(char * buffer, const int bufferlen)
    {
        snprintf(buffer, bufferlen, "[%.2f,%.2f]\n", x, y);
    }
};

inline v2 operator + (const v2 & a, const v2 & b) noexcept
{
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline v2 operator - (const v2 & a, const v2 & b) noexcept
{
    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline v2 operator * (const r32 value, const v2 & a) noexcept
{
    v2 result;
    result.x = a.x * value;
    result.y = a.y * value;
    return result;
}

// vector3 defination

struct v3
{
    r32 x, y, z;

    v3(void) noexcept {x = y = z = 0.0f;}
    v3(r32 a) noexcept {x = y = z = a;}
    v3(r32 x_, r32 y_, r32 z_) noexcept {x = x_; y = y_; z = z_;}
    v3(const v2 & other){x = other.x; y = other.y; z = 0.0f;}

    inline r32 length_squared (void)  const noexcept
    {
        r32 result = (x * x + y * y + z * z);
        return result; 
    }

    inline r32 length (void) const noexcept
    {
        r32 result = sqrt(length_squared());
        return result;
    }

    inline v3 normalise(void)
    {
        r32 len = length();

        if(len == 0.0f) 
        {
            *this = v3(0.0);
            return *this;
        }

        this->x = this->x / len;
        this->y = this->y / len;
        this->z = this->z / len;

        return *this;
    }

    void to_string(char * buffer, const int bufferlen)
    {
        snprintf(buffer, bufferlen, "[%.2f,%.2f,%.2f]\n", x, y, z);
    }
};

inline v3 operator + (const v3 & a, const v3 & b) noexcept
{
    v3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline v3 operator - (const v3 & a, const v3 & b) noexcept
{
    v3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline v3 operator * (const r32 value, const v3 & a) noexcept
{
    v3 result;
    result.x = a.x * value;
    result.y = a.y * value;
    result.z = a.z * value;
    return result;
}

inline r32 dot(const v3 & a, const v3 & b) noexcept
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline v3 cross(const v3 & a, const v3 & b) noexcept
{
    v3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}


inline v3 normalised(const v3 & a)
{
    v3 result;
    r32 length = a.length();
    
    if(length == 0.0f) return v3(0.0);
    
    result.x = a.x / length;
    result.y = a.y / length;
    result.z = a.z / length;
    return result;
}

// vector4 defination

struct v4
{
    r32 x, y, z, w;

    v4(void) noexcept
    {
        x = y = z = w = 0.0;
    }
    
    v4(r32 value) noexcept
    {
        x = y = z = w = value;
    }

    v4(r32 x_, r32 y_, r32 z_, r32 w_) noexcept
    {
        x = x_;
        y = y_;
        z = z_;
        w = w_;
    }

    v4(const v3 & other) noexcept
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = 0.0;
    }

    void to_string(char * buffer, const int bufferlen)
    {
        snprintf(buffer, bufferlen, "[%.2f,%.2f,%.2f,%.2f]\n", x, y, z, w);
    }
};

inline v4 operator + (const v4 & a, const v4 & b) noexcept
{
    v4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline v4 operator - (const v4 & a, const v4 & b) noexcept
{
    v4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline v4 operator * (const r32 value, const v4 & a) noexcept
{
    v4 result;
    result.x = a.x * value;
    result.y = a.y * value;
    result.z = a.z * value;
    result.w = a.w * value;
    return result;
}


#endif
