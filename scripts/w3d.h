#ifndef TT_INCLUDE_W3D_H
#define TT_INCLUDE_W3D_H

#include "iostruct.h"
#include "quaternion.h"

#define W3D_MAKE_VERSION(major,minor)		(((major) << 16) | (minor))
#define W3D_GET_MAJOR_VERSION(ver)			((ver)>>16)
#define W3D_GET_MINOR_VERSION(ver)			((ver) & 0xFFFF)
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#ifndef W3X
#define W3D_NAME_LEN	16
#else
#define W3D_NAME_LEN	256
#endif

enum class W3DChunkType : uint32
{
	MESH                          = 0x00000000,
	VERTICES                      = 0x00000002,
	VERTEX_NORMALS                = 0x00000003,
	MESH_USER_TEXT                = 0x0000000C,
	VERTEX_INFLUENCES             = 0x0000000E,
	MESH_HEADER3                  = 0x0000001F,
	TRIANGLES                     = 0x00000020,
	VERTEX_SHADE_INDICES          = 0x00000022,
	PRELIT_UNLIT                  = 0x00000023,
	PRELIT_VERTEX                 = 0x00000024,
	PRELIT_LIGHTMAP_MULTI_PASS    = 0x00000025,
	PRELIT_LIGHTMAP_MULTI_TEXTURE = 0x00000026,
	MATERIAL_INFO                 = 0x00000028,
	SHADERS                       = 0x00000029,
	VERTEX_MATERIALS              = 0x0000002A,
	VERTEX_MATERIAL               = 0x0000002B,
	VERTEX_MATERIAL_NAME          = 0x0000002C,
	VERTEX_MATERIAL_INFO          = 0x0000002D,
	VERTEX_MAPPER_ARGS0           = 0x0000002E,
	VERTEX_MAPPER_ARGS1           = 0x0000002F,
	TEXTURES                      = 0x00000030,
	TEXTURE                       = 0x00000031,
	TEXTURE_NAME                  = 0x00000032,
	TEXTURE_INFO                  = 0x00000033,
	MATERIAL_PASS                 = 0x00000038,
	VERTEX_MATERIAL_IDS           = 0x00000039,
	SHADER_IDS                    = 0x0000003A,
	DCG                           = 0x0000003B,
	DIG                           = 0x0000003C,
	SCG                           = 0x0000003E,
	FXSHADER_IDS                  = 0x0000003F,
	TEXTURE_STAGE                 = 0x00000048,
	TEXTURE_IDS                   = 0x00000049,
	STAGE_TEXCOORDS               = 0x0000004A,
	PER_FACE_TEXCOORD_IDS         = 0x0000004B,
	DEFORM                        = 0x00000058,
	DEFORM_SET                    = 0x00000059,
	DEFORM_KEYFRAME               = 0x0000005A,
	DEFORM_DATA                   = 0x0000005B,
	FX_SHADERS                    = 0x00000050,
	FX_SHADER,
	FX_SHADER_INFO,
	FX_SHADER_CONSTANT,
	TANGENTS                      = 0x00000060,
	BINORMALS,
	PS2_SHADERS                   = 0x00000080,
	AABBTREE                       = 0x00000090,
	AABBTREE_HEADER,
	AABBTREE_POLYINDICES,
	AABBTREE_NODES,
	HIERARCHY                     = 0x00000100,
	HIERARCHY_HEADER,
	PIVOTS,
	PIVOT_FIXUPS,
	ANIMATION                     = 0x00000200,
	ANIMATION_HEADER,
	ANIMATION_CHANNEL,
	BIT_CHANNEL,
	COMPRESSED_ANIMATION          = 0x00000280,
	COMPRESSED_ANIMATION_HEADER,
	COMPRESSED_ANIMATION_CHANNEL,
	COMPRESSED_BIT_CHANNEL,
	MORPH_ANIMATION               = 0x000002C0,
	MORPHANIM_HEADER,
	MORPHANIM_CHANNEL,
	MORPHANIM_POSENAME,
	MORPHANIM_KEYDATA,
	MORPHANIM_PIVOTCHANNELDATA,
	HMODEL                        = 0x00000300,
	HMODEL_HEADER,
	NODE,
	COLLISION_NODE,
	SKIN_NODE,
	OBSOLETE_HMODEL_AUX_DATA,
	OBSOLETE_SHADOW_NODE,
	LODMODEL                      = 0x00000400,
	LODMODEL_HEADER,
	LOD,
	COLLECTION                    = 0x00000420,
	COLLECTION_HEADER,
	COLLECTION_OBJ_NAME,
	PLACEHOLDER,
	TRANSFORM_NODE,
	POINTS                        = 0x00000440,
	LIGHT                         = 0x00000460,
	LIGHT_INFO,
	SPOT_LIGHT_INFO,
	NEAR_ATTENUATION,
	FAR_ATTENUATION,
	SPOT_LIGHT_INFO_5_0, // new
	EMITTER                       = 0x00000500,
	EMITTER_HEADER,
	EMITTER_USER_DATA,
	EMITTER_INFO,
	EMITTER_INFOV2,
	EMITTER_PROPS,
	OBSOLETE_EMITTER_COLOR_KEYFRAME,
	OBSOLETE_EMITTER_OPACITY_KEYFRAME,
	OBSOLETE_EMITTER_SIZE_KEYFRAME,
	EMITTER_LINE_PROPERTIES,
	EMITTER_ROTATION_KEYFRAMES,
	EMITTER_FRAME_KEYFRAMES,
	EMITTER_BLUR_TIME_KEYFRAMES,
	EMITTER_EXTRA_INFO,
	AGGREGATE                     = 0x00000600,
	AGGREGATE_HEADER,
	AGGREGATE_INFO,
	TEXTURE_REPLACER_INFO,
	AGGREGATE_CLASS_INFO,
	HLOD                          = 0x00000700,
	HLOD_HEADER,
	HLOD_LOD_ARRAY,
	HLOD_SUB_OBJECT_ARRAY_HEADER,
	HLOD_SUB_OBJECT,
	HLOD_AGGREGATE_ARRAY,
	HLOD_PROXY_ARRAY,
	BOX                           = 0x00000740,
	SPHERE,
	RING,
	NULL_OBJECT                   = 0x00000750,
	LIGHTSCAPE                    = 0x00000800,
	LIGHTSCAPE_LIGHT,
	LIGHT_TRANSFORM,
	DAZZLE                        = 0x00000900,
	DAZZLE_NAME,
	DAZZLE_TYPENAME,
	SOUNDROBJ                     = 0x00000A00,
	SOUNDROBJ_HEADER,
	SOUNDROBJ_DEFINITION,
	SECONDARY_VERTICES            = 0x00000C00,
	SECONDARY_VERTEX_NORMALS,
    LIGHTMAP_UV,

