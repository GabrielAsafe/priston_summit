#pragma once
// Inventory layout decisions belong to the V2 domain. The DAT reader alone
// translates old pixel coordinates and item records into these plain values.
#include <vector>

namespace gameplay { namespace inventory {

enum class BagSpaceStatus {
    Available, NoSpace, InvalidItemSize, CorruptPlacement
};
struct BagRect {
    int x, y, width, height;
    BagRect(int px, int py, int w, int h)
        : x(px), y(py), width(w), height(h) {}
};
struct BagSpaceResult {
    BagSpaceStatus status;
    int x, y;
    BagSpaceResult(BagSpaceStatus s, int px = 0, int py = 0)
        : status(s), x(px), y(py) {}
    bool Available() const { return status == BagSpaceStatus::Available; }
};

class CompanionBagSpacePolicy final {
public:
    static const int OriginX = 23;
    static const int OriginY = 436;
    static const int Columns = 12;
    static const int Rows = 6;
    static const int CellSize = 22;

    static bool Intersects(const BagRect& a, const BagRect& b) {
        return a.x < b.x + b.width && b.x < a.x + a.width &&
               a.y < b.y + b.height && b.y < a.y + a.height;
    }
    static bool InBounds(const BagRect& r) {
        return r.width > 0 && r.height > 0 &&
               r.x >= OriginX && r.y >= OriginY &&
               r.x + r.width <= OriginX + Columns * CellSize &&
               r.y + r.height <= OriginY + Rows * CellSize;
    }
    static BagSpaceResult FindFree(const std::vector<BagRect>& present,
                                   int width, int height) {
        if(width <= 0 || height <= 0 ||
           width > Columns * CellSize || height > Rows * CellSize)
            return BagSpaceResult(BagSpaceStatus::InvalidItemSize);
        // A moved item must fit by actual pixel dimensions. Existing DAT
        // objects need NOT have x/y aligned to 22px: legacy positioning can
        // include offsets. We reject collisions using their real rectangles.
        for(std::size_t i = 0; i < present.size(); ++i) {
            if(!InBounds(present[i]))
                return BagSpaceResult(BagSpaceStatus::CorruptPlacement);
            for(std::size_t j = 0; j < i; ++j)
                if(Intersects(present[i], present[j]))
                    return BagSpaceResult(BagSpaceStatus::CorruptPlacement);
        }
        for(int row = 0; row < Rows; ++row)
            for(int col = 0; col < Columns; ++col) {
                const BagRect candidate(OriginX + col*CellSize,
                                        OriginY + row*CellSize, width, height);
                if(!InBounds(candidate)) continue;
                bool free = true;
                for(const auto& object : present)
                    if(Intersects(candidate, object)) { free = false; break; }
                if(free) return BagSpaceResult(BagSpaceStatus::Available,
                                               candidate.x, candidate.y);
            }
        return BagSpaceResult(BagSpaceStatus::NoSpace);
    }
};
}} // namespace gameplay::inventory
