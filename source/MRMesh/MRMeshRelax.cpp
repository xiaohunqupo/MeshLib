#include "MRMeshRelax.hpp"
#include "MRMesh.h"
#include "MRBestFit.h"
#include "MREdgePaths.h"
#include "MRBestFitQuadric.h"
#include "MRVector4.h"
#include "MRRegionBoundary.h"
#include "MRMeshComponents.h"
#include "MRLaplacian.h"
#include "MRLineSegm.h"
#include "MRGeodesicPath.h"
#include "MRSymMatrix2.h"

namespace MR
{

[[maybe_unused]] static void testForScalars()
{
    MeshTopology topology;
    VertScalars field;
    relaxT( topology, field );
}

bool relax( Mesh& mesh, const MeshRelaxParams& params, const ProgressCallback& cb )
{
    mesh.invalidateCaches();
    return relaxT( mesh.topology, mesh.points, params, cb );
}

bool relax( const MeshTopology& topology, VertCoords& points, const MeshRelaxParams& params, const ProgressCallback& cb )
{
    return relaxT( topology, points, params, cb );
}

Vector3f vertexPosEqualNeiAreas( const MeshTopology& topology, const VertCoords& points, VertId v, bool noShrinkage )
{
    // computation in doubles improves quality of the result in case of degenerate input
    SymMatrix3d mat;
    Vector3d rhs;
    const EdgeId e0 = topology.edgeWithOrg( v );
    EdgeId ei = e0;
    EdgeId en = topology.next( ei );
    auto pi = Vector3d( destPnt( topology, points, ei ) );
    auto pn = Vector3d( destPnt( topology, points, en ) );
    for (;;)
    {
        if ( topology.left( ei ) )
        {
            const auto m = crossSquare( pn - pi );
            mat += m;
            rhs += m * pi;
        }
        if ( en == e0 )
            break;
        ei = en;
        pi = pn;
        en = topology.next( ei );
        pn = Vector3d( destPnt( topology, points, en ) );
    }

    if ( noShrinkage )
    {
        const auto norm = Vector3d( normal( topology, points, v ) );
        const auto [x,y] = norm.perpendicular();
        SymMatrix2d mat2;
        const auto mx = mat * x;
        mat2.xx = dot( mx, x );
        mat2.xy = dot( mx, y );
        mat2.yy = dot( mat * y, y );

        const auto det = mat2.det();
        const auto tr = mat2.trace();
        if ( DBL_EPSILON * std::abs( tr * tr ) >= std::abs( det ) )
            return points[v]; // the linear system cannot be trusted

        const auto p0 = dot( norm, Vector3d( points[v] ) ) * norm;
        rhs -= mat * p0;
        Vector2d rhs2;
        rhs2.x = dot( rhs, x );
        rhs2.y = dot( rhs, y );

        const auto sol2 = mat2.inverse( det ) * rhs2;
        return Vector3f( p0 + x * sol2.x + y * sol2.y );
    }

    const auto det = mat.det();
    const auto tr = mat.trace();
    if ( DBL_EPSILON * std::abs( tr * tr * tr ) >= std::abs( det ) )
        return points[v]; // the linear system cannot be trusted

    return Vector3f( mat.inverse( det ) * rhs );
}

Vector3f vertexPosEqualNeiAreas( const Mesh& mesh, VertId v, bool noShrinkage )
{
    return vertexPosEqualNeiAreas( mesh.topology, mesh.points, v, noShrinkage );
}

bool equalizeTriAreas( const MeshTopology& topology, VertCoords& points, const MeshEqualizeTriAreasParams& params, const ProgressCallback& cb )
{
    assert( !params.weights ); // custom weights are not supported
    if ( params.iterations <= 0 )
        return true;

    MR_TIMER;
    VertLimiter limiter( points, params );

    VertCoords newPoints;
    const VertBitSet& zone = topology.getVertIds( params.region );
    for ( int i = 0; i < params.iterations; ++i )
    {
        auto internalCb = subprogress( cb, [&]( float p ) { return ( float( i ) + p ) / float( params.iterations ); } );
        newPoints = points;
        if ( !BitSetParallelFor( zone, [&]( VertId v )
        {
            auto e0 = topology.edgeWithOrg( v );
            if ( !e0.valid() )
                return;
            auto np = newPoints[v];
            auto pushForce = params.force * ( vertexPosEqualNeiAreas( topology, points, v, params.noShrinkage ) - np );
            np += pushForce;
            newPoints[v] = limiter( v, np );
        }, internalCb ) )
            return false;
        points.swap( newPoints );
    }
    if ( params.hardSmoothTetrahedrons )
        hardSmoothTetrahedrons( topology, points, params.region );
    return true;
}

bool equalizeTriAreas( Mesh& mesh, const MeshEqualizeTriAreasParams& params, const ProgressCallback& cb )
{
    if ( params.iterations > 0 )
        mesh.invalidateCaches();
    return equalizeTriAreas( mesh.topology, mesh.points, params, cb );
}

bool relaxKeepVolume( const MeshTopology& topology, VertCoords& points, const MeshRelaxParams& params, const ProgressCallback& cb )
{
    assert( !params.weights ); // custom weights are not supported
    if ( params.iterations <= 0 )
        return true;

    MR_TIMER;
    VertLimiter limiter( points, params );

    VertCoords newPoints;

    const VertBitSet& zone = topology.getVertIds( params.region );
    std::vector<Vector3f> vertPushForces( zone.size() );
    for ( int i = 0; i < params.iterations; ++i )
    {
        auto internalCb1 = subprogress( cb, [&]( float p ) { return ( float( i ) + p * 0.5f ) / float( params.iterations ); } );
        auto internalCb2 = subprogress( cb, [&]( float p ) { return ( float( i ) + p * 0.5f + 0.5f ) / float( params.iterations ); } );
        newPoints = points;
        if ( !BitSetParallelFor( zone, [&]( VertId v )
        {
            Vector3d sum;
            int count = 0;
            for ( auto e : orgRing( topology, v ) )
            {
                sum += Vector3d( points[topology.dest( e )] );
                ++count;
            }
            vertPushForces[v] = params.force * ( Vector3f{sum / double( count )} - points[v] );
        }, internalCb1 ) )
            return false;

        if ( !BitSetParallelFor( zone, [&]( VertId v )
        {
            Vector3d sum;
            int count = 0;
            for ( auto e : orgRing( topology, v ) )
            {
                auto d = topology.dest( e );
                if ( zone.test( d ) )
                    sum += Vector3d( vertPushForces[d] );
                ++count;
            }
            auto np = newPoints[v] + vertPushForces[v] - Vector3f{ sum / double( count ) };
            newPoints[v] = limiter( v, np );
        }, internalCb2 ) )
            return false;

        points.swap( newPoints );
    }
    if ( params.hardSmoothTetrahedrons )
        hardSmoothTetrahedrons( topology, points, params.region );
    return true;
}

bool relaxKeepVolume( Mesh& mesh, const MeshRelaxParams& params, const ProgressCallback& cb )
{
    if ( params.iterations > 0 )
        mesh.invalidateCaches();
    return relaxKeepVolume( mesh.topology, mesh.points, params, cb );
}

bool relaxApprox( const MeshTopology& topology, VertCoords& points, const MeshApproxRelaxParams& params, const ProgressCallback& cb )
{
    assert( !params.weights ); // custom weights are not supported
    if ( params.iterations <= 0 )
        return true;

    MR_TIMER;
    VertLimiter limiter( points, params );

    float surfaceRadius = ( params.surfaceDilateRadius <= 0.0f ) ?
        ( float( std::sqrt( area( topology, points ) ) ) * 1e-3f ) : params.surfaceDilateRadius;

    VertCoords newPoints;
    const VertBitSet& zone = topology.getVertIds( params.region );
    for ( int i = 0; i < params.iterations; ++i )
    {
        auto internalCb = subprogress( cb, [&]( float p ) { return ( float( i ) + p ) / float( params.iterations ); } );
        newPoints = points;
        if ( !BitSetParallelFor( zone, [&] ( VertId v )
        {
            auto e0 = topology.edgeWithOrg( v );
            if ( !e0.valid() )
                return;
            VertBitSet neighbors( topology.lastValidVert() + 1 );
            neighbors.set( v );

            dilateRegion( topology, points, neighbors, surfaceRadius );

            PointAccumulator accum;
            int count = 0;
            for ( auto newV : neighbors )
            {
                Vector3d ptD = Vector3d( points[newV] );
                accum.addPoint( ptD );
                ++count;
            }
            if ( count < 6 )
                return;

            auto np = newPoints[v];

            Vector3f target;
            if ( params.type == RelaxApproxType::Planar )
                target = accum.getBestPlanef().project( np );
            else if ( params.type == RelaxApproxType::Quadric )
            {
                AffineXf3d basis = accum.getBasicXf();
                basis.A = basis.A.transposed();
                std::swap( basis.A.x, basis.A.y );
                std::swap( basis.A.y, basis.A.z );
                basis.A = basis.A.transposed();
                auto basisInv = basis.inverse();

                QuadricApprox approxAccum;
                for ( auto newV : neighbors )
                    approxAccum.addPoint( basisInv( Vector3d( points[newV] ) ) );

                auto centerPoint = basisInv( Vector3d( points[v] ) );
                const auto coefs = approxAccum.calcBestCoefficients();
                centerPoint.z =
                    coefs[0] * centerPoint.x * centerPoint.x +
                    coefs[1] * centerPoint.x * centerPoint.y +
                    coefs[2] * centerPoint.y * centerPoint.y +
                    coefs[3] * centerPoint.x +
                    coefs[4] * centerPoint.y +
                    coefs[5];
                target = Vector3f( basis( centerPoint ) );
            }
            np += ( params.force * ( target - np ) );
            newPoints[v] = limiter( v, np );
        }, internalCb ) )
            return false;
        points.swap( newPoints );
    }
    if ( params.hardSmoothTetrahedrons )
        hardSmoothTetrahedrons( topology, points, params.region );
    return true;
}

bool relaxApprox( Mesh& mesh, const MeshApproxRelaxParams& params, const ProgressCallback& cb )
{
    if ( params.iterations > 0 )
        mesh.invalidateCaches();
    return relaxApprox( mesh.topology, mesh.points, params, cb );
}

void removeSpikes( const MeshTopology& topology, VertCoords& points, int maxIterations, float minSumAngle, const VertBitSet * region )
{
    if ( maxIterations <= 0 )
        return;

    MR_TIMER;

    for ( int i = 0; i < maxIterations; ++i )
    {
        auto spikeVerts = findSpikeVertices( topology, points, minSumAngle, region ).value();
        if ( spikeVerts.none() )
            break;
        relax( topology, points, { { 1,&spikeVerts } } );
    }
}

void removeSpikes( Mesh & mesh, int maxIterations, float minSumAngle, const VertBitSet * region )
{
    return removeSpikes( mesh.topology, mesh.points, maxIterations, minSumAngle, region );
}

void smoothRegionBoundary( Mesh & mesh, const FaceBitSet & regionFaces, int numIters )
{
    MR_TIMER;
    assert( numIters > 0 );
    if ( !regionFaces.any() || numIters <= 0 )
        return;

    // value 1 for out-of-region vertices
    VertScalars scalarField( mesh.topology.vertSize(), 1 );

    const auto regionVerts = getIncidentVerts( mesh.topology, regionFaces );
    // value -1 for in-region vertices
    for( auto v : regionVerts )
        scalarField[v] = -1;

    /// free vertices must have both in-region and out-of-region neighbor faces
    VertBitSet freeVerts = getIncidentVerts( mesh.topology, mesh.topology.getValidFaces() - regionFaces ) & regionVerts;

    for ( const auto & cc : MeshComponents::getAllComponentsVerts( mesh ) )
    {
        auto freeCC = cc & freeVerts;
        auto numfree = freeCC.count();
        auto numCC = cc.count();
        assert( numfree <= numCC );
        if ( numfree <= 0 )
            continue; // too small connected component
        if ( numfree < numCC )
            continue; // at least one fixed vertex in the component

        // all component vertices are free, just fix them all (to -1) to avoid under-determined system of equations
        freeVerts -= cc;
    }

    // change topology: eliminate not boundary edges joining two boundary vertices
    for ( auto v : freeVerts )
    {
        mesh.topology.flipEdgesOut( v, [&]( EdgeId e )
        {
            assert( mesh.topology.org( e ) == v );
            if ( regionFaces.test( mesh.topology.left( e ) ) != regionFaces.test( mesh.topology.right( e ) ) )
                return false;

            auto c = mesh.topology.dest( e );

            if ( !freeVerts.test( c ) )
                return false;

            auto b = mesh.topology.dest( mesh.topology.prev( e ) );
            auto d = mesh.topology.dest( mesh.topology.next( e ) );
            if ( freeVerts.test( b ) && freeVerts.test( c ) )
              return false;

            if ( mesh.topology.findEdge( d, b ) )
                return false; // multiple edges between b and d will appear

            auto ap = mesh.points[v];
            auto bp = mesh.points[b];
            auto cp = mesh.points[c];
            auto dp = mesh.points[d];
            return isUnfoldQuadrangleConvex( ap, bp, cp, dp );
        } );
    }

    Laplacian lap( mesh );
    std::vector<Vector3f> newPos;
    for( int iter = 0; iter < numIters; ++iter )
    {
        lap.init( freeVerts, EdgeWeights::Cotan, VertexMass::Unit, Laplacian::RememberShape::No );
        lap.applyToScalar( scalarField );

        newPos.clear();
        newPos.reserve( freeVerts.count() );
        for ( const auto v : freeVerts )
        {
            const auto pt = mesh.points[v];
            Vector3f bestPos = pt;
            float bestDist2 = FLT_MAX;
            for ( auto e : orgRing( mesh.topology, v ) )
            {
                if ( !mesh.topology.left( e ) )
                    continue;

                VertId vs[3] = {
                    v,
                    mesh.topology.dest( e ),
                    mesh.topology.dest( mesh.topology.next( e ) )
                };

                for ( int i = 0; i < 3; ++i )
                {
                    const auto val  = scalarField[vs[0]];
                    const auto val1 = scalarField[vs[1]];
                    const auto val2 = scalarField[vs[2]];

                    if ( val * val1 >= 0 )
                        continue;
                    if ( val * val1 >= 0 )
                        continue;

                    LineSegm3f ls;
                    const float c1 = val / ( val - val1 );
                    ls.a = ( 1 - c1 ) * mesh.points[vs[0]] + c1 * mesh.points[vs[1]];
                    const float c2 = val / ( val - val2 );
                    ls.b = ( 1 - c2 ) * mesh.points[vs[0]] + c2 * mesh.points[vs[2]];

                    const auto proj = closestPointOnLineSegm( pt, ls );
                    const auto dist2 = ( pt - proj ).lengthSq();
                    if ( dist2 < bestDist2 )
                    {
                        bestPos = proj;
                        bestDist2 = dist2;
                    }

                    std::rotate( vs, vs + 1, vs + 3 );
                }
            }

            newPos.push_back( bestPos );
        }

        int n = 0;
        for ( const auto v : freeVerts )
            // 0.75 to reduce oscillation on the next iteration
            mesh.points[v] += 0.75f * ( newPos[n++] - mesh.points[v] );
    }
}

void hardSmoothTetrahedrons( Mesh & mesh, const VertBitSet *region )
{
    mesh.invalidateCaches();
    return hardSmoothTetrahedronsT( mesh.topology, mesh.points, region );
}

void hardSmoothTetrahedrons( const MeshTopology& topology, VertCoords& points, const VertBitSet *region )
{
    return hardSmoothTetrahedronsT( topology, points, region );
}

} //namespace MR
