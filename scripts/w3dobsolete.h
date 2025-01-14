#pragma once
enum ChunkType {

	W3D_CHUNK_MESH = 0x00000000,        // Mesh definition
	W3D_CHUNK_MESH_HEADER = 0x00000001,        // @
	W3D_CHUNK_VERTICES = 0x00000002,        // array of vertices (array of W3dVectorStruct's)
	W3D_CHUNK_VERTEX_NORMALS = 0x00000003,        // array of normals (array of W3dVectorStruct's)
	W3D_CHUNK_SURRENDER_NORMALS = 0x00000004,        // @ array of surrender normals (one per vertex as req. by surrender)
	W3D_CHUNK_TEXCOORDS = 0x00000005,        // @ array of texture coordinates
	O_W3D_CHUNK_MATERIALS = 0x00000006,        // @ array of materials
	O_W3D_CHUNK_TRIANGLES = 0x00000007,        // @ array of triangles
	O_W3D_CHUNK_QUADRANGLES = 0x00000008,        // @
	O_W3D_CHUNK_SURRENDER_TRIANGLES = 0x00000009,        // @ array of surrender format tris
	O_W3D_CHUNK_POV_TRIANGLES = 0x0000000A,        // @
	O_W3D_CHUNK_POV_QUADRANGLES = 0x0000000B,        // @
	W3D_CHUNK_MESH_USER_TEXT = 0x0000000C,        // Text from the MAX comment field (Null terminated string)
	W3D_CHUNK_VERTEX_COLORS = 0x0000000D,        // @
	W3D_CHUNK_VERTEX_INFLUENCES = 0x0000000E,        // Mesh Deformation vertex connections (array of W3dVertInfStruct's)

	W3D_CHUNK_DAMAGE = 0x0000000F,        // @ This chunk is used extensively in Earth & Beyond
	W3D_CHUNK_DAMAGE_HEADER,
	W3D_CHUNK_DAMAGE_VERTICES,
	W3D_CHUNK_DAMAGE_COLORS,
	W3D_CHUNK_DAMAGE_MATERIALS,

	O_W3D_CHUNK_MATERIALS2 = 0x00000014,        // @

	W3D_CHUNK_MATERIALS3 = 0x00000015,        // @
	W3D_CHUNK_MATERIAL3,
	W3D_CHUNK_MATERIAL3_NAME,
	W3D_CHUNK_MATERIAL3_INFO,
	W3D_CHUNK_MATERIAL3_DC_MAP = 0x00000019,        // @
	W3D_CHUNK_MAP3_FILENAME,
	W3D_CHUNK_MAP3_INFO,
	W3D_CHUNK_MATERIAL3_DI_MAP,
	W3D_CHUNK_MATERIAL3_SC_MAP,
	W3D_CHUNK_MATERIAL3_SI_MAP,

	W3D_CHUNK_MESH_HEADER3 = 0x0000001F,        //  mesh header contains general info about the mesh. (W3dMeshHeader3Struct)
	W3D_CHUNK_TRIANGLES = 0x00000020,        // New improved triangles chunk (array of W3dTriangleStruct's)
	W3D_CHUNK_PER_TRI_MATERIALS = 0x00000021,        // @
	W3D_CHUNK_VERTEX_SHADE_INDICES = 0x00000022,        // shade indexes for each vertex (array of uint32's)

	W3D_CHUNK_PRELIT_UNLIT = 0x00000023,        // optional unlit material chunk wrapper
	W3D_CHUNK_PRELIT_VERTEX = 0x00000024,        // optional vertex-lit material chunk wrapper
	W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS = 0x00000025,        // optional lightmapped multi-pass material chunk wrapper
	W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE = 0x00000026,        // optional lightmapped multi-texture material chunk wrapper

	W3D_CHUNK_MATERIAL_INFO = 0x00000028,        // materials information, pass count, etc (contains W3dMaterialInfoStruct)

	W3D_CHUNK_SHADERS = 0x00000029,        // shaders (array of W3dShaderStruct's)