	UNKNOWN                       = 0xFFFFFFFF
};

struct W3dChunkHeader
{
	W3DChunkType ChunkType;
	uint32     ChunkSize;
};

typedef IOVector3Struct W3dVectorStruct;
typedef IOQuaternionStruct W3dQuaternionStruct;
struct W3dTexCoordStruct
{
	bool operator == (W3dTexCoordStruct t)
	{
		return ((U == t.U) && (V == t.V));
	}
	float U;
	float V;
};
struct W3dRGBStruct
{
	W3dRGBStruct(): R(0), G(0), B(0), pad(0)
	{
	}

	W3dRGBStruct (uint8 r, uint8 g, uint8 b): pad(0)
	{
		R = r;
		G = g;
		B = b;
	}

	void Set (uint8 r, uint8 g, uint8 b)
	{
		R = r;
		G = g;
		B = b;
	}

	void Set (float r, float g, float b)
	{
		R = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, r) * ((float) (UCHAR_MAX + 1)));
		G = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, g) * ((float) (UCHAR_MAX + 1)));
		B = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, b) * ((float) (UCHAR_MAX + 1)));
	}

	bool operator == (W3dRGBStruct c)
	{
		return ((R == c.R) && (G == c.G) && (B == c.B));
	}

	bool operator != (W3dRGBStruct c)
	{
		return (!(*this == c));
	}
	
	W3dRGBStruct operator += (W3dRGBStruct c)
	{
		R = MIN (((unsigned) R) + ((unsigned) c.R), (unsigned) UCHAR_MAX);
		G = MIN (((unsigned) G) + ((unsigned) c.G), (unsigned) UCHAR_MAX);
		B = MIN (((unsigned) B) + ((unsigned) c.B), (unsigned) UCHAR_MAX);
		return (*this);
	}

	W3dRGBStruct operator *= (W3dRGBStruct c)
	{
		R = (((unsigned) R) * ((unsigned) c.R)) / ((unsigned) UCHAR_MAX);
		G = (((unsigned) G) * ((unsigned) c.G)) / ((unsigned) UCHAR_MAX);
		B = (((unsigned) B) * ((unsigned) c.B)) / ((unsigned) UCHAR_MAX);
		return (*this);
	}

	unsigned Get_Color()
	{
		return (R<<24)|(G<<16)|(B<<8);
	}

	uint8			R;
	uint8			G;
	uint8			B;
	uint8			pad;
};
struct W3dRGBAStruct
{
	uint8			R;
	uint8			G;
	uint8			B;
	uint8			A;
};
struct W3dMaterialInfoStruct
{
	uint32 PassCount;
	uint32 VertexMaterialCount;
	uint32 ShaderCount;
	uint32 TextureCount;
};
#define		W3DVERTMAT_USE_DEPTH_CUE								0x00000001
#define		W3DVERTMAT_ARGB_EMISSIVE_ONLY							0x00000002
#define		W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE					0x00000004
#define		W3DVERTMAT_DEPTH_CUE_TO_ALPHA							0x00000008

#define		W3DVERTMAT_STAGE0_MAPPING_MASK						0x00FF0000
#define		W3DVERTMAT_STAGE0_MAPPING_UV							0x00000000
#define		W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT				0x00010000
#define		W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT		0x00020000
#define		W3DVERTMAT_STAGE0_MAPPING_SCREEN						0x00030000
#define		W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET			0x00040000
#define		W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE				0x00050000
#define		W3DVERTMAT_STAGE0_MAPPING_SCALE						0x00060000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID						0x00070000
#define		W3DVERTMAT_STAGE0_MAPPING_ROTATE						0x00080000
#define		W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET		0x00090000
#define		W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET		0x000A0000
#define		W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET	0x000B0000
#define		W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV			0x000C0000
#define		W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT			0x000D0000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV		0x000E0000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT		0x000F0000
#define		W3DVERTMAT_STAGE0_MAPPING_RANDOM						0x00100000
#define		W3DVERTMAT_STAGE0_MAPPING_EDGE						0x00110000
#define		W3DVERTMAT_STAGE0_MAPPING_BUMPENV					0x00120000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV		0x00130000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT		0x00140000

#define		W3DVERTMAT_STAGE1_MAPPING_MASK						0x0000FF00
#define		W3DVERTMAT_STAGE1_MAPPING_UV							0x00000000
#define		W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT				0x00000100
#define		W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT		0x00000200
#define		W3DVERTMAT_STAGE1_MAPPING_SCREEN						0x00000300
#define		W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET			0x00000400
#define		W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE				0x00000500
#define		W3DVERTMAT_STAGE1_MAPPING_SCALE						0x00000600
#define		W3DVERTMAT_STAGE1_MAPPING_GRID						0x00000700
#define		W3DVERTMAT_STAGE1_MAPPING_ROTATE						0x00000800
#define		W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET		0x00000900
#define		W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET		0x00000A00
#define		W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET	0x00000B00
#define		W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV			0x00000C00
#define		W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT			0x00000D00
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV		0x00000E00
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT		0x00000F00
#define		W3DVERTMAT_STAGE1_MAPPING_RANDOM						0x00001000
#define		W3DVERTMAT_STAGE1_MAPPING_EDGE						0x00001100
#define		W3DVERTMAT_STAGE1_MAPPING_BUMPENV					0x00001200
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV		0x00001300
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT		0x00001400

struct W3dVertexMaterialStruct
{
	W3dVertexMaterialStruct(void): Attributes(0), Shininess(0), Opacity(0), Translucency(0)
	{
	}

	bool operator == (const W3dVertexMaterialStruct& vm)
	{
		return (	  Attributes   == vm.Attributes
				  && Ambient	   == vm.Ambient
				  && Diffuse	   == vm.Diffuse
				  && Specular	   == vm.Specular
				  && Emissive	   == vm.Emissive
				  && Shininess	   == vm.Shininess
				  && Opacity	   == vm.Opacity
				  && Translucency == vm.Translucency);
	}

	bool operator != (const W3dVertexMaterialStruct& vm)
	{
		return (!(*this == vm));
	}
	
