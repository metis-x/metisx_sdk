#pragma once
#include "mu.h"

#undef CLASS_NAME
#define CLASS_NAME MuSort

#define MAX_LEVELS 200000
__attribute__((section(".uninitialize"))) int beg[MAX_LEVELS], end[MAX_LEVELS];
struct CLASS_NAME : SLAVE_MU_PARENT
{
  SLAVE_CONSTRUCTOR
  {
  }
     void quick_sort(int *arr, uint64_t elements)
    {
        int64_t i = 0, L, R;
        uint64_t piv;
        beg[0] = 0;
        end[0] = elements;
        //printf("elements : %lld\n", elements);
        while (i >= 0)
        {
            L = beg[i];
            R = end[i] - 1;
            if (L < R)
            {
                piv = arr[L];
                if (i == MAX_LEVELS - 1)
                    __assert(0);
                while (L < R)
                {
                    while (arr[R] >= piv && L < R)
                    {
                        R--;
                    }
                    if (L < R)
                    {
                        arr[L++] = arr[R];
                    }
                    while (arr[L] <= piv && L < R)
                    {
                        L++;
                    }
                    if (L < R)
                    {
                        arr[R--] = arr[L];
                    }
                }
                arr[L]     = piv;
                beg[i + 1] = L + 1;
                end[i + 1] = end[i];
                end[i++]   = L;
            }
            else
            {
                i--;
            }
        }
        
    }

	void run(uint64_t header, int argc, void** argv)
	{
        quick_sort(reinterpret_cast<int*>(argv[0]), reinterpret_cast<uint64_t>(argv[1]));
	}
}; 