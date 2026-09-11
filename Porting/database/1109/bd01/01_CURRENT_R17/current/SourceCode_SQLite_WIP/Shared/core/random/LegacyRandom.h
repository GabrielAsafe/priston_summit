#pragma once

namespace summit {
namespace core {
namespace random {

enum class FloatRangePolicy
{
    PreserveSignedSpan,
    NormalizeSpan
};

using RandomFunction = int (*)();

inline int RandomInt(int minValue, int maxValue, RandomFunction random)
{
    const int span = (maxValue + 1) - minValue;
    if(span == 0)
        return maxValue;

    return minValue + (random() % span);
}

inline float RandomFloat(
    float minValue,
    float maxValue,
    int precisionScale,
    FloatRangePolicy policy,
    RandomFunction random)
{
    float span = maxValue - minValue;
    if(policy == FloatRangePolicy::NormalizeSpan && span < 0.0f)
        span = -span;

    const int scaledSpan = static_cast<int>(span * static_cast<float>(precisionScale));
    if(scaledSpan == 0)
        return maxValue;

    const int offset = random() % (scaledSpan + 1);
    return minValue + (static_cast<float>(offset) / static_cast<float>(precisionScale));
}

inline int MiddleInt(int minValue, int maxValue)
{
    return minValue + ((maxValue - minValue) / 2);
}

inline int RandomIntNormal(
    int minValue,
    int maxValue,
    bool normalMode,
    RandomFunction random)
{
    if(!normalMode)
        return RandomInt(minValue, maxValue, random);

    const int sourceSpan = maxValue - minValue;
    int quarterSpan = sourceSpan / 4;
    if(quarterSpan == 0 && sourceSpan >= 2)
        quarterSpan = 1;

    const int narrowedMin = minValue + quarterSpan;
    const int narrowedMax = maxValue - quarterSpan;
    const int narrowedSpan = (narrowedMax + 1) - narrowedMin;

    if(narrowedSpan == 0)
        return narrowedMax;

    return narrowedMin + (random() % narrowedSpan);
}

} // namespace random
} // namespace core
} // namespace summit