	uint32					Attributes;
	W3dRGBStruct			Ambient;
	W3dRGBStruct			Diffuse;
	W3dRGBStruct			Specular;
	W3dRGBStruct			Emissive;
	float					Shininess;
	float					Opacity;
	float					Translucency;
};
inline void W3d_Vertex_Material_Reset(W3dVertexMaterialStruct * vmat) 
{ 
	vmat->Attributes = 0;
	vmat->Ambient.R = vmat->Ambient.G = vmat->Ambient.B = 255;
	vmat->Diffuse.R = vmat->Diffuse.G = vmat->Diffuse.B = 255;
	vmat->Specular.R = vmat->Specular.G = vmat->Specular.B = 0;
	vmat->Emissive.R = vmat->Emissive.G = vmat->Emissive.B = 0;
	vmat->Shininess = 1.0f;
	vmat->Opacity = 1.0f;
	vmat->Translucency = 0.0f;
}
enum
{
	W3DSHADER_DEPTHCOMPARE_PASS_NEVER = 0,
	W3DSHADER_DEPTHCOMPARE_PASS_LESS,
	W3DSHADER_DEPTHCOMPARE_PASS_EQUAL,
	W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,
	W3DSHADER_DEPTHCOMPARE_PASS_GREATER,
	W3DSHADER_DEPTHCOMPARE_PASS_NOTEQUAL,
	W3DSHADER_DEPTHCOMPARE_PASS_GEQUAL,
	W3DSHADER_DEPTHCOMPARE_PASS_ALWAYS,
	W3DSHADER_DEPTHCOMPARE_PASS_MAX,

	W3DSHADER_DEPTHMASK_WRITE_DISABLE = 0,
	W3DSHADER_DEPTHMASK_WRITE_ENABLE,
	W3DSHADER_DEPTHMASK_WRITE_MAX,

	W3DSHADER_ALPHATEST_DISABLE = 0,
	W3DSHADER_ALPHATEST_ENABLE,
	W3DSHADER_ALPHATEST_MAX,

  	W3DSHADER_DESTBLENDFUNC_ZERO = 0,
  	W3DSHADER_DESTBLENDFUNC_ONE,
 	W3DSHADER_DESTBLENDFUNC_SRC_COLOR,
 	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR,
 	W3DSHADER_DESTBLENDFUNC_SRC_ALPHA,
 	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA,
 	W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG,
	W3DSHADER_DESTBLENDFUNC_MAX,

	W3DSHADER_PRIGRADIENT_DISABLE = 0,
	W3DSHADER_PRIGRADIENT_MODULATE,
	W3DSHADER_PRIGRADIENT_ADD,
	W3DSHADER_PRIGRADIENT_BUMPENVMAP,
	W3DSHADER_PRIGRADIENT_BUMPENVMAPLUMINANCE,
	W3DSHADER_PRIGRADIENT_MODULATE2X,
	W3DSHADER_PRIGRADIENT_MAX,

	W3DSHADER_SECGRADIENT_DISABLE = 0,
	W3DSHADER_SECGRADIENT_ENABLE,
	W3DSHADER_SECGRADIENT_MAX,

  	W3DSHADER_SRCBLENDFUNC_ZERO = 0,
  	W3DSHADER_SRCBLENDFUNC_ONE,
 	W3DSHADER_SRCBLENDFUNC_SRC_ALPHA,
 	W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA,
	W3DSHADER_SRCBLENDFUNC_MAX,

	W3DSHADER_TEXTURING_DISABLE = 0,
	W3DSHADER_TEXTURING_ENABLE,
	W3DSHADER_TEXTURING_MAX,

	W3DSHADER_DETAILCOLORFUNC_DISABLE = 0,
	W3DSHADER_DETAILCOLORFUNC_DETAIL,
	W3DSHADER_DETAILCOLORFUNC_SCALE,
	W3DSHADER_DETAILCOLORFUNC_INVSCALE,
	W3DSHADER_DETAILCOLORFUNC_ADD,
	W3DSHADER_DETAILCOLORFUNC_SUB,
	W3DSHADER_DETAILCOLORFUNC_SUBR,
	W3DSHADER_DETAILCOLORFUNC_BLEND,
	W3DSHADER_DETAILCOLORFUNC_DETAILBLEND,
	W3DSHADER_DETAILCOLORFUNC_ADDSIGNED,
	W3DSHADER_DETAILCOLORFUNC_ADDSIGNED2X,
	W3DSHADER_DETAILCOLORFUNC_SCALE2X,
	W3DSHADER_DETAILCOLORFUNC_MODALPHAADDCLR,
	W3DSHADER_DETAILCOLORFUNC_MAX,

	W3DSHADER_DETAILALPHAFUNC_DISABLE = 0,
	W3DSHADER_DETAILALPHAFUNC_DETAIL,
	W3DSHADER_DETAILALPHAFUNC_SCALE,
	W3DSHADER_DETAILALPHAFUNC_INVSCALE,
	W3DSHADER_DETAILALPHAFUNC_MAX,

	W3DSHADER_DEPTHCOMPARE_DEFAULT = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,
	W3DSHADER_DEPTHMASK_DEFAULT = W3DSHADER_DEPTHMASK_WRITE_ENABLE,
	W3DSHADER_ALPHATEST_DEFAULT = W3DSHADER_ALPHATEST_DISABLE,
	W3DSHADER_DESTBLENDFUNC_DEFAULT = W3DSHADER_DESTBLENDFUNC_ZERO,
	W3DSHADER_PRIGRADIENT_DEFAULT = W3DSHADER_PRIGRADIENT_MODULATE,
	W3DSHADER_SECGRADIENT_DEFAULT = W3DSHADER_SECGRADIENT_DISABLE,
	W3DSHADER_SRCBLENDFUNC_DEFAULT = W3DSHADER_SRCBLENDFUNC_ONE,
	W3DSHADER_TEXTURING_DEFAULT = W3DSHADER_TEXTURING_DISABLE,
	W3DSHADER_DETAILCOLORFUNC_DEFAULT = W3DSHADER_DETAILCOLORFUNC_DISABLE,
	W3DSHADER_DETAILALPHAFUNC_DEFAULT = W3DSHADER_DETAILALPHAFUNC_DISABLE,
};

enum PS2_SHADER_SETTINGS {
	PSS_SRC = 0,
	PSS_DEST,
	PSS_ZERO,
	PSS_SRC_ALPHA = 0,
	PSS_DEST_ALPHA,
	PSS_ONE,
	PSS_PRIGRADIENT_DECAL = 0,
	PSS_PRIGRADIENT_MODULATE,
	PSS_PRIGRADIENT_HIGHLIGHT,
	PSS_PRIGRADIENT_HIGHLIGHT2,
	PSS_PS2_PRIGRADIENT_MODULATE = 0,
	PSS_PS2_PRIGRADIENT_DECAL,
	PSS_PS2_PRIGRADIENT_HIGHLIGHT,
	PSS_PS2_PRIGRADIENT_HIGHLIGHT2,
	PSS_DEPTHCOMPARE_PASS_NEVER = 0,
	PSS_DEPTHCOMPARE_PASS_LESS,
	PSS_DEPTHCOMPARE_PASS_ALWAYS,
	PSS_DEPTHCOMPARE_PASS_LEQUAL,
};