	W3D_CHUNK_VERTEX_MATERIALS = 0x0000002A,        // wraps the vertex materials
	W3D_CHUNK_VERTEX_MATERIAL = 0x0000002B,
	W3D_CHUNK_VERTEX_MATERIAL_NAME = 0x0000002C,        // vertex material name (NULL-terminated string)
	W3D_CHUNK_VERTEX_MATERIAL_INFO = 0x0000002D,        // W3dVertexMaterialStruct
	W3D_CHUNK_VERTEX_MAPPER_ARGS0 = 0x0000002E,        // Null-terminated string
	W3D_CHUNK_VERTEX_MAPPER_ARGS1 = 0x0000002F,        // Null-terminated string

	W3D_CHUNK_TEXTURES = 0x00000030,        // wraps all of the texture info
	W3D_CHUNK_TEXTURE = 0x00000031,        // wraps a texture definition
	W3D_CHUNK_TEXTURE_NAME = 0x00000032,        // texture filename (NULL-terminated string)
	W3D_CHUNK_TEXTURE_INFO = 0x00000033,        // optional W3dTextureInfoStruct

	W3D_CHUNK_MATERIAL_PASS = 0x00000038,        // wraps the information for a single material pass
	W3D_CHUNK_VERTEX_MATERIAL_IDS = 0x00000039,        // single or per-vertex array of uint32 vertex material indices (check chunk size)
	W3D_CHUNK_SHADER_IDS = 0x0000003A,        // single or per-tri array of uint32 shader indices (check chunk size)
	W3D_CHUNK_DCG = 0x0000003B,        // per-vertex diffuse color values (array of W3dRGBAStruct's)
	W3D_CHUNK_DIG = 0x0000003C,        // per-vertex diffuse illumination values (array of W3dRGBStruct's)
	W3D_CHUNK_SCG = 0x0000003E,        // per-vertex specular color values (array of W3dRGBStruct's)
	W3D_CHUNK_FXSHADER_IDS = 0x0000003F,        // single or per-tri array of uint32 fx shader indices (check chunk size)

	W3D_CHUNK_TEXTURE_STAGE = 0x00000048,        // wrapper around a texture stage.
	W3D_CHUNK_TEXTURE_IDS = 0x00000049,        // single or per-tri array of uint32 texture indices (check chunk size)
	W3D_CHUNK_STAGE_TEXCOORDS = 0x0000004A,        // per-vertex texture coordinates (array of W3dTexCoordStruct's)
	W3D_CHUNK_PER_FACE_TEXCOORD_IDS = 0x0000004B,        // indices to W3D_CHUNK_STAGE_TEXCOORDS, (array of Vector3i)

	W3D_CHUNK_FX_SHADERS = 0x00000050,        //@ W3D_CHUNK_FX_SHADERS APPEARS FIRST IN BFME2 SEEMS
	W3D_CHUNK_FX_SHADER,
	W3D_CHUNK_FX_SHADER_INFO,
	W3D_CHUNK_FX_SHADER_CONSTANT,

	W3D_CHUNK_DEFORM = 0x00000058,        // mesh deform or 'damage' information.
	W3D_CHUNK_DEFORM_SET = 0x00000059,        // set of deform information
	W3D_CHUNK_DEFORM_KEYFRAME = 0x0000005A,        // a keyframe of deform information in the set
	W3D_CHUNK_DEFORM_DATA = 0x0000005B,        // deform information about a single vertex

	W3D_CHUNK_TANGENTS = 0x00000060,        // @
	W3D_CHUNK_BINORMALS = 0x00000061,        // @ some places on the web its called W3D_CHUNK_BITANGENTS

	W3D_CHUNK_PS2_SHADERS = 0x00000080,        // Shader info specific to the Playstation 2.

	W3D_CHUNK_AABTREE = 0x00000090,        // Axis-Aligned Box Tree for hierarchical polygon culling
	W3D_CHUNK_AABTREE_HEADER,                                       // catalog of the contents of the AABTree
	W3D_CHUNK_AABTREE_POLYINDICES,                                  // array of uint32 polygon indices with count=mesh.PolyCount
	W3D_CHUNK_AABTREE_NODES,                                        // array of W3dMeshAABTreeNode's with count=aabheader.NodeCount

