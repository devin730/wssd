#include "WSSD.hpp"

int main()
{
    uint32_t adjustRatioStart[5] = {1, 512, 1024, 0, 0};
    uint32_t adjustRatioEnd[5] = {256, 512, 2048, 0, 0};
    uint32_t adjustRatioWeight[5] = {1000, 200, 2000, 0, 0};

    uint32_t enableMM[5] = {0,1,2,0,0};
    uint32_t num = 3;

    WSSD wssd;
    wssd.Process(adjustRatioStart, adjustRatioEnd, adjustRatioWeight, enableMM, num);

    return 0;
}