struct W3dShaderStruct
{
	W3dShaderStruct(void): DepthCompare(0), DepthMask(0), ColorMask(0), DestBlend(0), FogFunc(0), PriGradient(0),
	                       SecGradient(0), SrcBlend(0), Texturing(0), DetailColorFunc(0), DetailAlphaFunc(0),
	                       ShaderPreset(0), AlphaTest(0), PostDetailColorFunc(0), PostDetailAlphaFunc(0), pad{}
	{
	}

	uint8						DepthCompare;
	uint8						DepthMask;
	uint8						ColorMask;
	uint8						DestBlend;
	uint8						FogFunc;
	uint8						PriGradient;
	uint8						SecGradient;
	uint8						SrcBlend;
	uint8						Texturing;
	uint8						DetailColorFunc;
	uint8						DetailAlphaFunc;
	uint8						ShaderPreset;
	uint8						AlphaTest;
	uint8						PostDetailColorFunc;
	uint8						PostDetailAlphaFunc;
	uint8						pad[1];
};

struct W3dPS2ShaderStruct
{
	uint8						DepthCompare;
	uint8						DepthMask;
	uint8						PriGradient;
	uint8						Texturing;
	uint8						AlphaTest;
	uint8						AParam;
	uint8						BParam;
	uint8						CParam;
	uint8						DParam;
	uint8						pad[3];
};

inline void W3d_Shader_Reset(W3dShaderStruct * s)									{
																										s->DepthCompare = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL;
																										s->DepthMask = W3DSHADER_DEPTHMASK_WRITE_ENABLE;
																										s->ColorMask = 0;
																										s->DestBlend = W3DSHADER_DESTBLENDFUNC_ZERO;
																										s->FogFunc = 0;
																										s->PriGradient = W3DSHADER_PRIGRADIENT_MODULATE;
																										s->SecGradient = W3DSHADER_SECGRADIENT_DISABLE;
																										s->SrcBlend = W3DSHADER_SRCBLENDFUNC_ONE;
																										s->Texturing = W3DSHADER_TEXTURING_DISABLE;
																										s->DetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
																										s->DetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
																										s->ShaderPreset = 0;
																										s->AlphaTest = W3DSHADER_ALPHATEST_DISABLE;
																										s->PostDetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
																										s->PostDetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
																										s->pad[0] = 0;
																									}

inline void W3d_Shader_Set_Depth_Compare(W3dShaderStruct * s,int val)			 { s->DepthCompare = (uint8)val; }
inline void W3d_Shader_Set_Depth_Mask(W3dShaderStruct * s,int val)				 { s->DepthMask = (uint8)val; }
inline void W3d_Shader_Set_Dest_Blend_Func(W3dShaderStruct * s,int val)			 { s->DestBlend = (uint8)val; }
inline void W3d_Shader_Set_Pri_Gradient(W3dShaderStruct * s,int val)				 { s->PriGradient = (uint8)val; }
inline void W3d_Shader_Set_Sec_Gradient(W3dShaderStruct * s,int val)				 { s->SecGradient = (uint8)val; }
inline void W3d_Shader_Set_Src_Blend_Func(W3dShaderStruct * s,int val)			 { s->SrcBlend = (uint8)val; }
inline void W3d_Shader_Set_Texturing(W3dShaderStruct * s,int val)					 { s->Texturing = (uint8)val; }
inline void W3d_Shader_Set_Detail_Color_Func(W3dShaderStruct * s,int val)		 { s->DetailColorFunc = (uint8)val; }
inline void W3d_Shader_Set_Detail_Alpha_Func(W3dShaderStruct * s,int val)		 { s->DetailAlphaFunc = (uint8)val; }
inline void W3d_Shader_Set_Alpha_Test(W3dShaderStruct * s,int val)				 { s->AlphaTest = (uint8)val; }
inline void W3d_Shader_Set_Post_Detail_Color_Func(W3dShaderStruct * s,int val) { s->PostDetailColorFunc = (uint8)val; }
inline void W3d_Shader_Set_Post_Detail_Alpha_Func(W3dShaderStruct * s,int val) { s->PostDetailAlphaFunc = (uint8)val; }
inline void W3d_Shader_Set_PS2_Param_A(W3dShaderStruct *s, int val) { s->ColorMask = (uint8)val; }
inline void W3d_Shader_Set_PS2_Param_B(W3dShaderStruct *s, int val) { s->FogFunc = (uint8)val; }
inline void W3d_Shader_Set_PS2_Param_C(W3dShaderStruct *s, int val) { s->ShaderPreset = (uint8)val; }
inline void W3d_Shader_Set_PS2_Param_D(W3dShaderStruct *s, int val) { s->pad[0] = (uint8)val; }
inline int W3d_Shader_Get_PS2_Param_A(const W3dShaderStruct *s) { return (s->ColorMask); }
inline int W3d_Shader_Get_PS2_Param_B(const W3dShaderStruct *s) { return (s->FogFunc); }
inline int W3d_Shader_Get_PS2_Param_C(const W3dShaderStruct *s) { return (s->ShaderPreset); }
inline int W3d_Shader_Get_PS2_Param_D(const W3dShaderStruct *s) { return (s->pad[0]); }
inline int W3d_Shader_Get_PS2_Param_A(const W3dPS2ShaderStruct *s) { return (s->AParam); }
inline int W3d_Shader_Get_PS2_Param_B(const W3dPS2ShaderStruct *s) { return (s->BParam); }
inline int W3d_Shader_Get_PS2_Param_C(const W3dPS2ShaderStruct *s) { return (s->CParam); }
inline int W3d_Shader_Get_PS2_Param_D(const W3dPS2ShaderStruct *s) { return (s->DParam); }
inline int W3d_Shader_Get_Depth_Compare(const W3dShaderStruct * s)				 { return s->DepthCompare; }
inline int W3d_Shader_Get_Depth_Mask(const W3dShaderStruct * s)					 { return s->DepthMask; }
inline int W3d_Shader_Get_Dest_Blend_Func(const W3dShaderStruct * s)				 { return s->DestBlend; }
inline int W3d_Shader_Get_Pri_Gradient(const W3dShaderStruct * s)					 { return s->PriGradient; } 
inline int W3d_Shader_Get_Sec_Gradient(const W3dShaderStruct * s)					 { return s->SecGradient; } 
inline int W3d_Shader_Get_Src_Blend_Func(const W3dShaderStruct * s)				 { return s->SrcBlend; } 
inline int W3d_Shader_Get_Texturing(const W3dShaderStruct * s)						 { return s->Texturing; } 
inline int W3d_Shader_Get_Detail_Color_Func(const W3dShaderStruct * s)			 { return s->DetailColorFunc; }
inline int W3d_Shader_Get_Detail_Alpha_Func(const W3dShaderStruct * s)			 { return s->DetailAlphaFunc; }
inline int W3d_Shader_Get_Alpha_Test(const W3dShaderStruct * s)					 { return s->AlphaTest; }
inline int W3d_Shader_Get_Post_Detail_Color_Func(const W3dShaderStruct * s)	 { return s->PostDetailColorFunc; }
inline int W3d_Shader_Get_Post_Detail_Alpha_Func(const W3dShaderStruct * s)	 { return s->PostDetailAlphaFunc; }
#define W3DTEXTURE_PUBLISH					0x0001
#define W3DTEXTURE_RESIZE_OBSOLETE		0x0002
#define W3DTEXTURE_NO_LOD					0x0004
#define W3DTEXTURE_CLAMP_U					0x0008
#define W3DTEXTURE_CLAMP_V					0x0010
#define W3DTEXTURE_ALPHA_BITMAP			0x0020
#define W3DTEXTURE_MIP_LEVELS_MASK		0x00c0
#define W3DTEXTURE_MIP_LEVELS_ALL		0x0000
#define W3DTEXTURE_MIP_LEVELS_2			0x0040
#define W3DTEXTURE_MIP_LEVELS_3			0x0080
#define W3DTEXTURE_MIP_LEVELS_4			0x00c0
#define W3DTEXTURE_HINT_SHIFT				8
#define W3DTEXTURE_HINT_MASK				0x0000ff00
#define W3DTEXTURE_HINT_BASE				0x0000
#define W3DTEXTURE_HINT_EMISSIVE			0x0100
#define W3DTEXTURE_HINT_ENVIRONMENT		0x0200
#define W3DTEXTURE_HINT_SHINY_MASK		0x0300
#define W3DTEXTURE_TYPE_MASK				0x1000	
#define W3DTEXTURE_TYPE_COLORMAP			0x0000
#define W3DTEXTURE_TYPE_BUMPMAP			0x1000
#define W3DTEXTURE_ANIM_LOOP				0x0000
#define W3DTEXTURE_ANIM_PINGPONG			0x0001
#define W3DTEXTURE_ANIM_ONCE				0x0002
#define W3DTEXTURE_ANIM_MANUAL			0x0003
struct W3dTextureInfoStruct
{
	W3dTextureInfoStruct(void): Attributes(0), AnimType(0), FrameCount(0), FrameRate(0)
	{
	}

