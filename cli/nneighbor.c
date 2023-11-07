#include "nneighbor.h"

double _nearer(double, double, double);

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

    int left = 0, right = size - 1, guess = 0;
    while (left < right)
    {
        double interp = (target - array[left]) / (array[right] - array[left]);
        guess = left + (int)(interp * (right - left));

        if (array[guess] == target)
        {
            if (index)
                *index = guess;
            return array[guess];
        }

        if (target < array[guess])
        {
            if (guess > 0 && target > array[guess - 1])
            {
                double nearer_value = _nearer(array[guess - 1], array[guess], target);
                if (index)
                    *index = (nearer_value == array[guess - 1]) ? (guess - 1) : guess;
                return nearer_value;
            }
            right = guess;
        }
        else
        {
            if (guess < size - 1 && target < array[guess + 1])
            {
                double nearer_value = _nearer(array[guess], array[guess + 1], target);
                if (index)
                    *index = (nearer_value == array[guess]) ? guess : (guess + 1);
                return nearer_value;
            }
            left = guess + 1;
        }
    }

    if (index)
        *index = guess;
    return array[guess];
}

double _nearer(double a, double b, double target)
{
    if (target - a >= b - target)
        return b;
    else
        return a;
}