	W3D_CHUNK_HIERARCHY = 0x00000100,        // hierarchy tree definition
	W3D_CHUNK_HIERARCHY_HEADER,
	W3D_CHUNK_PIVOTS,
	W3D_CHUNK_PIVOT_FIXUPS,                                             // only needed by the exporter...
	W3D_CHUNK_PIVOT_UNKNOWN1,                                           // @ appears in ENB

	W3D_CHUNK_ANIMATION = 0x00000200,        // hierarchy animation data
	W3D_CHUNK_ANIMATION_HEADER,
	W3D_CHUNK_ANIMATION_CHANNEL,                                        // channel of vectors
	W3D_CHUNK_BIT_CHANNEL,                                              // channel of boolean values (e.g. visibility)

	W3D_CHUNK_COMPRESSED_ANIMATION = 0x00000280,        // compressed hierarchy animation data
	W3D_CHUNK_COMPRESSED_ANIMATION_HEADER,                              // describes playback rate, number of frames, and type of compression
	W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL,                             // compressed channel, format dependent on type of compression
	W3D_CHUNK_COMPRESSED_BIT_CHANNEL,                                   // compressed bit stream channel, format dependent on type of compression
	W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL,                      // @ appears first in BFME seems

	W3D_CHUNK_MORPH_ANIMATION = 0x000002C0,        // hierarchy morphing animation data (morphs between poses, for facial animation)
	W3D_CHUNK_MORPHANIM_HEADER,                                         // W3dMorphAnimHeaderStruct describes playback rate, number of frames, and type of compression
	W3D_CHUNK_MORPHANIM_CHANNEL,                                        // wrapper for a channel
	W3D_CHUNK_MORPHANIM_POSENAME,                                   // name of the other anim which contains the poses for this morph channel
	W3D_CHUNK_MORPHANIM_KEYDATA,                                    // morph key data for this channel
	W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA,                               // uin32 per pivot in the htree, indicating which channel controls the pivot

	W3D_CHUNK_HMODEL = 0x00000300,        // blueprint for a hierarchy model
	W3D_CHUNK_HMODEL_HEADER,                                            // Header for the hierarchy model
	W3D_CHUNK_NODE,                                                     // render objects connected to the hierarchy
	W3D_CHUNK_COLLISION_NODE,                                           // collision meshes connected to the hierarchy
	W3D_CHUNK_SKIN_NODE,                                                // skins connected to the hierarchy
	OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA,                                 // extension of the hierarchy model header
	OBSOLETE_W3D_CHUNK_SHADOW_NODE,                                     // shadow object connected to the hierarchy

	W3D_CHUNK_LODMODEL = 0x00000400,        // blueprint for an LOD model.  This is simply a
	W3D_CHUNK_LODMODEL_HEADER,                                          // collection of 'n' render objects, ordered in terms
	W3D_CHUNK_LOD,                                                      // of their expected rendering costs. (highest is first)

	W3D_CHUNK_COLLECTION = 0x00000420,        // collection of render object names
	W3D_CHUNK_COLLECTION_HEADER,                                        // general info regarding the collection
	W3D_CHUNK_COLLECTION_OBJ_NAME,                                      // contains a string which is the name of a render object
	W3D_CHUNK_PLACEHOLDER,                                              // contains information about a 'dummy' object that will be instanced later
	W3D_CHUNK_TRANSFORM_NODE,                                           // contains the filename of another w3d file that should be transformed by this node

	W3D_CHUNK_POINTS = 0x00000440,        // array of W3dVectorStruct's.  May appear in meshes, hmodels, lodmodels, or collections.

	W3D_CHUNK_LIGHT = 0x00000460,        // description of a light
	W3D_CHUNK_LIGHT_INFO,                                               // generic light parameters
	W3D_CHUNK_SPOT_LIGHT_INFO,                                          // extra spot light parameters
	W3D_CHUNK_NEAR_ATTENUATION,                                         // optional near attenuation parameters
	W3D_CHUNK_FAR_ATTENUATION,                                          // optional far attenuation parameters
	W3D_CHUNK_SPOT_LIGHT_INFO_5_0,                                      // extra spot light parameters (new in 5.0)