	uint16					Attributes;
	uint16					AnimType;
	uint32					FrameCount;
	float					FrameRate;
};
struct W3dTriStruct
{
	uint32					Vindex[3];
	uint32					Attributes;
	W3dVectorStruct			Normal;
	float					Dist;
};

enum class SURFACE_TYPE : uint32
{
	LIGHT_METAL = 0,
	HEAVY_METAL,
	WATER,
	SAND,
	DIRT,
	MUD,
	GRASS,
	WOOD,
	CONCRETE,
	FLESH,
	ROCK,
	SNOW,
	ICE,
	DEFAULT,
	GLASS,
	CLOTH,
	TIBERIUM_FIELD,
	FOLIAGE_PERMEABLE,
	GLASS_PERMEABLE,
	ICE_PERMEABLE,
	CLOTH_PERMEABLE,
	ELECTRICAL,
	ELECTRICAL_PERMEABLE,
	FLAMMABLE,
	FLAMMABLE_PERMEABLE,
	STEAM,
	STEAM_PERMEABLE,
	WATER_PERMEABLE,
	TIBERIUM_WATER,
	TIBERIUM_WATER_PERMEABLE,
	UNDERWATER_DIRT,
	UNDERWATER_TIBERIUM_DIRT,
	BLUE_TIBERIUM,
	RED_TIBERIUM,
	TIBERIUM_VEINS,
	LASER,
	SNOW_PERMIABLE,
	ELECTRICAL_GLASS,
	ELECTRICAL_GLASS_PERMEABLE,
	SLUSH,
	EXTRA_1,
	EXTRA_2,
	EXTRA_3,
	EXTRA_4,
	EXTRA_5,
	EXTRA_6,
	EXTRA_7,
	EXTRA_8,
	MAX
};

static constexpr std::array<const char*, static_cast<size_t>(SURFACE_TYPE::MAX)> SURFACE_TYPE_STRINGS
{
	"Light Metal",
	"Heavy Metal",
	"Water",
	"Sand",
	"Dirt",
	"Mud",
	"Grass",
	"Wood",
	"Concrete",
	"Flesh",
	"Rock",
	"Snow",
	"Ice",
	"Default",
	"Glass",
	"Cloth",
	"Tiberium Field",
	"Foliage Permeable",
	"Glass Permeable",
	"Ice Permeable",
	"Cloth Permeable",
	"Electrical",
	"Electrical Permeable",
	"Flammable",
	"Flammable Permeable",
	"Steam",
	"Steam Permeable",
	"Water Permeable",
	"Tiberium Water",
	"Tiberium Water Permeable",
	"Underwater Dirt",
	"Underwater Tiberium Dirt",
	"Blue Tiberium",
	"Red Tiberium",
	"Tiberium Veins",
	"Laser",
	"Snow Permeable",
	"Electrical Glass",
	"Electrical Glass Permeable",
	"Slush",
	"Extra 1",
	"Extra 2",
	"Extra 3",
	"Extra 4",
	"Extra 5",
	"Extra 6",
	"Extra 7",
	"Extra 8"
};

