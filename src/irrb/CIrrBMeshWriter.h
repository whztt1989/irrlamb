//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#ifndef __IRR_IRRB_MESH_WRITER_H_INCLUDED__
#define __IRR_IRRB_MESH_WRITER_H_INCLUDED__

#include "IMeshWriter.h"
#include "S3DVertex.h"
#include "IVideoDriver.h"
#include "IFileSystem.h"

namespace irr
{
    namespace io
    {
        class IXMLWriter;
    }
    namespace scene
    {

        /* v 1.6

        [header]
        [animated mesh]
        [mesh]
           [mesh info]
               [buffer count]
               [verts]        (all mesh buffers)
               [indices]      (all mesh buffers)
           [/mesh info]

           [mesh buffer]
              [meshbuffer info] (vertex/index offsets/counts)
              [material]
                 [common mat attributes]
                 [layer count]
                    [layer]
                       [texture name string chunk]
                       [layer attributes]
                    [/layer]
           [/mesh buffer]

           [mesh buffer]
              ...
           [/mesh buffer]
        [/mesh]

        [mesh]
        ...
        [/mesh]

        */

        class IMeshBuffer;

        // byte-align structures
        #if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
        #	pragma pack( push, packing )
        #	pragma pack( 1 )
        #	define PACK_STRUCT
        #elif defined( __GNUC__ )
        #	define PACK_STRUCT	__attribute__((packed))
        #else
        #	define PACK_STRUCT
        #endif

        #define IRRB_VERSION    IRRLICHT_VERSION_MAJOR << 8 & IRRLICHT_VERSION_MINOR  // coincides with Irrlicht version

        #define CHUNK_ID(c0, c1, c2, c3) \
		((u32)(u8)(c0) | ((u32)(u8)(c1) << 8) | \
		((u32)(u8)(c2) << 16) | ((u32)(u8)(c3) << 24 ))

        #define INFO_ANIMATION_SKINNED  0x0001
        #define INFO_ANIMATION_VERTEX   0x0002

        #define CID_MESH     CHUNK_ID('m','e','s','h')
        #define CID_MATERIAL CHUNK_ID('m','a','t',' ')
        #define CID_MESHBUF  CHUNK_ID('m','b','u','f')
        #define CID_VBUFFER  CHUNK_ID('v','b','u','f')
        #define CID_IBUFFER  CHUNK_ID('i','b','u','f')
        #define CID_TEXTURE  CHUNK_ID('t','e','x',' ')
        #define CID_SKEL     CHUNK_ID('s','k','e','l')
        #define CID_MORPH    CHUNK_ID('m','r','p','h')
        #define CID_STRING   CHUNK_ID('s','t','r',' ')

        // irrb header
        struct IrrbHeader
        {
            c8      hSig[12];   // 'irrb vh.vl' eof
            u32     hSigCheck;
            u16     hVersion;
            u16     hFill1;
            u32     hFlags;     // compression/encryption/endianess/int bits
            c8      hCreator[32];
            u32     hMeshCount;
            u32     hMeshBufferCount;
            u32     hCRC;
        } PACK_STRUCT;

        struct IrrbChunkInfo
        {
            u32     iId;
            u32     iSize;
        } PACK_STRUCT;

        struct Irrb3f
        {
            f32     x;
            f32     y;
            f32     z;
        } PACK_STRUCT;

        struct Irrb2f
        {
            f32     x;
            f32     y;
        } PACK_STRUCT;

        struct IrrbMeshInfo
        {
            u32     iMeshBufferCount;
            u32     iVertexCount;
            u32     iIndexCount;
            u32     iMaterialCount;
            Irrb3f  ibbMin;
            Irrb3f  ibbMax;
        } PACK_STRUCT;

        struct IrrbVertex
        {
            struct Irrb3f   vPos;
            struct Irrb3f   vNormal;
            u32             vColor;
            struct Irrb2f   vUV1;
            struct Irrb2f   vUV2;
            struct Irrb3f   vTangent;
            struct Irrb3f   vBiNormal;
        } PACK_STRUCT;