	W3D_CHUNK_EMITTER = 0x00000500,        // description of a particle emitter
	W3D_CHUNK_EMITTER_HEADER,                                           // general information such as name and version
	W3D_CHUNK_EMITTER_USER_DATA,                                        // user-defined data that specific loaders can switch on
	W3D_CHUNK_EMITTER_INFO,                                             // generic particle emitter definition
	W3D_CHUNK_EMITTER_INFOV2,                                           // generic particle emitter definition (version 2.0)
	W3D_CHUNK_EMITTER_PROPS,                                            // Key-frameable properties
	OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME,                          // structure defining a single color keyframe
	OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME,                        // structure defining a single opacity keyframe
	OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME,                           // structure defining a single size keyframe
	W3D_CHUNK_EMITTER_LINE_PROPERTIES,                                  // line properties, used by line rendering mode
	W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES,                               // rotation keys for the particles
	W3D_CHUNK_EMITTER_FRAME_KEYFRAMES,                                  // frame keys (u-v based frame animation)
	W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES,                              // length of tail for line groups
	W3D_CHUNK_EMITTER_EXTRA_INFO,                                       // @ ParticleEmitterDefClass::Save_Extra_Info writes it, ENB has this https://github.com/therealKyp/Earth-and-Beyond-server/blob/master/trunk/Net7Tools/ChunkTypes/ChunkTypes.cpp#L184 //0x50d is in shockfizzle03.w3d

	W3D_CHUNK_AGGREGATE = 0x00000600,        // description of an aggregate object
	W3D_CHUNK_AGGREGATE_HEADER,                                         // general information such as name and version
	W3D_CHUNK_AGGREGATE_INFO,                                       // references to 'contained' models
	W3D_CHUNK_TEXTURE_REPLACER_INFO,                                    // information about which meshes need textures replaced
	W3D_CHUNK_AGGREGATE_CLASS_INFO,                                     // information about the original class that created this aggregate

	W3D_CHUNK_HLOD = 0x00000700,        // description of an HLod object (see HLodClass)
	W3D_CHUNK_HLOD_HEADER,                                              // general information such as name and version
	W3D_CHUNK_HLOD_LOD_ARRAY,                                           // wrapper around the array of objects for each level of detail
	W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER,                         // info on the objects in this level of detail array
	W3D_CHUNK_HLOD_SUB_OBJECT,                                      // an object in this level of detail array
	W3D_CHUNK_HLOD_AGGREGATE_ARRAY,                                     // array of aggregates, contains W3D_CHUNK_SUB_OBJECT_ARRAY_HEADER and W3D_CHUNK_SUB_OBJECT_ARRAY
	W3D_CHUNK_HLOD_PROXY_ARRAY,                                         // array of proxies, used for application-defined purposes, provides a name and a bone.

	W3D_CHUNK_BOX = 0x00000740,        // defines an collision box render object (W3dBoxStruct)
	W3D_CHUNK_SPHERE,
	W3D_CHUNK_RING,

	W3D_CHUNK_NULL_OBJECT = 0x00000750,        // defines a NULL object (W3dNullObjectStruct)

	W3D_CHUNK_LIGHTSCAPE = 0x00000800,        // wrapper for lights created with Lightscape.  
	W3D_CHUNK_LIGHTSCAPE_LIGHT,                                         // definition of a light created with Lightscape.
	W3D_CHUNK_LIGHT_TRANSFORM,                                      // position and orientation (defined as right-handed 4x3 matrix transform W3dLightTransformStruct).

	W3D_CHUNK_DAZZLE = 0x00000900,        // wrapper for a glare object.  Creates halos and flare lines seen around a bright light source
	W3D_CHUNK_DAZZLE_NAME,                                              // null-terminated string, name of the dazzle (typical w3d object naming: "container.object")
	W3D_CHUNK_DAZZLE_TYPENAME,                                          // null-terminated string, type of dazzle (from dazzle.ini)

	W3D_CHUNK_SOUNDROBJ = 0x00000A00,        // description of a sound render object
	W3D_CHUNK_SOUNDROBJ_HEADER,                                         // general information such as name and version
	W3D_CHUNK_SOUNDROBJ_DEFINITION,                                     // chunk containing the definition of the sound that is to play 