#define W3D_MESH_FLAG_NONE										0x00000000
#define W3D_MESH_FLAG_COLLISION_BOX							0x00000001
#define W3D_MESH_FLAG_SKIN										0x00000002
#define W3D_MESH_FLAG_SHADOW									0x00000004
#define W3D_MESH_FLAG_ALIGNED									0x00000008
#define W3D_MESH_FLAG_COLLISION_TYPE_MASK					0x00000FF0
#define W3D_MESH_FLAG_COLLISION_TYPE_SHIFT							4
#define W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL			0x00000010
#define W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE			0x00000020
#define W3D_MESH_FLAG_COLLISION_TYPE_VIS					0x00000040
#define W3D_MESH_FLAG_COLLISION_TYPE_CAMERA				0x00000080
#define W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE				0x00000100
#define W3D_MESH_FLAG_COLLISION_TYPE_USER1				0x00000200
#define W3D_MESH_FLAG_COLLISION_TYPE_USER2				0x00000400
#define W3D_MESH_FLAG_HIDDEN									0x00001000
#define W3D_MESH_FLAG_TWO_SIDED								0x00002000
#define OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED				0x00004000
#define W3D_MESH_FLAG_CAST_SHADOW							0x00008000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_MASK					0x00FF0000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL				0x00000000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED		0x00010000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN					0x00020000
#define OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW	0x00030000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX					0x00040000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX					0x00050000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED	0x00060000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED		0x00070000
#define W3D_MESH_FLAG_PRELIT_MASK							0x0F000000
#define W3D_MESH_FLAG_PRELIT_UNLIT							0x01000000
#define W3D_MESH_FLAG_PRELIT_VERTEX							0x02000000
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS		0x04000000
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE	0x08000000
#define W3D_MESH_FLAG_SHATTERABLE							0x10000000
#define W3D_MESH_FLAG_NPATCHABLE								0x20000000
#define W3D_MESH_FLAG_PRELIT									0x40000000
#define W3D_MESH_FLAG_ALWAYSDYNLIGHT							0x80000000
#define W3D_CURRENT_MESH_VERSION		W3D_MAKE_VERSION(4,2)
#define W3D_VERTEX_CHANNEL_LOCATION		0x00000001
#define W3D_VERTEX_CHANNEL_NORMAL		0x00000002
#define W3D_VERTEX_CHANNEL_TEXCOORD		0x00000004
#define W3D_VERTEX_CHANNEL_COLOR			0x00000008
#define W3D_VERTEX_CHANNEL_BONEID		0x00000010
#define W3D_VERTEX_CHANNEL_TANGENT		0x00000020
#define W3D_VERTEX_CHANNEL_BINORMAL		0x00000040
#define W3D_VERTEX_CHANNEL_SMOOTHSKIN		0x00000080
#define W3D_FACE_CHANNEL_FACE				0x00000001
#define SORT_LEVEL_NONE						0
#define MAX_SORT_LEVEL						32
#define SORT_LEVEL_BIN1						20
#define SORT_LEVEL_BIN2						15
#define SORT_LEVEL_BIN3						10
struct W3dMeshHeader3Struct
{
	uint32					Version;
	uint32					Attributes;
	char					MeshName[W3D_NAME_LEN];
	char					ContainerName[W3D_NAME_LEN];
	uint32					NumTris;
	uint32					NumVertices;
	uint32					NumMaterials;
	uint32					NumDamageStages;
	sint32					SortLevel;
	uint32					PrelitVersion;
	uint32					FutureCounts[1];
	uint32					VertexChannels;
	uint32					FaceChannels;
	W3dVectorStruct			Min;
	W3dVectorStruct			Max;
	W3dVectorStruct			SphCenter;
	float					SphRadius;
};

struct W3dVertInfStruct
{
    uint16 BoneIdx[2];
    uint16 Weight[2];
};
struct W3dMeshDeform
{
	uint32					SetCount;
	uint32					AlphaPasses;
	uint32					reserved[3];
};
struct W3dDeformSetInfo
{
	uint32					KeyframeCount;
	uint32					flags;
	uint32					reserved[1];
};
#define W3D_DEFORM_SET_MANUAL_DEFORM	0x00000001
struct W3dDeformKeyframeInfo
{
	float					DeformPercent;
	uint32					DataCount;
	uint32					reserved[2];
};
struct W3dDeformData
{
	uint32					VertexIndex;
	W3dVectorStruct		Position;
	W3dRGBAStruct			Color;
	uint32					reserved[2];
};
struct W3dMeshAABTreeHeader
{
	uint32					NodeCount;
	uint32					PolyCount;
	uint32					Padding[6];
};
struct W3dMeshAABTreeNode
{
	W3dVectorStruct		Min;
	W3dVectorStruct		Max;
	uint32				FrontOrPoly0;
	uint32				BackOrPolyCount;
};
#define W3D_CURRENT_HTREE_VERSION		W3D_MAKE_VERSION(4,1)
struct W3dHierarchyStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	uint32					NumPivots;				
	W3dVectorStruct		Center;					
};
struct W3dPivotStruct
{
	char						Name[W3D_NAME_LEN];
	uint32					ParentIdx;
	W3dVectorStruct		Translation;
	W3dVectorStruct		EulerAngles;
	W3dQuaternionStruct	Rotation;
};
struct W3dPivotFixupStruct
{
	float TM[4][3];
};
#define W3D_CURRENT_HANIM_VERSION				W3D_MAKE_VERSION(4,1)
#define W3D_CURRENT_COMPRESSED_HANIM_VERSION	W3D_MAKE_VERSION(0,1)
#define W3D_CURRENT_MORPH_HANIM_VERSION		W3D_MAKE_VERSION(0,1)
struct W3dAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				
	char						HierarchyName[W3D_NAME_LEN];
	uint32					NumFrames;
	uint32					FrameRate;

};
struct W3dCompressedAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				
	char						HierarchyName[W3D_NAME_LEN];
	uint32					NumFrames;
	uint16					FrameRate;
	uint16					Flavor;
};
enum 
{
	ANIM_CHANNEL_X = 0,
	ANIM_CHANNEL_Y,
	ANIM_CHANNEL_Z,
	ANIM_CHANNEL_XR,
	ANIM_CHANNEL_YR,
	ANIM_CHANNEL_ZR,
	ANIM_CHANNEL_Q,

	ANIM_CHANNEL_TIMECODED_X,
	ANIM_CHANNEL_TIMECODED_Y,
	ANIM_CHANNEL_TIMECODED_Z,
	ANIM_CHANNEL_TIMECODED_Q,

	ANIM_CHANNEL_ADAPTIVEDELTA_X,
	ANIM_CHANNEL_ADAPTIVEDELTA_Y,
	ANIM_CHANNEL_ADAPTIVEDELTA_Z,
	ANIM_CHANNEL_ADAPTIVEDELTA_Q,

    ANIM_CHANNEL_VIS,
};
enum
{
    ANIM_FLAVOR_TIMECODED = 0,
    ANIM_FLAVOR_ADAPTIVE_DELTA,

