#pragma once

#include "vector3.h"

class AAPlaneClass
{
public:

    enum AxisEnum
    {
        XNORMAL = 0,
        YNORMAL = 1,
        ZNORMAL = 2
    };

    AAPlaneClass(): Normal(), Dist(0)
	{
	}

	AAPlaneClass(AxisEnum normal,float dist) :
        Normal(normal),
        Dist(dist)
    {}

    void Set(AxisEnum normal, float dist)
    {
        Normal = normal;
        Dist = dist;
    }

    void Get_Normal(Vector3* normal) const
    {
        normal->Set(0,0,0);
        (*normal)[Normal] = 1.0f;
    }

    inline bool In_Front(const Vector3& point) const
    {
        return Dist > point[Normal];
    }

	AxisEnum    Normal;
    float       Dist;
};
