#pragma once

#include "MRId.h"
#include <cassert>
#include <utility>

namespace MR
{

/// describes a triangle as three vertex IDs sorted in a way to quickly find same triangles
/// irrespective of vertex order (clockwise or counterclockwise)
struct UnorientedTriangle : ThreeVertIds
{
    UnorientedTriangle( const ThreeVertIds & inVs,
        bool * outFlipped = nullptr ) ///< optional output: true if the orientation of the triangle has flipped
        : ThreeVertIds( inVs )
    {
        bool flipped = false;
        auto checkSwap = [this, &flipped]( int i, int j )
        {
            assert( i < j );
            assert( (*this)[i] != (*this)[j] );
            if ( (*this)[i] > (*this)[j] )
            {
                flipped = !flipped;
                std::swap( (*this)[i], (*this)[j] );
            }
        };
        checkSwap( 0, 1 );
        checkSwap( 0, 2 );
        checkSwap( 1, 2 );
        if ( outFlipped )
            *outFlipped = flipped;
    }
    friend bool operator==( const UnorientedTriangle& a, const UnorientedTriangle& b ) = default;
};

/// defines hash function for UnorientedTriangle
struct UnorientedTriangleHasher
{
    size_t operator()( const UnorientedTriangle& triplet ) const
    {
        return 
            2 * size_t( triplet[0] ) +
            3 * size_t( triplet[1] ) +
            5 * size_t( triplet[2] );
    }
};

} //namespace MR