	 ANIM_FLAVOR_VALID
};
struct W3dAnimChannelStruct
{
	uint16					FirstFrame;			
	uint16					LastFrame;			
	uint16					VectorLen;
	uint16					Flags;
	uint16					Pivot;
	uint16					pad;
	float					Data[1];
};
enum 
{
	BIT_CHANNEL_VIS = 0,
	BIT_CHANNEL_TIMECODED_VIS,
};
struct W3dBitChannelStruct
{
	uint16					FirstFrame;
	uint16					LastFrame;			
	uint16					Flags;
	uint16					Pivot;
	uint8						DefaultVal;
	uint8						Data[1];
};
#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000
struct W3dTimeCodedAnimChannelStruct
{
	uint32					NumTimeCodes;
	uint16					Pivot;
	uint8						VectorLen;
	uint8						Flags;
	uint32					Data[1];
};								  
#define W3D_TIMECODED_BIT_MASK	0x80000000
struct W3dTimeCodedBitChannelStruct
{
	uint32					NumTimeCodes;
	uint16					Pivot;
	uint8						Flags;
	uint8						DefaultVal;
	uint32					Data[1];
};
struct W3dAdaptiveDeltaAnimChannelStruct
{
	uint32					NumFrames;
	uint16					Pivot;
	uint8						VectorLen;
	uint8						Flags;
	float						Scale;

	uint32					Data[1];

};
enum
{
	ANIM_FLAVOR_NEW_TIMECODED = 0,
	ANIM_FLAVOR_NEW_ADAPTIVE_DELTA_4,
	ANIM_FLAVOR_NEW_ADAPTIVE_DELTA_8,
	ANIM_FLAVOR_NEW_VALID
};
struct W3dCompressedMotionChannelStruct
{
	uint8 Zero;
	uint8 Flavor;
	uint8 VectorLen;
	uint8 Flags;
	uint16 NumTimeCodes;
	uint16 Pivot;
};
struct W3dMorphAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
	uint32					FrameCount;
	float					FrameRate;
	uint32					ChannelCount;
};
struct W3dMorphAnimKeyStruct
{
	uint32					MorphFrame;
	uint32					PoseFrame;
};
#define W3D_CURRENT_HMODEL_VERSION				W3D_MAKE_VERSION(4,2)
struct W3dHModelHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
	uint16					NumConnections;				
};
struct W3dHModelNodeStruct
{
	char						RenderObjName[W3D_NAME_LEN];
	uint16					PivotIdx;
};
#define W3D_CURRENT_LIGHT_VERSION			W3D_MAKE_VERSION(1,0)
#define W3D_LIGHT_ATTRIBUTE_TYPE_MASK						0x000000FF
#define W3D_LIGHT_ATTRIBUTE_POINT							0x00000001
#define W3D_LIGHT_ATTRIBUTE_DIRECTIONAL					0x00000002
#define W3D_LIGHT_ATTRIBUTE_SPOT								0x00000003
#define W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS					0x00000100
struct W3dLightStruct
{
	uint32				Attributes;
	uint32				Unused;
	W3dRGBStruct		Ambient;
	W3dRGBStruct		Diffuse;
	W3dRGBStruct		Specular;
	float				Intensity;
};
struct W3dSpotLightStruct
{
	W3dVectorStruct	SpotDirection;
	float SpotAngle;
	float SpotExponent;
};
struct W3dSpotLightStruct_v5_0 // new 5.0 spot lights
{
	float SpotOuterAngle;
	float SpotInnerAngle;
};
struct W3dLightAttenuationStruct
{
	float				Start;
	float				End;
};
struct W3dLightTransformStruct
{
	float Transform [3][4];
};
#define W3D_CURRENT_EMITTER_VERSION				0x00020000
enum
{
	EMITTER_TYPEID_DEFAULT = 0,
	EMITTER_TYPEID_COUNT
};
extern const char *EMITTER_TYPE_NAMES[EMITTER_TYPEID_COUNT];
struct W3dEmitterHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
};
struct W3dEmitterUserInfoStruct
{
	uint32				Type;
	uint32				SizeofStringParam;
	char					StringParam[1];
};
struct W3dEmitterInfoStruct
{
	char					TextureFilename[260];
	float				StartSize;
	float				EndSize;
	float				Lifetime;
	float				EmissionRate;
	float				MaxEmissions;
	float				VelocityRandom;
	float				PositionRandom;
	float				FadeTime;
	float				Gravity;
	float				Elasticity;
	W3dVectorStruct	Velocity;
	W3dVectorStruct	Acceleration;
	W3dRGBAStruct		StartColor;
	W3dRGBAStruct		EndColor;
};
struct W3dVolumeRandomizerStruct
{
	uint32				ClassID;
	float				Value1;
	float				Value2;
	float				Value3;
	uint32				reserved[4];
};
struct W3dEmitterExtraInfoStruct
{
	float FutureStartTime;
	uint8 unk1;
	uint32				reserved[8];
};
#define W3D_EMITTER_RENDER_MODE_TRI_PARTICLES		0
#define W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES		1
#define W3D_EMITTER_RENDER_MODE_LINE					2
#define W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA		3
#define W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM		4
#define W3D_EMITTER_FRAME_MODE_1x1						0
#define W3D_EMITTER_FRAME_MODE_2x2						1
#define W3D_EMITTER_FRAME_MODE_4x4						2
#define W3D_EMITTER_FRAME_MODE_8x8						3
#define W3D_EMITTER_FRAME_MODE_16x16					4
struct W3dEmitterInfoStructV2
{
	uint32							BurstSize;
	W3dVolumeRandomizerStruct	CreationVolume;
	W3dVolumeRandomizerStruct	VelRandom;
	float							OutwardVel;
	float							VelInherit;
	W3dShaderStruct				Shader;
	uint32							RenderMode;
	uint32							FrameMode;
	uint32							reserved[6];
};
struct W3dEmitterPropertyStruct
{
	uint32				ColorKeyframes;
	uint32				OpacityKeyframes;
	uint32				SizeKeyframes;
	W3dRGBAStruct		ColorRandom;
	float				OpacityRandom;
	float				SizeRandom;
	uint32				reserved[4];
};
struct W3dEmitterColorKeyframeStruct
{
	float				Time;
	W3dRGBAStruct		Color;
};
struct W3dEmitterOpacityKeyframeStruct
{
	float				Time;
	float				Opacity;
};
struct W3dEmitterSizeKeyframeStruct
{
	float				Time;
	float				Size;
};
struct W3dEmitterRotationHeaderStruct
{
	uint32				KeyframeCount;
	float				Random;
	float				OrientationRandom;
	uint32				Reserved[1];
};
struct W3dEmitterRotationKeyframeStruct
{
	float				Time;
	float				Rotation;
};
struct W3dEmitterFrameHeaderStruct
{
	uint32				KeyframeCount;
	float				Random;
	uint32				Reserved[2];
};
struct W3dEmitterFrameKeyframeStruct
{
	float				Time;
	float				Frame;
};
struct W3dEmitterBlurTimeHeaderStruct
{
	uint32				KeyframeCount;
	float				Random;
	uint32				Reserved[1];
};
struct W3dEmitterBlurTimeKeyframeStruct
{
	float				Time;
	float				BlurTime;
};
#define W3D_ELINE_MERGE_INTERSECTIONS 				0x00000001
#define W3D_ELINE_FREEZE_RANDOM						0x00000002
#define W3D_ELINE_DISABLE_SORTING					0x00000004
#define W3D_ELINE_END_CAPS 							0x00000008
#define W3D_ELINE_TEXTURE_MAP_MODE_MASK 			0xFF000000
#define W3D_ELINE_TEXTURE_MAP_MODE_OFFSET 		24
#define W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP		0x00000000
#define W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP 	0x00000001
#define W3D_ELINE_TILED_TEXTURE_MAP					0x00000002
#define W3D_ELINE_DEFAULT_BITS	(W3D_ELINE_MERGE_INTERSECTIONS | (W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET))
struct W3dEmitterLinePropertiesStruct
{
	uint32							Flags;
	uint32							SubdivisionLevel;	
	float							NoiseAmplitude;
	float							MergeAbortFactor;
	float							TextureTileFactor;
	float							UPerSec;
	float							VPerSec;
	uint32							Reserved[9];
};
#define W3D_CURRENT_AGGREGATE_VERSION			0x00010003
const int MESH_PATH_ENTRIES						= 15;
const int MESH_PATH_ENTRY_LEN						= (W3D_NAME_LEN * 2);
struct W3dAggregateHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
};
struct W3dAggregateInfoStruct
{
	char					BaseModelName[W3D_NAME_LEN*2];
	uint32				SubobjectCount;
};
struct W3dAggregateSubobjectStruct
{
	char					SubobjectName[W3D_NAME_LEN*2];
	char					BoneName[W3D_NAME_LEN*2];
};
const int W3D_AGGREGATE_FORCE_SUB_OBJ_LOD		= 0x00000001;
struct W3dAggregateMiscInfo
{
	uint32				OriginalClassID;
	uint32				Flags;
	uint32				reserved[3];
};
#define W3D_CURRENT_HLOD_VERSION			W3D_MAKE_VERSION(1,0)
#define NO_MAX_SCREEN_SIZE					WWMATH_FLOAT_MAX
struct W3dHLodHeaderStruct
{
	uint32					Version;
	uint32					LodCount;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
};
struct W3dHLodArrayHeaderStruct
{
	uint32					ModelCount;
	float					MaxScreenSize;
};
struct W3dHLodSubObjectStruct
{
	uint32					BoneIndex;
	char						Name[W3D_NAME_LEN*2];
#ifdef W3X
	int Type;
#endif
};
#define W3D_BOX_CURRENT_VERSION								W3D_MAKE_VERSION(1,0)
#define W3D_BOX_ATTRIBUTE_ORIENTED							0x00000001
#define W3D_BOX_ATTRIBUTE_ALIGNED							0x00000002
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_MASK			0x00000FF0
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_SHIFT						4
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL		0x00000010
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE	0x00000020
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS				0x00000040
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA			0x00000080
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE		0x00000100
struct W3dBoxStruct
{
	uint32				Version;
	uint32				Attributes;
	char					Name[2*W3D_NAME_LEN];
	W3dRGBStruct		Color;
	W3dVectorStruct	Center;
	W3dVectorStruct	Extent;
};
#define W3D_NULL_OBJECT_CURRENT_VERSION					W3D_MAKE_VERSION(1,0)
struct W3dNullObjectStruct
{
	uint32				Version;
	uint32				Attributes;
	uint32				pad[2];
	char					Name[2*W3D_NAME_LEN];
};
#define W3D_CURRENT_SOUNDROBJ_VERSION			0x00010000
struct W3dSoundRObjHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
	uint32				Flags;
	uint32				Padding[8];
};
struct W3dCollectionHeaderStruct
{
	unsigned long Version;
	char Name[W3D_NAME_LEN];
	unsigned long RenderObjectCount;
	unsigned long pad[2];
};

