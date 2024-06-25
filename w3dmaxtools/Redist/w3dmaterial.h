#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_MATERIAL_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_MATERIAL_H
#include <Graphics/ITextureDisplay.h>
#include <iparamb2.h>
#include <max.h>

#include "EnumUtilities.h"


enum class SURFACE_TYPE : uint32;
class BitmapTex;

namespace W3D::MaxTools
{
	enum class W3DMaterialMappingType : int
	{
		UV = 0,
		Environment,
		ClassicEnvironment,
		Screen,
		LinearOffset,
		Silhouette,
		Scale,
		Grid,
		Rotate,
		Sine,
		Step,
		ZigZag,
		WSClassicEnv,
		WSEnvironment,
		GridClassicEnv,
		GridEnvironment,
		Random,
		Edge,
		BumpEnv,
		GridWSClassicEnv,
		GridWSEnv,

		Num
	};

	const TSTR& W3DMaterialMappingTypeString(W3DMaterialMappingType mappingType);

	enum class W3DMaterialBlendMode : int
	{
		Opaque = 0,
		Add,
		Multiply,
		MultiplyAndAdd,
		Screen,
		AlphaBlend,
		AlphaTest,
		AlphaTestAndBlend,
		Custom,

		Num
	};

	const TSTR& W3DMaterialBlendModeString(W3DMaterialBlendMode blendMode);

	enum class W3DMaterialBlendModeSrcType : int
	{
		Zero = 0,
		One,
		SrcAlpha,
		OneMinusSrcAlpha,

		Num
	};

	const TSTR& W3DMaterialBlendModeSrcString(W3DMaterialBlendModeSrcType srcType);

	enum class W3DMaterialBlendModeDestType : int
	{
		Zero = 0,
		One,
		SrcColour,
		OneMinusSrcColour,
		SrcAlpha,
		OneMinusSrcAlpha,
		SrcColourPreFog,

		Num
	};

	const TSTR& W3DMaterialBlendModeDestString(W3DMaterialBlendModeDestType destType);

	enum class W3DMaterialPrimaryGradientMode : int
	{
		Disable = 0,
		Modulate,
		Add,
		BumpEnvironment,
		BumpEnvLum,
		Mod2X,

		Num
	};

	const TSTR& W3DMaterialPrimaryGradientModeString(W3DMaterialPrimaryGradientMode gradientMode);

	enum class W3DMaterialDepthCompMode : int
	{
		PassNever = 0,
		PassLess,
		PassEqual,
		PassLEqual,
		PassGreater,
		PassNEqual,
		PassGEqual,
		PassAlways,

		Num
	};

	const TSTR& W3DMaterialDepthCompModeString(W3DMaterialDepthCompMode depthCompMode);

	enum class W3DMaterialDetailColourMode : int
	{
		Disable = 0,
		Detail,
		Scale,
		InvScale,
		Add,
		Sub,
		SubR,
		Blend,
		DetailBlend,
		AddSigned,
		AddSigned2X,
		Scale2X,
		ModAlphaAddClr,

		Num
	};

	const TSTR& W3DMaterialDetailColourModeString(W3DMaterialDetailColourMode detailColourMode);

	enum class W3DMaterialDetailAlphaMode : int
	{
		Disable = 0,
		Detail,
		Scale,
		InvScale,

		Num
	};

	const TSTR& W3DMaterialDetailAlphaModeString(W3DMaterialDetailAlphaMode detailAlphaMode);

	enum class W3DMaterialTextureAnimMode : int
	{
		Loop = 0,
		PingPong,
		Once,
		Manual,

		Num
	};

	const TSTR& W3DMaterialTextureAnimModeString(W3DMaterialTextureAnimMode textureAnimMode);

	enum class W3DMaterialTexturePassHint : int
	{
		BaseTexture = 0,
		EmissiveLightMap,
		EnvironmentMap,
		ShinynessMap,

		Num
	};

	const TSTR& W3DMaterialTexturePassHintString(W3DMaterialTexturePassHint texturePassHint);