	W3D_CHUNK_SHDMESH = 0x00000B00,        // @ ShdMeshClass, appears in Renegade 2 Grizzly, Prism, Rhino, Vulture models, seems to be mesh parts and appears in ENB code
	W3D_CHUNK_SHDMESH_NAME,                                           // @ appears in ENB code
	W3D_CHUNK_SHDMESH_HEADER,
	W3D_CHUNK_SHDMESH_USER_TEXT,

	W3D_CHUNK_SHDSUBMESH = 0x00000B20,        // @ read by ShdSubMeshClass::Load_W3D
	W3D_CHUNK_SHDSUBMESH_HEADER,              // @ read by ShdSubMeshClass::Load_W3D

	W3D_CHUNK_SHDSUBMESH_SHADER = 0x00000B40,                           // @ read by ShdSubMeshClass::Read_B40
	W3D_CHUNK_SHDSUBMESH_SHADER_TYPE,                                   // @ read by ShdSubMeshClass::Read_B40
	W3D_CHUNK_SHDSUBMESH_SHADER_DATA,                                   // @ read by ShdSubMeshClass::Read_B40
	W3D_CHUNK_SHDSUBMESH_VERTICES,                                      // @ read by ShdSubMeshClass::read_vertices
	W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS,                                // @ read by ShdSubMeshClass::read_vertex_normals
	W3D_CHUNK_SHDSUBMESH_TRIANGLES,                                     // @ read by ShdSubMeshClass::read_triangles
	W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES,                          // @ read by ShdSubMeshClass::read_vertex_shade_indices
	W3D_CHUNK_SHDSUBMESH_UV0,                                           // @ read by ShdSubMeshClass::read_uv0
	W3D_CHUNK_SHDSUBMESH_UV1,                                           // @ read by ShdSubMeshClass::read_uv1
	W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S,                               // @ read by ShdSubMeshClass::read_tangent_basis_s
	W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T,                               // @ read by ShdSubMeshClass::read_tangent_basis_t
	W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT,                             // @ read by ShdSubMeshClass::read_tangent_basis_sxt
	W3D_CHUNK_SHDSUBMESH_B4C,                                           // @ 
	W3D_CHUNK_SHDSUBMESH_VERTEX_INFLUENCES,                             // @ read by ShdSubMeshClass::read_vertex_influences

	W3D_CHUNK_SECONDARY_VERTICES = 0x00000C00,        // @ called VERTICES_COPY by the community
	W3D_CHUNK_SECONDARY_VERTEX_NORMALS,                                     // @ called VERTEX_NORMALS_COPY by the community

	W3D_CHUNK_LIGHTMAP_UV = 0x00000C02,        // @
};

struct W3dMeshHeaderStruct
{
	uint32 Version;
	char MeshName[W3D_NAME_LEN];
	uint32 Attributes;
	uint32 NumTris;
	uint32 NumQuads;
	uint32 NumSrTris;
	uint32 field_24;
	uint32 NumPovQuads;
	uint32 NumVertices;
	uint32 NumNormals;
	uint32 NumSrNormals;
	uint32 NumTexCoords;
	uint32 NumMaterials;
	uint32 NumVertColors;
	uint32 NumVertInfluences;
	uint32 NumDamageStages;
	uint32 FutureCounts[5];
	float LODMin;
	float LODMax;
	W3dVectorStruct Min;
	W3dVectorStruct Max;
	W3dVectorStruct SphCenter;
	float SphRadius;
	W3dVectorStruct Translation;
	float Rotation[9];
	W3dVectorStruct MassCenter;
	float Inertia[9];
	float Volume;
	char HierarchyTreeName[W3D_NAME_LEN];
	char HierarchyModelName[W3D_NAME_LEN];
	uint32 FutureUse[24];
};

struct W3dMaterialStruct
{
	char MaterialName[W3D_NAME_LEN];
	char PrimaryName[W3D_NAME_LEN];
	char SecondaryName[W3D_NAME_LEN];
	uint32 RenderFlags;
	uint8 Red;
	uint8 Green;
	uint8 Blue;
};

struct W3dSurrenderTriangleStruct
{
	uint32 VertexIndices[3];
	W3dTexCoordStruct TexCoord[3];
	uint32 MaterialIdx;
	W3dVectorStruct Normal;
	uint32 Attributes;
	W3dRGBStruct Gourad[3];
};

