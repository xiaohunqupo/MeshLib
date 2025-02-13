#pragma once

namespace MR
{

/// determines the weight of each edge in applications like Laplacian
enum class EdgeWeights
{
    /// all edges have same weight=1
    Unit = 0,

    /// edge weight depends on local geometry and uses cotangent values
    Cotan,

    /// [deprecated] edge weight is equal to edge length times cotangent weight
    CotanTimesLength,

    /// cotangent edge weights and equation weights inversely proportional to square root of local area
    CotanWithAreaEqWeight
};

/// typically returned from callbacks to control the behavior of main algorithm
enum class Processing : bool
{
    Continue,
    Stop
};

/// the method how to choose between two opposite normal orientations
enum class OrientNormals
{
    TowardOrigin,
    AwayFromOrigin,
    Smart
};

enum class OffsetMode : int
{
    Smooth,     ///< create mesh using dual marching cubes from OpenVDB library
    Standard,   ///< create mesh using standard marching cubes implemented in MeshLib
    Sharpening  ///< create mesh using standard marching cubes with additional sharpening implemented in MeshLib
};

/// Type of object coloring,
/// \note that texture are applied over main coloring
enum class ColoringType
{
    SolidColor,   ///< Use one color for whole object
    PrimitivesColorMap, ///< Use different color (taken from faces colormap) for each primitive
    FacesColorMap = PrimitivesColorMap, ///< Use different color (taken from faces colormap) for each face (primitive for object mesh)
    LinesColorMap = PrimitivesColorMap, ///< Use different color (taken from faces colormap) for each line (primitive for object lines)
    VertsColorMap  ///< Use different color (taken from verts colormap) for each vertex
};

} //namespace MR
