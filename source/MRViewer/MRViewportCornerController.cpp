#include "MRViewportCornerController.h"
#include "MRMesh/MRMesh.h"
#include "MRMesh/MRMeshBuilder.h"
#include "MRMesh/MRColor.h"
#include "MRMesh/MRVector.h"
#include "MRColorTheme.h"
#include "MRViewer.h"
#include "MRViewport.h"
#include "MRMesh/MRSystemPath.h"
#include "MRMesh/MRMeshTexture.h"
#include "MRMesh/MRVector.h"
#include "MRMesh/MRImageLoad.h"

namespace MR
{

Mesh makeCornerControllerMesh( float size, float cornerRatio /*= 0.15f */ )
{
    Mesh outMesh;
    outMesh.points.resize( 4 * 6 + 2 * 12 * 2 + 8 * 3 ); // 4x6 - verts on each side, 2x12 - verts on 2-rank corners (x2 to have disconnected edges), 8 - verts on 3-rank corners (x3 to have disconnected corners)
    Triangulation t;
    t.resize( 2 * 6 + 4 * 12 + 6 * 8 ); // 2x6 - faces on each side, 4x12 - faces on 2-rank corners, 6x8 - faces on 3-rank corners

    // 4x6 - verts on each side
    for ( int i = 0; i < 6; ++i )
    {
        int axis = i / 2;
        float sign = ( i % 2 ) * 2.0f - 1.0f;

        int ax2 = ( axis + 1 ) % 3;
        int ax3 = ( axis + 2 ) % 3;
        if ( sign > 0 )
            std::swap( ax2, ax3 );
        for ( int j = 0; j < 4; ++j )
        {
            auto& pt = outMesh.points[VertId( i * 4 + j )];
            pt[axis] = sign * 0.5f * size;
            pt[ax2] = ( ( j / 2 ) * 2.0f - 1.0f ) * ( ( 0.5f - cornerRatio ) * size );
            pt[ax3] = ( ( j % 2 ) * 2.0f - 1.0f ) * ( ( 0.5f - cornerRatio ) * size );
        }
        // add faces
        t[FaceId( i * 2 )] = { VertId( i * 4 ),VertId( i * 4 + 1 ),VertId( i * 4 + 3 ) };
        t[FaceId( i * 2 + 1 )] = { VertId( i * 4 ),VertId( i * 4 + 3 ),VertId( i * 4 + 2 ) };
    }

    int vOffset = 4 * 6;
    int fOffset = 2 * 6;
    // 2x12 - verts on 2-rank corners (x2 to have disconnected edges)
    for ( int i = 0; i < 12; ++i )
    {
        int baseSide = i / 4; // 0 - (xy,x-y,-xy,-x-y), 1 - (yz,y-z,-yz,-y-z), 2 - (zx,z-x,-zx,-z-x)
        int baseSide2 = ( baseSide + 1 ) % 3;
        int signsId = i % 4;
        int signsId1 = signsId / 2;
        int signsId2 = signsId % 2;
        float sign = signsId1 * 2.0f - 1.0f;
        float sign2 = signsId2 * 2.0f - 1.0f;
        for ( int k = 0; k < 2; ++k )
        {
            for ( int j = 0; j < 2; ++j )
            {
                auto& pt = outMesh.points[VertId( vOffset + i * 4 + k * 2 + j )];
                pt[baseSide] = sign * 0.5f * size;
                pt[baseSide2] = sign2 * 0.5f * size;
                pt[( baseSide2 + 1 ) % 3] = ( ( j % 2 ) * 2.0f - 1.0f ) * ( ( 0.5f - cornerRatio ) * size );
            }
        }
        // add faces
        for ( int j = 0; j < 2; ++j )
        {
            auto faceBaseSide = j == 0 ? baseSide : baseSide2;
            auto faceSign = j == 0 ? signsId1 : signsId2;
            auto otherSign = j == 0 ? signsId2 : signsId1;

            VertId baseVert = VertId( faceBaseSide * 8 + faceSign * 4 );
            VertId v0, v1;

            bool inversed = false;
            if ( j == 0 )
            {
                inversed = faceSign != otherSign;
                v0 = baseVert + ( ( 3 * otherSign + 2 * faceSign ) % 4 );
                v1 = baseVert + 1 + 1 * otherSign + ( inversed ? -1 : 1 ) * faceSign;
            }
            else
            {
                inversed = faceSign == otherSign;
                v0 = baseVert + ( inversed ? -1 : 1 ) * faceSign + 3 * otherSign;
                v1 = baseVert + ( ( 2 + 2 * faceSign + 3 * otherSign ) % 4 );
            }
            if ( j != 0 )
                std::swap( v0, v1 );
            VertId edgeVert0 = VertId( vOffset + i * 4 + j * 2 );
            VertId edgeVert1 = edgeVert0 + 1;
            t[FaceId( fOffset + i * 4 + j * 2 + 0 )] = { edgeVert0, v1, v0 };
            t[FaceId( fOffset + i * 4 + j * 2 + 1 )] = { edgeVert0,inversed ? v0 : edgeVert1,inversed ? edgeVert1 : v1 };
        }
    }

    int vOffset2 = 12 * 2 * 2;
    int fOffset2 = 12 * 4;
    // 8 - verts on 3-rank corners (x3 to have disconnected corners)
    for ( int i = 0; i < 8; ++i )
    {
        Vector3i sign = Vector3i();
        for ( int j = 0; j < 3; ++j )
            sign[j] = bool( i & ( 1 << j ) ) ? 1 : 0;

        auto centerV = VertId( vOffset + vOffset2 + i * 3 );
        for ( int k = 0; k < 3; ++k )
        {
            auto& pt = outMesh.points[centerV + k];
            for ( int j = 0; j < 3; ++j )
                pt[j] = ( sign[j] * 2.0f - 1.0f ) * 0.5f * size;
        }
        // add faces
        std::array<VertId, 9> ringVerts;
        for ( int j = 0; j < 9; ++j )
        {
            int mainAxis = j / 3;
            int nextAxis = ( mainAxis + 1 ) % 3;
            int otherAxis = ( mainAxis + 2 ) % 3;
            if ( j % 3 == 0 )
            {
                // lower corner vert
                ringVerts[j] = VertId( vOffset + 2 * ( otherAxis * 8 + sign[otherAxis] * 4 + sign[mainAxis] * 2 + 1 ) + sign[nextAxis] );
            }
            else if ( j % 3 == 1 )
            {
                // inner vert
                ringVerts[j] = VertId( mainAxis * 8 + sign[mainAxis] * 4 );
                ringVerts[j] += ( sign[mainAxis] == 0 ? 2 : 1 ) * sign[nextAxis] + ( sign[mainAxis] == 0 ? 1 : 2 ) * sign[otherAxis];
            }
            else
            {
                // upper corner vert
                ringVerts[j] = VertId( vOffset + 2 * ( mainAxis * 8 + sign[mainAxis] * 4 + sign[nextAxis] * 2 ) + sign[otherAxis] );
            }
        }
        for ( int j = 0; j < 6; ++j )
        {
            int ind = ( j / 2 ) * 3 + ( j % 2 );
            VertId nextV = ringVerts[ind];
            VertId next2V = ringVerts[ind + 1];
            if ( ( sign[0] + sign[1] + sign[2] ) % 2 == 0 )
                std::swap( nextV, next2V );
            t[FaceId( fOffset + fOffset2 + i * 6 + j )] = { VertId( centerV + j / 2 ), nextV, next2V };
        }
    }

    outMesh.topology = MeshBuilder::fromTriangles( t );
    return outMesh;
}

VertUVCoords makeCornerControllerUVCoords( float cornerRatio /*= 0.2f */ )
{
    VertUVCoords uvs( 4 * 6 + 2 * 12 * 2 + 8 * 3 ); // 4x6 - verts on each side, 2x12 - verts on 2-rank corners (x2 to have disconnected edges), 8 - verts on 3-rank corners (x3 to have disconnected corners);
    VertId vOffset = VertId( 4 * 6 );
    VertId vOffset2 = VertId( 2 * 12 * 2 );

    constexpr std::array<UVCoord, 4 * 6> cBaseUvs
    {
        UVCoord( 1.0f,2.f / 3.f ),UVCoord( 1.0f,1.0f ),UVCoord( 0.5f,2.f / 3.f ),UVCoord( 0.5f,1.0f ),//-+ -- ++ +-
        UVCoord( 0.0f,2.f / 3.f ),UVCoord( 0.5f,2.f / 3.f ),UVCoord( 0.0f,1.0f ),UVCoord( 0.5f,1.0f ),//++ -+ +- --
        
        UVCoord( 0.0f,0.0f ),UVCoord( 0.5f,0.0f ),UVCoord( 0.0f,1.f / 3.f ),UVCoord( 0.5f,1.f / 3.f ),//++ -+ +- --
        UVCoord( 1.0f,0.0f ),UVCoord( 1.0f,1.f / 3.f ),UVCoord( 0.5f,0.0f ),UVCoord( 0.5f,1.f / 3.f ),//-+ -- ++ +-

        UVCoord( 1.0f,1.f / 3.f ),UVCoord( 1.0f,2.f / 3.f ),UVCoord( 0.5f,1.f / 3.f ),UVCoord( 0.5f,2.f / 3.f ),//-+ -- ++ +-
        UVCoord( 0.0f,1.f / 3.f ),UVCoord( 0.5f,1.f / 3.f ),UVCoord( 0.0f,2.f / 3.f ),UVCoord( 0.5f,2.f / 3.f ),//++ -+ +- --
    };

    constexpr std::array<bool, 2 * 4 * 6> cBaseSign
    {
        false,true,  false,false,  true,true,   true,false, //-+ -- ++ +-
        true,true,   false,true,   true,false,  false,false,//++ -+ +- --

        true,true,   false,true,   true,false,  false,false,//++ -+ +- --
        false,true,  false,false,  true,true,   true,false, //-+ -- ++ +-

        false,true,  false,false,  true,true,   true,false, //-+ -- ++ +-
        true,true,   false,true,   true,false,  false,false,//++ -+ +- --
    };

    // 4x6 - verts on each side
    for ( int i = 0; i < 6; ++i )
    {
        const UVCoord* cornerUVs = &cBaseUvs[i * 4];

        for ( int j = 0; j < 4; ++j )
        {
            auto& uv = uvs[VertId( i * 4 + j )];
            uv = cornerUVs[j];

            auto sign0 = ( cBaseSign[i * 8 + j * 2] ? 1.0f : -1.0f );
            auto sign1 = ( cBaseSign[i * 8 + j * 2 + 1] ? 1.0f : -1.0f );
            uv.x += sign0 * 0.5f * cornerRatio;
            uv.y += sign1 * ( 1.f / 3.f ) * cornerRatio;
        }
    }

    // 2x12 - verts on 2-rank corners (x2 to have disconnected edges)
    for ( int i = 0; i < 12; ++i )
    {
        int baseSide = i / 4; // 0 - (xy,x-y,-xy,-x-y), 1 - (yz,y-z,-yz,-y-z), 2 - (zx,z-x,-zx,-z-x)
        int baseSide2 = ( baseSide + 1 ) % 3;
        int signsId = i % 4;
        int signsId1 = signsId / 2;
        int signsId2 = signsId % 2;
        // add faces
        for ( int j = 0; j < 2; ++j )
        {
            auto faceBaseSide = j == 0 ? baseSide : baseSide2;
            auto faceSign = j == 0 ? signsId1 : signsId2;
            auto otherSign = j == 0 ? signsId2 : signsId1;

            VertId baseVert = VertId( faceBaseSide * 8 + faceSign * 4 );
            VertId v0, v1;

            bool inversed = false;
            if ( j == 0 )
            {
                inversed = faceSign != otherSign;
                v0 = baseVert + ( ( 3 * otherSign + 2 * faceSign ) % 4 );
                v1 = baseVert + 1 + 1 * otherSign + ( inversed ? -1 : 1 ) * faceSign;
            }
            else
            {
                inversed = faceSign == otherSign;
                v0 = baseVert + ( inversed ? -1 : 1 ) * faceSign + 3 * otherSign;
                v1 = baseVert + ( ( 2 + 2 * faceSign + 3 * otherSign ) % 4 );
            }
            if ( signsId1 != signsId2 )
                std::swap( v0, v1 );
            VertId edgeVert0 = VertId( vOffset + i * 4 + j * 2 );
            VertId edgeVert1 = edgeVert0 + 1;
            uvs[edgeVert0] = cBaseUvs[v0];
            uvs[edgeVert1] = cBaseUvs[v1];

            auto updateCoord = ( baseSide == 0 || ( baseSide == 2 && j == 0 ) ) ? 1 : 0;
            uvs[edgeVert0][updateCoord] += ( cBaseSign[2 * v0 + int( updateCoord )] ? 1.0f : -1.0f ) * ( updateCoord == 0 ? 0.5f : 1.f / 3.f ) * cornerRatio;
            uvs[edgeVert1][updateCoord] += ( cBaseSign[2 * v1 + int( updateCoord )] ? 1.0f : -1.0f ) * ( updateCoord == 0 ? 0.5f : 1.f / 3.f ) * cornerRatio;
        }
    }

    // 8 - verts on 3-rank corners (x3 to have disconnected corners)
    for ( int i = 0; i < 8; ++i )
    {
        Vector3i sign = Vector3i();
        auto centerV = VertId( vOffset + vOffset2 + i * 3 );
        for ( int j = 0; j < 3; ++j )
            sign[j] = bool( i & ( 1 << j ) ) ? 1 : 0;
        for ( int j = 0; j < 3; ++j )
        {
            int mainAxis = j;
            int nextAxis = ( mainAxis + 1 ) % 3;
            int otherAxis = ( mainAxis + 2 ) % 3;
            //if ( sign[mainAxis] > 0 )
            //    std::swap( nextAxis, otherAxis );

            auto innerV = VertId( mainAxis * 8 + sign[mainAxis] * 4 );
            innerV += ( sign[mainAxis] == 0 ? 2 : 1 ) * sign[nextAxis] + ( sign[mainAxis] == 0 ? 1 : 2 ) * sign[otherAxis];
            uvs[centerV + j] = cBaseUvs[innerV];
        }
    }

    return uvs;
}

Vector<MR::MeshTexture, TextureId> loadCornerControllerTextures()
{
    Vector<MR::MeshTexture, TextureId> res;
    res.resize( TextureId( 3 ) );

    auto path = SystemPath::getResourcesDirectory() / "resource" / "textures";
    const std::array<std::filesystem::path, 3> cTexturePaths = {
        path / "controller_cube_default.png",
        path / "controller_cube_sides.png",
        path / "controller_cube_edges.png"
    };
    for ( int i = 0; i < 3; ++i )
    {
        auto loaded = ImageLoad::fromAnySupportedFormat( cTexturePaths[i] );
        if ( !loaded.has_value() )
            return {};
        res[TextureId( i )].pixels = std::move( loaded->pixels );
        res[TextureId( i )].resolution = loaded->resolution;
        res[TextureId( i )].filter = FilterType::Linear;
    }
    return res;
}

const TexturePerFace& getCornerControllerTexureMap()
{
    static TexturePerFace textures;
    if ( textures.empty() )
    {
        const int f2RankOffset = 2 * 6;
        const int f2RankSize = 12 * 4;
        const int f3RankOffset = f2RankOffset + f2RankSize;

        textures.resize( f3RankOffset + 8 * 6, TextureId( 0 ) );
    }
    return textures;
}

RegionId getCornerControllerRegionByFace( FaceId face )
{
    static Face2RegionMap map;
    if ( map.empty() )
    {
        RegionId currentRegion;
        map.resize( 2 * 6 + 4 * 12 + 6 * 8 );
        for ( int i = 0; i < 2 * 6; ++i )
        {
            if ( i % 2 == 0 )
                ++currentRegion;
            map[FaceId( i )] = currentRegion;
        }
        auto f2RankOffset = 2 * 6;
        for ( int i = 0; i < 4 * 12; ++i )
        {
            if ( i % 4 == 0 )
                ++currentRegion;
            map[FaceId( f2RankOffset + i )] = currentRegion;
        }
        auto f3RankOffset = f2RankOffset + 4 * 12;
        for ( int i = 0; i < 6 * 8; ++i )
        {
            if ( i % 6 == 0 )
                ++currentRegion;
            map[FaceId( f3RankOffset + i )] = currentRegion;
        }
    }    
    return map[face];
}

TexturePerFace getCornerControllerHoveredTextureMap( RegionId rId )
{
    auto textures = getCornerControllerTexureMap();
    const int fOffset = 2 * 6;
    const int fOffset2 = 4 * 12;
    if ( rId < 6 )
    {
        FaceId shift = FaceId( rId * 2 );
        for ( FaceId f( shift ); f < shift + 2; ++f )
            textures[f] = TextureId( 1 );

        // hover edges
        int baseInd = rId / 2;
        int otherInd = ( baseInd + 2 ) % 3;
        textures[FaceId( fOffset + baseInd * 16 + ( rId % 2 ) * 8 + 0 )] = TextureId( 1 );
        textures[FaceId( fOffset + baseInd * 16 + ( rId % 2 ) * 8 + 1 )] = TextureId( 1 );
        textures[FaceId( fOffset + baseInd * 16 + ( rId % 2 ) * 8 + 4 )] = TextureId( 1 );
        textures[FaceId( fOffset + baseInd * 16 + ( rId % 2 ) * 8 + 5 )] = TextureId( 1 );
        textures[FaceId( fOffset + otherInd * 16 + 0 + ( rId % 2 ) * 4 + 2 )] = TextureId( 1 );
        textures[FaceId( fOffset + otherInd * 16 + 0 + ( rId % 2 ) * 4 + 3 )] = TextureId( 1 );
        textures[FaceId( fOffset + otherInd * 16 + 8 + ( rId % 2 ) * 4 + 2 )] = TextureId( 1 );
        textures[FaceId( fOffset + otherInd * 16 + 8 + ( rId % 2 ) * 4 + 3 )] = TextureId( 1 );

        // hover corners
        for ( int i = 0; i < 8; ++i )
        {
            Vector3i sign = Vector3i();
            for ( int j = 0; j < 3; ++j )
                sign[j] = bool( i & ( 1 << j ) ) ? 1 : 0;

            if ( sign[baseInd] != ( rId % 2 ) )
                continue;

            textures[FaceId( fOffset + fOffset2 + i * 6 + baseInd * 2 + 0 )] = TextureId( 1 );
            textures[FaceId( fOffset + fOffset2 + i * 6 + baseInd * 2 + 1 )] = TextureId( 1 );
        }
    }
    else if ( rId < 6 + 12 )
    {
        FaceId shift = FaceId( fOffset + ( rId - 6 ) * 4 );
        for ( FaceId f( shift ); f < shift + 4; ++f )
            textures[f] = TextureId( 2 );
    }
    else
    {
        FaceId shift = FaceId( fOffset + fOffset2 + ( rId - 6 - 12 ) * 6 );
        for ( FaceId f( shift ); f < shift + 6; ++f )
            textures[f] = TextureId( 2 );
    }
    return textures;
}

void updateCurrentViewByControllerRegion( RegionId rId )
{
    Viewport& vp = getViewerInstance().viewport();
    switch ( int( rId ) )
    {
    // sides
    case 0: //from  -x
        vp.cameraLookAlong( { 1,0,0 }, { 0,0,1 } );
        break;
    case 1: //from  x
        vp.cameraLookAlong( { -1,0,0 }, { 0,0,1 } );
        break;
    case 2: //from  -y
        vp.cameraLookAlong( { 0,1,0 }, { 0,0,1 } );
        break;
    case 3: //from  y
        vp.cameraLookAlong( { 0,-1,0 }, { 0,0,1 } );
        break;
    case 4: //from  -z
        vp.cameraLookAlong( { 0,0,1 }, { 0,1,0 } );
        break;
    case 5: //from  z
        vp.cameraLookAlong( { 0,0,-1 }, { 0,1,0 } );
        break;

    // 2 rank corners
    case 6 + 0: //from  -x -y
        vp.cameraLookAlong( { 1,1,0 }, { 0,0,1 } );
        break;
    case 6 + 1: //from  -x y
        vp.cameraLookAlong( { 1,-1,0 }, { 0,0,1 } );
        break;
    case 6 + 2: //from  x -y
        vp.cameraLookAlong( { -1,1,0 }, { 0,0,1 } );
        break;
    case 6 + 3: //from  x y
        vp.cameraLookAlong( { -1,-1,0 }, { 0,0,1 } );
        break;
    case 6 + 4: //from  -y -z
        vp.cameraLookAlong( { 0,1,1 }, { 0,-1,1 } );
        break;
    case 6 + 5: //from  -y z
        vp.cameraLookAlong( { 0,1,-1 }, { 0,1,1 } );
        break;
    case 6 + 6: //from  y -z
        vp.cameraLookAlong( { 0,-1,1 }, { 0,1,1 } );
        break;
    case 6 + 7: //from  y z
        vp.cameraLookAlong( { 0,-1,-1 }, { 0,-1,1 } );
        break;
    case 6 + 8: //from  -z -x
        vp.cameraLookAlong( { 1,0,1 }, { -1,0,1 } );
        break;
    case 6 + 9: //from  -z x
        vp.cameraLookAlong( { -1,0,1 }, { 1,0,1 } );
        break;
    case 6 + 10: //from  z -x
        vp.cameraLookAlong( { 1,0,-1 }, { 1,0,1 } );
        break;
    case 6 + 11: //from  z x
        vp.cameraLookAlong( { -1,0,-1 }, { -1,0,1 } );
        break;

    // 3 rank corners
    case 6 + 12 + 0: //from -x -y -z
        vp.cameraLookAlong( { 1,1,1 }, { -1,-1,2 } );
        break;
    case 6 + 12 + 1: //from x -y -z
        vp.cameraLookAlong( { -1,1,1 }, { 1,-1,2 } );
        break;
    case 6 + 12 + 2: //from -x y -z
        vp.cameraLookAlong( { 1,-1,1 }, { -1,1,2 } );
        break;
    case 6 + 12 + 3: //from x y -z
        vp.cameraLookAlong( { -1,-1,1 }, { 1,1,2 } );
        break;
    case 6 + 12 + 4: //from -x -y z
        vp.cameraLookAlong( { 1,1,-1 }, { 1,1,2 } );
        break;
    case 6 + 12 + 5: //from x -y z
        vp.cameraLookAlong( { -1,1,-1 }, { -1,1,2 } );
        break;
    case 6 + 12 + 6: //from -x y z
        vp.cameraLookAlong( { 1,-1,-1 }, { 1,-1,2 } );
        break;
    case 6 + 12 + 7: //from x y z
        vp.cameraLookAlong( { -1,-1,-1 }, { -1,-1,2 } );
        break;

    default:
        return;
    }

    vp.preciseFitDataToScreenBorder( { 0.9f } );
}

}