#pragma once

namespace presentation
{
struct PresentationTarget
{
    int x;
    int y;
    int z;

    PresentationTarget(int xValue = 0, int yValue = 0, int zValue = 0)
        : x(xValue), y(yValue), z(zValue) {}
};
}
