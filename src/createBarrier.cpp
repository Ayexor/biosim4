// createBarrier.cpp

#include <cassert>
#include "simulator.h"

namespace BS {

// This generates barrier points, which are grid locations with value
// BARRIER. A list of barrier locations is saved in private member
// Grid::barrierLocations and, for some scenarios, Grid::barrierCenters.
// Those members are available read-only with Grid::getBarrierLocations().
// This function assumes an empty grid. This is typically called by
// the main simulator thread after Grid::init() or Grid::zeroFill().

// This file typically is under constant development and change for
// specific scenarios.

void Grid::createBarrier(unsigned barrierType)
{
    barrierLocations.clear();
    barrierCenters.clear();  // used only for some barrier types

    auto drawBox = [&](int16_t minX, int16_t minY, int16_t maxX, int16_t maxY) {
        for (int16_t x = minX; x <= maxX; ++x) {
            for (int16_t y = minY; y <= maxY; ++y) {
                grid.set(x, y, BARRIER);
                barrierLocations.push_back( {x, y} );
            }
        }
    };

    switch(barrierType) {
    case 0:
        return;

    // Vertical bar in constant location
    case 1:
        {
            int16_t minX = p.sizeX / 2;
            int16_t maxX = minX + 1;
            int16_t minY = p.sizeY / 4;
            int16_t maxY = minY + p.sizeY / 2;

            for (int16_t x = minX; x <= maxX; ++x) {
                for (int16_t y = minY; y <= maxY; ++y) {
                    grid.set(x, y, BARRIER);
                    barrierLocations.push_back( {x, y} );
                }
            }
        }
        break;

    // Vertical bar in random location
    case 2:
        {
            int16_t midX = randomUint(p.sizeX / 10, p.sizeX - p.sizeX / 10);
            int16_t midY = randomUint(p.sizeY / 4, p.sizeY - p.sizeY / 4);
            int16_t minX = midX - 1;
            int16_t maxX = midX + 1;
            int16_t minY = midY - p.sizeY / 4;
            int16_t maxY = midY + p.sizeY / 4;

            barrierCenters.push_back({midX, midY});

            for (int16_t x = minX; x <= maxX; ++x) {
                for (int16_t y = minY; y <= maxY; ++y) {
                    grid.set(x, y, BARRIER);
                    barrierLocations.push_back( {x, y} );
                }
            }
        }
        break;

    // five blocks staggered
    case 3:
        {
            int16_t blockSizeX = 2;
            int16_t blockSizeY = p.sizeX / 3;

            int16_t x0 = p.sizeX / 4 - blockSizeX / 2;
            int16_t y0 = p.sizeY / 4 - blockSizeY / 2;
            int16_t x1 = x0 + blockSizeX;
            int16_t y1 = y0 + blockSizeY;

            drawBox(x0, y0, x1, y1);
            x0 += p.sizeX / 2;
            x1 = x0 + blockSizeX;
            drawBox(x0, y0, x1, y1);
            y0 += p.sizeY / 2;
            y1 = y0 + blockSizeY;
            drawBox(x0, y0, x1, y1);
            x0 -= p.sizeX / 2;
            x1 = x0 + blockSizeX;
            drawBox(x0, y0, x1, y1);
            x0 = p.sizeX / 2 - blockSizeX / 2;
            x1 = x0 + blockSizeX;
            y0 = p.sizeY / 2 - blockSizeY / 2;
            y1 = y0 + blockSizeY;
            drawBox(x0, y0, x1, y1);
            return;
        }
        break;

    // Horizontal bar in constant location
    case 4:
        {
            int16_t minX = p.sizeX / 4;
            int16_t maxX = minX + p.sizeX / 2;
            int16_t minY = p.sizeY / 2 + p.sizeY / 4;
            int16_t maxY = minY + 2;

            for (int16_t x = minX; x <= maxX; ++x) {
                for (int16_t y = minY; y <= maxY; ++y) {
                    grid.set(x, y, BARRIER);
                    barrierLocations.push_back( {x, y} );
                }
            }
        }
        break;

    // Three floating islands -- different locations every generation
    case 5:
        {
            float radius = 3;
            unsigned margin = radius * 4;
            const unsigned numIslands = 12;
            std::vector<Coord> centers(numIslands);

            auto randomLoc = [&]() {
                return Coord( (int16_t)randomUint(margin, p.sizeX - margin),
                              (int16_t)randomUint(margin, p.sizeY - margin) );
            };

            bool placementValid = false;
            while ( ! placementValid) {
                for (unsigned idx = 0; idx < numIslands; ++idx)
                    centers[idx] = randomLoc();

                placementValid = true;
                for (unsigned a = 0; a < numIslands - 1; ++a)
                    for (unsigned b = a + 1; b < numIslands; ++b)
                        if ( (centers[a] - centers[b]).length() < margin)
                            placementValid = false;
            }

            auto f = [&](Coord loc) {
                grid.set(loc, BARRIER);
                barrierLocations.push_back(loc);
            };

            for (unsigned idx = 0; idx < numIslands; ++idx) {
                barrierCenters.push_back(centers[idx]);
                visitNeighborhood(centers[idx], radius, f);
            }
        }
        break;

    // Spots, specified number, radius, locations
    case 6:
        {
            unsigned numberOfLocations = 5;
            float radius = 5.0;

            auto f = [&](Coord loc) {
                grid.set(loc, BARRIER);
                barrierLocations.push_back(loc);
            };

            unsigned verticalSliceSize = p.sizeY / (numberOfLocations + 1);

            for (unsigned n = 1; n <= numberOfLocations; ++n) {
                Coord loc = { (int16_t)(p.sizeX / 2),
                              (int16_t)(n * verticalSliceSize) };
                visitNeighborhood(loc, radius, f);
                barrierCenters.push_back(loc);
            }
        }
        break;

    default:
        assert(false);
    }
}

} // end namespace BS
