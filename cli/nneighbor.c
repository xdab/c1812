#include "nneighbor.h"

double nearer(double, double, double);

double nneighbor(const double *array, int size, double target, int *index)
{
    if (size == 0)
    {
        if (index)
            *index = -1;
        return 0;
    }

    if (target <= array[0])
    {
        if (index)
            *index = 0;
        return array[0];
    }

    if (target >= array[size - 1])
    {
        if (index)
            *index = size - 1;
        return array[size - 1];
    }

    int left = 0, right = size - 1, mid = 0;
    while (left < right)
    {
        mid = left;
        double k = target - array[left];
        k *= (double)right - (double)left;
        k /= array[right] - array[left];
        mid += (int)k;

        if (array[mid] == target)
        {
            if (index)
                *index = mid;
            return array[mid];
        }

        if (target < array[mid])
        {
            if (mid > 0 && target > array[mid - 1])
            {
                double nearer_value = nearer(array[mid - 1], array[mid], target);
                if (index)
                    *index = (nearer_value == array[mid - 1]) ? (mid - 1) : mid;
                return nearer_value;
            }
            right = mid;
        }
        else
        {
            if (mid < size - 1 && target < array[mid + 1])
            {
                double nearer_value = nearer(array[mid], array[mid + 1], target);
                if (index)
                    *index = (nearer_value == array[mid]) ? mid : (mid + 1);
                return nearer_value;
            }
            left = mid + 1;
        }
    }

    if (index)
        *index = mid;
    return array[mid];
}

double nearer(double a, double b, double target)
{
    if (target - a >= b - target)
        return b;
    else
        return a;
}