	enum class W3DMaterialBlockID : BlockID
	{
		SurfaceType = 0,
		PassCount,
		PassOne,
		PassTwo,
		PassThree,
		PassFour,

		Num
	};

	enum class W3DMaterialParamID : ParamID
	{
		SurfaceType = 0,
		StaticSortOn,
		StaticSortLevel,

		PassCount,

		AmbientColour,
		DiffuseColour,
		SpecularColour,
		EmissiveColour,
		SpecularToDiffuse,
		Opacity,
		Translucency,
		Shininess,
		Stage0Mapping,
		Stage0MappingArgs,
		Stage0MappingUVChannel,
		Stage1Mapping,
		Stage1MappingArgs,
		Stage1MappingUVChannel,

		BlendMode,
		CustomSrcMode,
		CustomDestMode,
		BlendWriteZBuffer,
		AlphaTest,
		PriGradient,
		SecGradient,
		DepthCmp,
		DetailColour,
		DetailAlpha,

		Stage0TextureEnabled,
		Stage0TextureMap,
		Stage0Publish,
		Stage0Display,
		Stage0ClampU,
		Stage0ClampV,
		Stage0NoLOD,
		Stage0Frames,
		Stage0FPS,
		Stage0AnimMode,
		Stage0PassHint,
		Stage0AlphaBitmap,

		Stage1TextureEnabled,
		Stage1TextureMap,
		Stage1Publish,
		Stage1Display,
		Stage1ClampU,
		Stage1ClampV,
		Stage1NoLOD,
		Stage1Frames,
		Stage1FPS,
		Stage1AnimMode,
		Stage1PassHint,
		Stage1AlphaBitmap,

		ObsoleteParam,
	};

	enum class W3DMaterialRefID : int
	{
		PassCountBlock = 1,
		PassOneBlock = 2,
		PassTwoBlock = 3,
		PassThreeBlock = 4,
		PassFourBlock = 5,
		Texmap1Block = 6,
		Texmap2Block = 7,
		Texmap3Block = 8,
		Texmap4Block = 9,
		Texmap5Block = 10,
		Texmap6Block = 11,
		Texmap7Block = 12,
		Texmap8Block = 13,
		SurfaceTypeBlock = 15,

		Num
	};

	class W3DMaterialPass
	{
	public:
		W3DMaterialPass(int DialogTitleResourceID)
			: ParamBlock(nullptr)
			, Dialog(nullptr)
			, DialogTitleResourceID(DialogTitleResourceID)
		{ }

		IParamBlock2*   ParamBlock;
		IAutoMParamDlg* Dialog;
		int             DialogTitleResourceID;
	};