struct W3dDamageStruct
{
	uint32 NumDamageMaterials;
	uint32 NumDamageVerts;
	uint32 NumDamageColors;
	uint32 DamageIndex;
	uint32 reserved[4];
};

struct W3dDamageVertexStruct
{
	uint32 VertexIndex;
	uint32 NewVertex;
	int reserved[2];
};

struct W3dDamageColorStruct
{
	uint32 VertexIndex;
	W3dRGBStruct NewColor;
};

struct W3dMaterial2Struct
{
	char MaterialName[W3D_NAME_LEN];
	char PrimaryName[W3D_NAME_LEN];
	char SecondaryName[W3D_NAME_LEN];
	int RenderFlags;
	char Red;
	char Green;
	char Blue;
	uint8 Alpha;
	uint16 PrimaryNumFrames;
	uint16 SecondaryNumFrames;
	uint32 reserved[3];
};

#define W3DMATERIAL_USE_ALPHA						0x00000001
#define W3DMATERIAL_USE_SORTING						0x00000002
#define W3DMATERIAL_HINT_DIT_OVER_DCT				0x00000010
#define W3DMATERIAL_HINT_SIT_OVER_SCT				0x00000020
#define W3DMATERIAL_HINT_DIT_OVER_DIG				0x00000040
#define W3DMATERIAL_HINT_SIT_OVER_SIG				0x00000080
#define W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA	0x00000100
#define W3DMATERIAL_PSX_TRANS_100					0x01000000
#define W3DMATERIAL_PSX_TRANS_50					0x02000000
#define W3DMATERIAL_PSX_TRANS_25					0x03000000
#define W3DMATERIAL_PSX_TRANS_MINUS_100				0x04000000
#define W3DMATERIAL_PSX_NO_RT_LIGHTING				0x08000000
struct W3dMaterial3Struct
{
	uint32 Attributes;
	W3dRGBStruct DiffuseColor;
	W3dRGBStruct SpecularColor;
	W3dRGBStruct EmissiveCoefficients;
	W3dRGBStruct AmbientCoefficients;
	W3dRGBStruct DiffuseCoefficients;
	W3dRGBStruct SpecularCoefficients;
	float Shininess;
	float Opacity;
	float Translucency;
	float FogCoeff;
};

struct W3dMap3Struct
{
	uint16 MappingType;
	uint16 FrameCount;
	float FrameRate;
};

#define W3DVERTMAT_PSX_MASK							0xFF000000
#define W3DVERTMAT_PSX_TRANS_MASK					0x07000000
#define W3DVERTMAT_PSX_TRANS_NONE                   0x00000000
#define W3DVERTMAT_PSX_TRANS_100					0x01000000
#define W3DVERTMAT_PSX_TRANS_50						0x02000000
#define W3DVERTMAT_PSX_TRANS_25						0x03000000
#define W3DVERTMAT_PSX_TRANS_MINUS_100				0x04000000
#define W3DVERTMAT_PSX_NO_RT_LIGHTING				0x08000000

struct W3dHModelAuxDataStruct
{
	uint32 Attributes;
	uint32 MeshCount;
	uint32 CollisionCount;
	uint32 SkinCount;
	uint32 FutureCounts[8];
	float LODMin;
	float LODMax;
	uint32 FutureUse[32];
};

struct W3dLODModelHeaderStruct
{
	uint32 Version;
	char Name[W3D_NAME_LEN];
	uint16 NumLODs;
};

struct W3dLODStruct
{
	char RenderObjName[W3D_NAME_LEN * 2];
	float LODMin;
	float LODMax;
};

struct W3dPlaceholderStruct
{
	uint32 Version;
	float Transform[4][3];
	uint32 reserved;
	char Name[1];
};

struct W3dTextureReplacerHeaderStruct
{
	uint32 ReplacedTexturesCount;
};

struct W3dTextureReplacerStruct
{
	char MeshPath[15][32];
	char BonePath[15][32];
	char OldTextureName[260];
	char NewTextureName[260];
	W3dTextureInfoStruct TextureParams;
};
