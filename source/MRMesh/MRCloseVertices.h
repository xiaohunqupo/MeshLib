#pragma once

#include "MRMeshFwd.h"

namespace MR
{

/// returns a map where each valid vertex is mapped to the smallest valid vertex Id located within given distance (including itself), and this smallest vertex is mapped to itself,
/// each vertex not from valid set is mapped to itself
[[nodiscard]] MRMESH_API VertMap findSmallestCloseVertices( const Mesh & mesh, float closeDist );
/// returns a map where each valid vertex is mapped to the smallest valid vertex Id located within given distance (including itself), and this smallest vertex is mapped to itself,
/// each vertex not from valid set is mapped to itself
[[nodiscard]] MRMESH_API VertMap findSmallestCloseVertices( const VertCoords & points, float closeDist, const VertBitSet * valid = nullptr );

/// finds all close vertices, where for each vertex there is another one located within given distance
[[nodiscard]] MRMESH_API VertBitSet findCloseVertices( const Mesh & mesh, float closeDist );
/// finds all close vertices, where for each vertex there is another one located within given distance
[[nodiscard]] MRMESH_API VertBitSet findCloseVertices( const VertCoords & points, float closeDist, const VertBitSet * valid = nullptr );
/// finds all close vertices, where for each vertex there is another one located within given distance; smallestMap is the result of findSmallestCloseVertices function call
[[nodiscard]] MRMESH_API VertBitSet findCloseVertices( const VertMap & smallestMap );

/// finds all twin edges, which connect vertices different by id but sharing same coordinates
[[nodiscard]] MRMESH_API EdgeBitSet findTwinEdges( const Mesh & mesh, float closeDist );
/// finds all twin edges, which connect vertices different by id but sharing same coordinates
[[nodiscard]] MRMESH_API UndirectedEdgeBitSet findTwinUndirectedEdges( const Mesh & mesh, float closeDist );

} //namespace MR
