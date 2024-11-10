#include "WSSD.hpp"

WSSD::WSSD() : st_(NULL), end_(NULL), weight_(NULL), enInfo_(NULL)
{
    size_ = 0;
}

WSSD::~WSSD()
{
}

void WSSD::Process(const uint32_t *st,
                   const uint32_t *end,
                   const uint32_t *weight,
                   const uint32_t *enInfo,
                   uint32_t num)
{
    InitParams(st, end, weight, enInfo, num);

    uint32_t guessAdjRatio = GetGuessRatio();

    cout << guessAdjRatio << endl;

    auto startA = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        GradientAdj(guessAdjRatio);
    }
    cout << "gradient function: " << GradientAdj(guessAdjRatio) << endl;
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::microseconds>(endA - startA);

    auto startB = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        PointwiseSearchAdj();
    }
    cout << "PointwiseSearchAdj " << PointwiseSearchAdj() << endl;
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::microseconds>(endB - startB);

    std::cout << "Time taken by function GradientAdj: " << durationA.count() << " microseconds" << std::endl;
    std::cout << "Time taken by function PointwiseSearchAdj: " << durationB.count() << " microseconds" << std::endl;
}

void WSSD::InitParams(const uint32_t *st,
                      const uint32_t *end,
                      const uint32_t *weight,
                      const uint32_t *enInfo,
                      uint32_t num)
{
    st_ = st;
    end_ = end;
    weight_ = weight;
    enInfo_ = enInfo;
    size_ = num;
}

uint32_t WSSD::GetGuessRatio()
{
    uint32_t mapAdjRatio[MAX_METERMODULE_NUM * 2];
    uint32_t mapWeight[MAX_METERMODULE_NUM * 2];
    uint32_t mapSize = 0;
    uint32_t weightSum = 0;
    uint32_t halfWeightSum = 0;
    uint32_t guessAdjRatio = 0;

    memset(mapAdjRatio, 0, MAX_METERMODULE_NUM * 2 * sizeof(uint32_t));
    memset(mapWeight, 0, MAX_METERMODULE_NUM * 2 * sizeof(uint32_t));

    for (uint32_t i = 0; i < size_; i++)
    {
        uint32_t idx = enInfo_[i];
        const uint32_t adjStart = st_[idx];
        const uint32_t adjEnd = end_[idx];
        const uint32_t weight = weight_[idx];

        PushMap(adjStart, weight, mapAdjRatio, mapWeight, &mapSize);
        weightSum += weight;

        if (adjStart != adjEnd)
        {
            PushMap(adjEnd, weight, mapAdjRatio, mapWeight, &mapSize);
            weightSum += weight;
        }
    }

    totalAdjSt_ = mapAdjRatio[0];
    totalAdjEnd_ = mapAdjRatio[mapSize - 1];

    halfWeightSum = weightSum / 2;

    print(mapAdjRatio);
    print(mapWeight);

    uint32_t tempWeightSum = 0;

    for (uint32_t i = 0; i < mapSize; i++)
    {
        tempWeightSum += mapWeight[i];
        if (tempWeightSum >= halfWeightSum)
        {
            guessAdjRatio = mapAdjRatio[i];
            break;
        }
    }

    return guessAdjRatio;
}

void WSSD::PushMap(const uint32_t inAdj, const uint32_t inWeight, uint32_t *adj, uint32_t *weight, uint32_t *size)
{
    uint32_t targetIdx = 0;
    uint32_t currentMapLength = *size;

    while (targetIdx < currentMapLength)
    {
        if (inAdj < adj[targetIdx])
        {
            break;
        }
        targetIdx++;
    }

    for (uint32_t i = currentMapLength; i > targetIdx; i--)
    {
        adj[i] = adj[i - 1];
        weight[i] = weight[i - 1];
    }

    adj[targetIdx] = inAdj;
    weight[targetIdx] = inWeight;

    *size = currentMapLength + 1;
}

int32_t WSSD::CalcGradient(uint32_t ratio)
{
    int32_t sum = 0;
    for (uint32_t i = 0; i < size_; i++)
    {
        uint32_t idx = enInfo_[i];
        const uint32_t adjStart = st_[idx];
        const uint32_t adjEnd = end_[idx];
        const uint32_t weight = weight_[idx];

        if (ratio < adjStart)
        {
            sum += 2 * ((int)ratio - (int)adjStart) * (int)weight;
        }
        else if (ratio > adjEnd)
        {
            sum += 2 * ((int)ratio - (int)adjEnd) * (int)weight;
        }
    }
    return sum;
}

uint32_t WSSD::CalcWeightedSquareSumDiff(uint32_t ratio)
{
    uint32_t costSum = 0;
    for (uint32_t i = 0; i < size_; i++)
    {
        uint32_t idx = enInfo_[i];
        const uint32_t adjStart = st_[idx];
        const uint32_t adjEnd = end_[idx];
        const uint32_t weight = weight_[idx];

        if (ratio < adjStart)
        {
            costSum += (adjStart - ratio) * (adjStart - ratio) * weight;
        }
        else if (ratio > adjEnd)
        {
            costSum += (ratio - adjEnd) * (ratio - adjEnd) * weight;
        }
    }

    return costSum;
}

uint32_t WSSD::GradientAdj(uint32_t stAdj)
{
    int32_t iter = 0;
    int curAdjRatio = stAdj;
    while (iter++ <= MAX_ITERATION)
    {
        int gradient = CalcGradient(curAdjRatio);
        int step = gradient * pow(0.96, iter);

        int nextAdj = curAdjRatio - LEARNING_RATE * step;

        if (ABS(nextAdj - curAdjRatio) < TOLERANCE)
        {
            break;
        }
        else
        {
            curAdjRatio = nextAdj;
        }
    }
    return curAdjRatio;
}

uint32_t WSSD::PointwiseSearchAdj()
{
    uint32_t minCostSum = 1 << 31;
    uint32_t minAdjIdx = 0;
    for (uint32_t adjIdx = totalAdjSt_; adjIdx <= totalAdjEnd_; adjIdx++)
    {
        uint32_t cost = CalcWeightedSquareSumDiff(adjIdx);

        if (cost < minCostSum)
        {
            minCostSum = cost;
            minAdjIdx = adjIdx;
        }
    }

    return minAdjIdx;
}