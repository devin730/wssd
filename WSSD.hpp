#ifndef __WSSD_HPP__
#define __WSSD_HPP__

#include <iostream>
#include <cstring>
#include <cmath>
#include <chrono>

using namespace std;

#define ABS(x) (x)>0?x:-(x)
#define MAX_METERMODULE_NUM (10)
#define LEARNING_RATE (0.0001)
#define TOLERANCE (1)
#define MAX_ITERATION (256)

class WSSD
{
public:
    WSSD();
    ~WSSD();

    void Process(const uint32_t *st,
                 const uint32_t *end,
                 const uint32_t *weight,
                 const uint32_t *enInfo,
                 uint32_t num);

private:
    const uint32_t *st_;
    const uint32_t *end_;
    const uint32_t *weight_;
    const uint32_t *enInfo_;
    uint32_t size_;

    uint32_t totalAdjSt_;
    uint32_t totalAdjEnd_;

    void InitParams(const uint32_t *st,
                    const uint32_t *end,
                    const uint32_t *weight,
                    const uint32_t *enInfo,
                    uint32_t num);

    uint32_t GetGuessRatio();
    void PushMap(const uint32_t inAdj, const uint32_t inWeight, uint32_t* adj,
                       uint32_t* weight, uint32_t* size);

    int32_t CalcGradient(uint32_t ratio);
    uint32_t CalcWeightedSquareSumDiff(uint32_t ratio);
    uint32_t GradientAdj(uint32_t stAdj);
    uint32_t PointwiseSearchAdj();
};

inline void print(uint32_t* in)
{
    for(int i=0;i<10;i++)
    {
        cout << in[i] << " ";
    }
    cout << endl;
}

#endif