	class W3DMaterial
		: public Mtl
		, MaxSDK::Graphics::ITextureDisplay
	{
	public:
		using W3DMaterialPassArray = std::array<W3DMaterialPass, 4>;

		W3DMaterial();
		W3DMaterial(BOOL loading);
		~W3DMaterial();

		int NumActivePasses();
		int GetSurfaceType();
		int GetSortLevel();
		W3DMaterialPass& GetMaterialPass(int i) { return m_Passes[i]; }
		W3DMaterialPassArray GetMaterialPasses() { return m_Passes; }

		void RefreshPasses();
		void ClearDisplayFlags();
		void InvalidateDisplayTexture();

		// From MtlBase and Mtl
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
		void      Update(TimeValue t, Interval& valid);
		Interval  Validity(TimeValue t);
		void      Reset();

		void NotifyChanged();
		void MaterialDirty();

		virtual BOOL SupportTexDisplay() override { return TRUE; }
		virtual BOOL SupportsMultiMapsInViewport() override { return TRUE; }
		virtual void ActivateTexDisplay(BOOL onoff) override { if (!onoff) DiscardTextureHandle(); }
		virtual BaseInterface* GetInterface(Interface_ID id) override;

		virtual void SetAmbient(Color c, TimeValue t);
		virtual void SetDiffuse(Color c, TimeValue t);
		virtual void SetSpecular(Color c, TimeValue t);
		virtual void SetShininess(float v, TimeValue t);
		virtual Color GetAmbient(int mtlNum = 0, BOOL backFace = FALSE);
		virtual Color GetDiffuse(int mtlNum = 0, BOOL backFace = FALSE);
		virtual Color GetSpecular(int mtlNum = 0, BOOL backFace = FALSE);
		virtual float GetXParency(int mtlNum = 0, BOOL backFace = FALSE);
		virtual float GetShininess(int mtlNum = 0, BOOL backFace = FALSE);
		virtual float GetShinStr(int mtlNum = 0, BOOL backFace = FALSE);

		// Inherited via ITextureDisplay
		virtual void SetupTextures(TimeValue t, MaxSDK::Graphics::DisplayTextureHelper & updater) override;

		virtual ULONG Requirements(int subMtlNum);

		// Shade and displacement calculation
		virtual void     Shade(ShadeContext& sc);
		virtual float    EvalDisplacement(ShadeContext& sc);

		// SubTexmap access methods
		virtual int     NumSubTexmaps() { return static_cast<int>(m_Passes.size() * 2); }
		virtual Texmap* GetSubTexmap(int i);
		virtual void    SetSubTexmap(int i, Texmap *m);
		virtual TSTR    GetSubTexmapSlotName(int i, bool localized);
		virtual TSTR    GetSubTexmapTVName(int i);

		virtual BOOL SetDlgThing(ParamDlg* dlg);

		// Loading/Saving
		virtual IOResult Load(ILoad *iload);
		virtual IOResult Save(ISave *isave);

		// From Animatable
		virtual Class_ID ClassID();
		virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
		virtual void GetClassName(TSTR& s, bool localized);

		virtual RefTargetHandle Clone(RemapDir &remap);
		virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);

		virtual int NumSubs() { return 0; }
		virtual Animatable* SubAnim(int i);
		virtual TSTR SubAnimName(int i, bool localized);

		virtual int NumRefs() { return enum_to_value(W3DMaterialRefID::Num); }
		virtual RefTargetHandle GetReference(int i);

		virtual int NumParamBlocks() { return enum_to_value(W3DMaterialBlockID::Num); } // return number of ParamBlocks in this instance
		virtual IParamBlock2* GetParamBlock(int i) { return GetParamBlockByID(i); }
		virtual IParamBlock2* GetParamBlockByID(BlockID id); // return id'd ParamBlock

		virtual void DeleteThis() { delete this; }

	private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
		void DiscardTextureHandle();
		std::pair<W3DMaterialPass&, BitmapTex*> GetDisplayPassAndBitmap();

		TexHandle*           m_ActiveTextureHandle;
		IParamBlock2*        m_SurfaceTypeBlock;
		IParamBlock2*        m_PassCountBlock;
		IAutoMParamDlg*      m_MasterDialog;
		HWND                 m_MtlDlgHandle;
		IMtlParams*          m_MtlParams;
		Interval             m_ValidInterval;
		Interval             m_TexHandleValidity;
		W3DMaterialPassArray m_Passes;
		std::array<Texmap *, 8> m_OldTexMaps;
	};



	class W3DMaterialClassDesc : public ClassDesc2
	{
	public:
		virtual int IsPublic() { return TRUE; }
		virtual void* Create(BOOL loading = FALSE) { return new W3DMaterial(loading); }
		virtual const TCHAR *	ClassName() { return _T("W3D"); }
		virtual const TCHAR *	NonLocalizedClassName() { return _T("W3D"); }
		virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
		virtual Class_ID ClassID() { return Class_ID(0x29397211, 0x28C016C2); }
		virtual const TCHAR* Category() { return _T(""); }

		virtual const TCHAR* InternalName() { return _T("W3DMaterial"); }	// returns fixed parsable name (scripter-visible name)
		virtual HINSTANCE HInstance() { return hInstance; }					// returns owning module handle

		static W3DMaterialClassDesc* Instance();

	private:
		W3DMaterialClassDesc() {}
	};
}

#endif // W3D_MAX_TOOLS_INCLUDE_W3D_MATERIAL_H