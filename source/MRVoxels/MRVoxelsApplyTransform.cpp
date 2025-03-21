#include "MRVoxelsApplyTransform.h"
#include "MRVoxels/MRVDBFloatGrid.h"
#include "MRMesh/MRMatrix4.h"
#include "MRVDBConversions.h"
#include "MRObjectVoxels.h"

#include <openvdb/math/Maps.h>
#include <openvdb/tools/GridTransformer.h>


namespace MR
{


TransformVdbVolumeResult transformVdbVolume( const VdbVolume& volume, const AffineXf3f& xf0, bool fixBox, const Box3f& box )
{
    AffineXf3f xf = xf0;
    Box3f newBox;
    {
        Box3f pseudoBox;
        if ( box.valid() )
            pseudoBox = box;
        else
            pseudoBox = Box3f{ { 0.f, 0.f, 0.f }, mult( Vector3f( volume.dims ), volume.voxelSize ) };

        for ( auto c : getCorners( pseudoBox ) )
            newBox.include( xf( c ) );
    }

    bool boxFixed = false;
    if ( fixBox && box.valid() &&
         std::any_of( begin( newBox.min ), end( newBox.min ), [] ( float f ) { return f < 0; } ) )
    {
        Vector3f fixer;
        for ( int i = 0; i < 3; ++i )
        {
            if ( newBox.min[i] < 0 )
            {
                fixer[i] = -newBox.min[i];
                newBox.min[i] = 0;
                newBox.max[i] += fixer[i];
            }
        }

        xf = AffineXf3f::translation( fixer ) * xf;
        boxFixed = true;
    }

    Matrix4d m( Matrix3d( xf.A ), Vector3d( div( xf.b, volume.voxelSize ) ) );
    m = m.transposed();
    openvdb::math::AffineMap map( openvdb::math::Mat4d{ ( double* )&m } );
    openvdb::tools::GridTransformer tr( map.getConstMat4() );

    const FloatGrid& oldGrid = volume.data;
    auto newGrid = openvdb::FloatGrid::create( oldGrid->background() );
    newGrid->setGridClass( oldGrid->getGridClass() );
    tr.transformGrid<openvdb::tools::QuadraticSampler, openvdb::FloatGrid>( *volume.data, *newGrid );

    VdbVolume res;
    res = volume;
    res.data = MakeFloatGrid( std::move( newGrid ) );
    res.dims = Vector3i( div( newBox.max, volume.voxelSize ) );
    evalGridMinMax( res.data, res.min, res.max );

    return { .volume = res, .boxFixed = boxFixed };
}

bool voxelsApplyTransform( ObjectVoxels& obj, const AffineXf3f& xf, bool fixBox )
{
    auto [vol, fix] = transformVdbVolume( obj.vdbVolume(), xf, fixBox, obj.getBoundingBox() );
    obj.updateVdbVolume( vol );
    obj.updateHistogramAndSurface();
    return fix;
}


}
