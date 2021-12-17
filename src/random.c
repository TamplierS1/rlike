#include "mt19937ar.h"

#include "random.h"

int rand_random_int(int min, int max)
{
    // This is to avoid deleting by 0.
    min++;
    max++;
    // genrand_int32 generates up to a certain number exclusively.
    // So to include that number into generation we add one to max.
    max++;

    if (min < 0)
    {
        min -= min;
        max -= min;
    }

    int result = genrand_int32() % max;
    if (result < min)
        return min - 1;
    else if (min < 0)
        return result + min - 1;
    else
        return result - 1;
}