enum {
	RING_CAMERA_ALIGNED = 1,
	RING_LOOPING = 2
};
struct W3dRingStruct
{
	int unk0;
	int Flags;
	char Name[W3D_NAME_LEN * 2];
	IOVector3Struct Center;
	IOVector3Struct Extent;
	float AnimationDuration;
	IOVector3Struct Color;
	float Alpha;
	IOVector2Struct InnerScale;
	IOVector2Struct OuterScale;
	IOVector2Struct InnerExtent;
	IOVector2Struct OuterExtent;
	char TextureName[W3D_NAME_LEN * 2];
	W3dShaderStruct Shader;
	int TextureTiling;
};

enum
{
	SPHERE_ALPHA_VECTOR = 1,
	SPHERE_CAMERA_ALIGNED = 2,
	SPHERE_INVERT_EFFECT = 4,
	SPHERE_LOOPING = 8
};

struct AlphaVectorStruct
{
	Quaternion Quat;
	float Magnitude;
	AlphaVectorStruct() : Quat(0, 0, 0, 1), Magnitude(1)
	{
	}
	AlphaVectorStruct(Quaternion &quat, float magnitude) : Quat(quat), Magnitude(magnitude)
	{
	}
};

struct W3dSphereStruct
{
	int unk0;
	int Flags;
	char Name[W3D_NAME_LEN * 2];
	IOVector3Struct Center;
	IOVector3Struct Extent;
	float AnimationDuration;
	IOVector3Struct Color;
	float Alpha;
	IOVector3Struct Scale;
	AlphaVectorStruct Vector;
	char TextureName[W3D_NAME_LEN * 2];
	W3dShaderStruct Shader;
};

struct W3dFXShaderStruct
{
	char shadername[W3D_NAME_LEN * 2];
	uint8 technique;
	uint8 pad[3];
};

enum
{
	CONSTANT_TYPE_TEXTURE = 1,
	CONSTANT_TYPE_FLOAT1 = 2,
	CONSTANT_TYPE_FLOAT2 = 3,
	CONSTANT_TYPE_FLOAT3 = 4,
	CONSTANT_TYPE_FLOAT4 = 5,
	CONSTANT_TYPE_INT = 6,
	CONSTANT_TYPE_BOOL = 7
};

#endif