        struct IrrbMaterialLayer
        {
            u8      mTextureWrapU:4;
            u8      mTextureWrapV:4;
            f32     mMatrix[16];
            u8      mAnisotropicFilter;
            u8      mLODBias;
            bool    mBilinearFilter:1;
            bool    mTrilinearFilter:1;
        } PACK_STRUCT;

        struct IrrbMaterial
        {
            u32     mType;
            u32     mAmbient;
            u32     mDiffuse;
            u32     mEmissive;
            u32     mSpecular;
            f32     mShininess;
            f32     mParm1;
            f32     mParm2;
            f32     mThickness;
            u8      mZBuffer;
            u8      mAntiAliasing;
            u8      mColorMask;
            u8      mColorMaterial;
		    u8      mBlendOperation;
		    u8      mPolygonOffsetFactor;
    		u8      mPolygonOffsetDirection;
            u8      mLayerCount;
            bool    mWireframe:1;
            bool    mPointCloud:1;
            bool    mGrouraudShading:1;
            bool    mLighting:1;
            bool    mZWriteEnabled:1;
            bool    mBackfaceCulling:1;
            bool    mFrontfaceCulling:1;
            bool    mFogEnable:1;
            bool    mNormalizeNormals:1;
            bool    mUseMipMaps:1;
        } PACK_STRUCT;

        struct IrrbMeshBufInfo
        {
            u32     iVertexType;
            u32     iVertCount;
            u32     iVertStart;
            u32     iIndexCount;
            u32     iIndexStart;
            u32     iFaceCount;
            u32     iMaterialIndex;
            char    iMaterialName[64];
            Irrb3f  ibbMin;
            Irrb3f  ibbMax;
        } PACK_STRUCT;

        struct SCustomMaterial
        {
            core::stringc Name;
            video::E_MATERIAL_TYPE  Type;
            video::IMaterialRenderer* Renderer;
        };

        // Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT


        //! class to write meshes, implementing a IrrMesh (.irrmesh, .xml) writer
        /** This writer implementation has been originally developed for irrEdit and then
        merged out to the Irrlicht Engine */
        class CIrrBMeshWriter : public IMeshWriter
        {
        public:

            CIrrBMeshWriter(video::IVideoDriver* driver, io::IFileSystem* fs, core::array<SCustomMaterial>* customMaterials=0);
            virtual ~CIrrBMeshWriter();

            //! Returns the type of the mesh writer
            virtual EMESH_WRITER_TYPE getType() const;

            //! writes a mesh
            virtual bool writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags=EMWF_NONE);

            void setVersion(u16 value)
            {
                Version = value;
                VMajor = (Version & 0xFF00) >> 8;
                VMinor = Version & 0x00FF;
            }
            void setCreator(irr::core::stringc value) {Creator = value;}

            void setRelativeBase(irr::core::stringc value) {RelativeBase = value;}

        protected:

            void writeHeader(const scene::IMesh* mesh);

            void updateMaterial(const video::SMaterial& material,struct IrrbMaterial& mat);
            void updateMaterialLayer(const video::SMaterial& material,u8 layerNumber, irr::core::stringc& textureName, struct IrrbMaterialLayer& layer);

            bool _writeMesh(const scene::IMesh* mesh);

            u32 _writeChunkInfo(u32 id, u32 size);
            void _writeStringChunk(irr::core::stringc value);
            void _updateChunkSize(u32 id, u32 offset);
            void updateBuffers(const scene::IMesh* mesh, struct IrrbVertex* vbuffer, u32* ibuffer);

            bool addMaterial(irr::video::SMaterial& material);
            u32 getMaterialIndex(irr::video::SMaterial& material);
            irr::core::stringc getMaterialName(irr::video::SMaterial& material);

            // member variables:
            irr::core::array<irr::video::SMaterial> Materials;
            struct IrrbVertex*   VBuffer;
            u32*    IBuffer;
            io::IFileSystem* FileSystem;
            video::IVideoDriver* VideoDriver;
            core::array<SCustomMaterial>* CustomMaterials;
            io::IWriteFile* Writer;
            u16 Version, VMajor, VMinor;
            irr::core::stringc Creator;
            irr::core::stringc RelativeBase;
        };

    int get_endianess(void);  // 1-big, 0-lil

    } // end namespace
} // end namespace

#endif
