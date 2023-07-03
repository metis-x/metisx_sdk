#pragma once
#include "mu.h"

#undef CLASS_NAME
#define CLASS_NAME MuPrint

struct CLASS_NAME : SLAVE_MU_PARENT
{
    SLAVE_CONSTRUCTOR
    {

    }
    
	void run(uint64_t header, int argc, void** argv)
	{
		MuHeader muHeader;
		muHeader.u64 = header;
        
        printf("Hello MetisX\n");
	}
}; 