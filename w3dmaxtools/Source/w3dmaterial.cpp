#include "general.h"
#include <stdmat.h>
#include "w3dmaterial.h"

#include "Dialog/w3dmaterialpassdlg.h"

#include "resource.h"
#include "w3d.h"
#include "engine_string.h"


extern HINSTANCE hInstance;

namespace W3D::MaxTools
{

	class PassCountParamBlockProc
		: public ParamMap2UserDlgProc
	{
	public:
		static PassCountParamBlockProc & Instance()
		{
			static PassCountParamBlockProc s_instance;
			return s_instance;
		}

		void SetThing(ReferenceTarget *m) override
		{
			m_Material = static_cast<W3DMaterial*>(m);
		}

	private:
		PassCountParamBlockProc()
			: m_Material(nullptr)
			, m_ParamBlock(nullptr)
		{ }

		// Inherited via ParamMap2UserDlgProc
		virtual INT_PTR DlgProc(TimeValue t, IParamMap2 * map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
		{
			switch (msg)
			{
			case WM_INITDIALOG:
			{
				m_ParamBlock = map->GetParamBlock();
				SetPassCountText(hWnd, t);
				break;
			}
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_BTN_PASS_COUNT_CHANGE:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						s_CurrentPassCount = m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PassCount), t);
						if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_W3D_MAT_PASS_COUNT_POPUP), hWnd, (DLGPROC)PassCountChangeProc) == IDOK)
						{
							m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::PassCount), t, s_CurrentPassCount);
							SetPassCountText(hWnd, t);
							m_Material->RefreshPasses();
						}
					}
					break;
				}
				break;
			}
			return 0;
		}

		void SetPassCountText(HWND hWnd, TimeValue t)
		{
			BOOL result = SetDlgItemInt(hWnd, IDC_PASS_COUNT_DISPLAY_EDIT, m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PassCount), t), FALSE);
			assert(result);
		}


		virtual void SetParamBlock(IParamBlock2* pb) override
		{
			m_ParamBlock = pb;
		}

		virtual void DeleteThis() override
		{ }

		static BOOL CALLBACK PassCountChangeProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_INITDIALOG:
			{
				HWND updwnscrl = GetDlgItem(hwndDlg, IDC_PASS_COUNT_SPIN);
				assert(updwnscrl != nullptr);
				if (updwnscrl)
				{
					SendMessage(updwnscrl, UDM_SETRANGE, 0, MAKELPARAM(4, 1));
					SendMessage(updwnscrl, UDM_SETPOS, 0, s_CurrentPassCount);
				}
				break;
			}
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDOK:
				{
					BOOL result = FALSE;
					int new_pass_count = GetDlgItemInt(hwndDlg, IDC_PASS_COUNT_SPIN_EDIT, &result, FALSE);
					if (result)
						s_CurrentPassCount = new_pass_count;
				}
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					return TRUE;
				}
			}
			return FALSE;
		}

		static int s_CurrentPassCount;

		W3DMaterial*  m_Material;
		IParamBlock2* m_ParamBlock;
	};

	int PassCountParamBlockProc::s_CurrentPassCount = 1;

	static ParamBlockDesc2 s_W3DMatSurfaceTypeParamBlock(enum_to_value(W3DMaterialBlockID::SurfaceType), _T("surfacetypeblock"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT + P_AUTO_UI, enum_to_value(W3DMaterialRefID::SurfaceTypeBlock),
		//rollout
		IDD_W3D_MAT_SURFACE_TYPE, IDS_MATERIAL_SURFACE_TYPE, 0, 0, nullptr,
		//params
		enum_to_value(W3DMaterialParamID::SurfaceType), _T("surfacetype"), TYPE_INT, 0, IDS_SURFACE_TYPE, 
		p_ui, TYPE_INT_COMBOBOX, IDC_SURFACE_TYPE, enum_to_value(SURFACE_TYPE::MAX),
		IDS_SURFACE_TYPE_LIGHT_METAL,
		IDS_SURFACE_TYPE_HEAVY_METAL,
		IDS_SURFACE_TYPE_WATER,
		IDS_SURFACE_TYPE_SAND,
		IDS_SURFACE_TYPE_DIRT,
		IDS_SURFACE_TYPE_MUD,
		IDS_SURFACE_TYPE_GRASS,
		IDS_SURFACE_TYPE_WOOD,
		IDS_SURFACE_TYPE_CONCRETE,
		IDS_SURFACE_TYPE_FLESH,
		IDS_SURFACE_TYPE_ROCK,
		IDS_SURFACE_TYPE_SNOW,
		IDS_SURFACE_TYPE_ICE,
		IDS_SURFACE_TYPE_DEFAULT,
		IDS_SURFACE_TYPE_GLASS,
		IDS_SURFACE_TYPE_CLOTH,
		IDS_SURFACE_TYPE_TIBERIUM_FIELD,
		IDS_SURFACE_TYPE_FOLIAGE_PERMEABLE,
		IDS_SURFACE_TYPE_GLASS_PERMEABLE,
		IDS_SURFACE_TYPE_ICE_PERMEABLE,
		IDS_SURFACE_TYPE_CLOTH_PERMEABLE,
		IDS_SURFACE_TYPE_ELECTRICAL,
		IDS_SURFACE_TYPE_ELECTRICAL_PERMEABLE,
		IDS_SURFACE_TYPE_FLAMMABLE,
		IDS_SURFACE_TYPE_FLAMMABLE_PERMEABLE,
		IDS_SURFACE_TYPE_STEAM,
		IDS_SURFACE_TYPE_STEAM_PERMEABLE,
		IDS_SURFACE_TYPE_WATER_PERMEABLE,
		IDS_SURFACE_TYPE_TIBERIUM_WATER,
		IDS_SURFACE_TYPE_TIBERIUM_WATER_PERMEABLE,
		IDS_SURFACE_TYPE_UNDERWATER_DIRT,
		IDS_SURFACE_TYPE_UNDERWATER_TIBERIUM_DIRT,
		IDS_SURFACE_TYPE_BLUE_TIBERIUM,
		IDS_SURFACE_TYPE_RED_TIBERIUM,
		IDS_SURFACE_TYPE_TIBERIUM_VEINS,
		IDS_SURFACE_TYPE_LASER,
		IDS_SURFACE_TYPE_SNOW_PERMIABLE,
		IDS_SURFACE_TYPE_ELECTRICAL_GLASS,
		IDS_SURFACE_TYPE_ELECTRICAL_GLASS_PERMEABLE,
		IDS_SURFACE_TYPE_SLUSH,
		IDS_SURFACE_TYPE_EXTRA_1,
		IDS_SURFACE_TYPE_EXTRA_2,
		IDS_SURFACE_TYPE_EXTRA_3,
		IDS_SURFACE_TYPE_EXTRA_4,
		IDS_SURFACE_TYPE_EXTRA_5,
		IDS_SURFACE_TYPE_EXTRA_6,
		IDS_SURFACE_TYPE_EXTRA_7,
		IDS_SURFACE_TYPE_EXTRA_8,
		p_default, 0,
		p_end,

		enum_to_value(W3DMaterialParamID::StaticSortOn), _T("staticsorton"), TYPE_BOOL, 0, IDS_STATIC_SORTING_LEVEL,
		p_default, false,
		p_ui, TYPE_SINGLECHECKBOX, IDC_STATIC_SORT_ON,
		p_end,

		enum_to_value(W3DMaterialParamID::StaticSortLevel), _T("staticsortlvl"), TYPE_INT, 0, IDS_STATIC_SORTING_LEVEL,
		p_default, 0,
		p_range, 0, 32,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_STATIC_SORT_LVL_EDIT, IDC_STATIC_SORT_LVL_SPIN, 1,
		p_end,

		p_end
	);

	static ParamBlockDesc2 s_W3DMatPassCountParamBlock(enum_to_value(W3DMaterialBlockID::PassCount), _T("passcountblock"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT + P_AUTO_UI, enum_to_value(W3DMaterialRefID::PassCountBlock),

		//rollout
		IDD_W3D_MAT_PASS_COUNT, IDS_MATERIAL_PASS_COUNT, 0, 0, &PassCountParamBlockProc::Instance(),

		//params
		enum_to_value(W3DMaterialParamID::PassCount), _T("passcount"), TYPE_INT, 0, IDS_MATERIAL_PASS_COUNT,
		p_default, 1,
		p_range, 1, 4,
		p_end,

		p_end
	);

	static ParamBlockDesc2 s_W3DMatPassOneParamBlock(enum_to_value(W3DMaterialBlockID::PassOne), _T("passone"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT, enum_to_value(W3DMaterialRefID::PassOneBlock),

		//params

		//Vertex Tab
		enum_to_value(W3DMaterialParamID::AmbientColour), _T("ambientcolour"), TYPE_RGBA, 0, IDS_AMBIENT_COLOUR,
		p_default, RGBA(1.0f, 1.0f, 1.0f),
		p_end,

		enum_to_value(W3DMaterialParamID::DiffuseColour), _T("diffusecolour"), TYPE_RGBA, 0, IDS_DIFFUSE_COLOUR,
		p_default, RGBA(1.0f, 1.0f, 1.0f),
		p_end,

		enum_to_value(W3DMaterialParamID::SpecularColour), _T("specularcolour"), TYPE_RGBA, 0, IDS_SPECULAR_COLOUR,
		p_default, RGBA(0.0f, 0.0f, 0.0f),
		p_end,
			
		enum_to_value(W3DMaterialParamID::EmissiveColour), _T("emissivecolour"), TYPE_RGBA, 0, IDS_EMISSIVE_COLOUR,
		p_default, RGBA(0.0f, 0.0f, 0.0f),
		p_end,

		enum_to_value(W3DMaterialParamID::SpecularToDiffuse), _T("speculartodiffuse"), TYPE_BOOL, 0, IDS_SPECULAR_TO_DIFFUSE,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Opacity), _T("opacity"), TYPE_FLOAT, 0, IDS_OPACITY,
		p_range, 0.0f, 1.0f,
		p_default, 1.0f,
		p_end,
			
		enum_to_value(W3DMaterialParamID::Translucency), _T("translucency"), TYPE_FLOAT, 0, IDS_TRANSLUCENCY,
		p_range, 0.0f, 1.0f,
		p_default, 0.0f,
		p_end,

		enum_to_value(W3DMaterialParamID::Shininess), _T("shininess"), TYPE_FLOAT, 0, IDS_SHININESS,
		p_range, 0.0f, 1.0f,
		p_default, 1.0f,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0Mapping), _T("stage0mapping"), TYPE_INT, 0, IDS_STAGE_0_MAPPING,
		p_range, 0, enum_to_value(W3DMaterialMappingType::Num),
		p_default, 0,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0MappingArgs), _T("stage0mappingargs"), TYPE_STRING, 0, IDS_STAGE_0_MAPPING,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel), _T("stage0mappinguvchannel"), TYPE_INT, 0, IDS_STAGE_0_MAPPING_UV_CHANNEL,
		p_default, 1,
		p_range, 1, 99,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1Mapping), _T("stage1mapping"), TYPE_INT, 0, IDS_STAGE_1_MAPPING,
		p_range, 0, enum_to_value(W3DMaterialMappingType::Num),
		p_default, 0,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1MappingArgs), _T("stage1mappingargs"), TYPE_STRING, 0, IDS_STAGE_1_MAPPING,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel), _T("stage1mappinguvchannel"), TYPE_INT, 0, IDS_STAGE_1_MAPPING_UV_CHANNEL,
		p_default, 1,
		p_range, 1, 99,
		p_end,

		//Shader Tab
		enum_to_value(W3DMaterialParamID::BlendMode), _T("blendmode"), TYPE_INT, 0, IDS_BLEND_MODE,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialBlendMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::CustomSrcMode), _T("blendmodesrc"), TYPE_INT, 0, IDS_BLEND_MODE_SRC,
		p_default, 1,
		p_range, 0, enum_to_value(W3DMaterialBlendModeSrcType::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::CustomDestMode), _T("blendmodedest"), TYPE_INT, 0, IDS_BLEND_MODE_DEST,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialBlendModeDestType::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::BlendWriteZBuffer), _T("customblendwritezbuffer"), TYPE_BOOL, 0, IDS_BLEND_WRITE_Z_BUFFER,
		p_default, true,
		p_end,

		enum_to_value(W3DMaterialParamID::AlphaTest), _T("alphatest"), TYPE_BOOL, 0, IDS_ALPHA_TEST,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::PriGradient), _T("prigradient"), TYPE_INT, 0, IDS_PRIMARY_GRADIENT,
		p_default, enum_to_value(W3DMaterialPrimaryGradientMode::Modulate),
		p_range, 0, enum_to_value(W3DMaterialPrimaryGradientMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::SecGradient), _T("secgradient"), TYPE_BOOL, 0, IDS_SECONDARY_GRADIENT,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::DepthCmp), _T("depthcmp"), TYPE_INT, 0, IDS_DEPTH_COMPARISON,
		p_default, enum_to_value(W3DMaterialDepthCompMode::PassLEqual),
		p_range, 0, enum_to_value(W3DMaterialDepthCompMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::DetailColour), _T("detailcolour"), TYPE_INT, 0, IDS_DETAIL_COLOUR,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialDetailColourMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::DetailAlpha), _T("detailalpha"), TYPE_INT, 0, IDS_DETAIL_ALPHA,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialDetailAlphaMode::Num),
		p_end,

		//Textures Tab - Stage 0
		enum_to_value(W3DMaterialParamID::Stage0TextureEnabled), _T("stage0texenabled"), TYPE_BOOL, 0, IDS_STAGE_0_TEXTURE,
		p_default, false,
		p_end,
			
		enum_to_value(W3DMaterialParamID::Stage0TextureMap), _T("stage0texturemap"), TYPE_TEXMAP, 0, IDS_STAGE_0_TEXTURE,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0Publish), _T("stage0publish"), TYPE_BOOL, 0, IDS_PUBLISH,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0Display), _T("stage0display"), TYPE_BOOL, 0, IDS_DISPLAY,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0ClampU), _T("stage0clampu"), TYPE_BOOL, 0, IDS_CLAMP_U,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0ClampV), _T("stage0clampv"), TYPE_BOOL, 0, IDS_CLAMP_V,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0NoLOD), _T("stage0nolod"), TYPE_BOOL, 0, IDS_NO_LOD,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0Frames), _T("stage0frames"), TYPE_INT, 0, IDS_ANIM_FRAMES,
		p_default, 1,
		p_range, 0, 999,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0FPS), _T("stage0fps"), TYPE_FLOAT, 0, IDS_ANIM_FPS,
		p_default, 15.0f,
		p_range, 0.0f, 60.0f,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0AnimMode), _T("stage0animmode"), TYPE_INT, 0, IDS_ANIM_MODE,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialTextureAnimMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0PassHint), _T("stage0passhint"), TYPE_INT, 0, IDS_PASS_HINT,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialTexturePassHint::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::Stage0AlphaBitmap), _T("stage0alphabitmap"), TYPE_BOOL, 0, IDS_ALPHA_BITMAP,
		p_default, false,
		p_end,

		//Textures Tab - Stage 1
		enum_to_value(W3DMaterialParamID::Stage1TextureEnabled), _T("stage1texenabled"), TYPE_BOOL, 0, IDS_STAGE_1_TEXTURE,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1TextureMap), _T("stage1texturemap"), TYPE_TEXMAP, 0, IDS_STAGE_1_TEXTURE,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1Publish), _T("stage1publish"), TYPE_BOOL, 0, IDS_PUBLISH,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1Display), _T("stage1display"), TYPE_BOOL, 0, IDS_DISPLAY,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1ClampU), _T("stage1clampu"), TYPE_BOOL, 0, IDS_CLAMP_U,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1ClampV), _T("stage1clampv"), TYPE_BOOL, 0, IDS_CLAMP_V,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1NoLOD), _T("stage1nolod"), TYPE_BOOL, 0, IDS_NO_LOD,
		p_default, false,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1Frames), _T("stage1frames"), TYPE_INT, 0, IDS_ANIM_FRAMES,
		p_default, 1,
		p_range, 0, 999,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1FPS), _T("stage1fps"), TYPE_FLOAT, 0, IDS_ANIM_FPS,
		p_default, 15.0f,
		p_range, 0.0f, 60.0f,
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1AnimMode), _T("stage1animmode"), TYPE_INT, 0, IDS_ANIM_MODE,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialTextureAnimMode::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1PassHint), _T("stage1passhint"), TYPE_INT, 0, IDS_PASS_HINT,
		p_default, 0,
		p_range, 0, enum_to_value(W3DMaterialTexturePassHint::Num),
		p_end,

		enum_to_value(W3DMaterialParamID::Stage1AlphaBitmap), _T("stage1alphabitmap"), TYPE_BOOL, 0, IDS_ALPHA_BITMAP,
		p_default, false,
		p_end,

		p_end
	);

	static ParamBlockDesc2 s_W3DMatPassTwoParamBlock(enum_to_value(W3DMaterialBlockID::PassTwo), _T("passtwo"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT + P_USE_PARAMS, enum_to_value(W3DMaterialRefID::PassTwoBlock),

		//Base
		&s_W3DMatPassOneParamBlock
	);

	static ParamBlockDesc2 s_W3DMatPassThreeParamBlock(enum_to_value(W3DMaterialBlockID::PassThree), _T("passthree"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT + P_USE_PARAMS, enum_to_value(W3DMaterialRefID::PassThreeBlock),

		//Base
		&s_W3DMatPassOneParamBlock
	);

	static ParamBlockDesc2 s_W3DMatPassFourParamBlock(enum_to_value(W3DMaterialBlockID::PassFour), _T("passfour"), 0, W3DMaterialClassDesc::Instance(),
		P_AUTO_CONSTRUCT + P_USE_PARAMS, enum_to_value(W3DMaterialRefID::PassFourBlock),

		//Base
		&s_W3DMatPassOneParamBlock
	);
		
	static const std::array<ParamBlockDesc2*, 4> s_W3DMatPassBlocks { &s_W3DMatPassOneParamBlock, &s_W3DMatPassTwoParamBlock, &s_W3DMatPassThreeParamBlock, &s_W3DMatPassFourParamBlock };

	ParamBlockDescID passcount[] = { { TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::PassCount) } };
	ParamBlockDescID materialpass[] = {
		{TYPE_POINT3, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::AmbientColour) },
		{TYPE_POINT3, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::DiffuseColour) },
		{TYPE_POINT3, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::SpecularColour) },
		{TYPE_POINT3, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::EmissiveColour) },
		{TYPE_FLOAT, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::Shininess) },
		{TYPE_FLOAT, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::Opacity) },
		{TYPE_FLOAT, nullptr, 1, (DWORD)enum_to_value(W3DMaterialParamID::Translucency) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::SpecularToDiffuse) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0Mapping) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::DepthCmp) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::BlendWriteZBuffer) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::CustomDestMode) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::PriGradient) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::SecGradient) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::CustomSrcMode) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::DetailColour) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::DetailAlpha) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0TextureEnabled) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0Publish) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0ClampU) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0ClampV) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0PassHint) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0Display) },
		{TYPE_FLOAT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0FPS) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0Frames) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0AnimMode) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1TextureEnabled) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1Publish) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1ClampU) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1ClampV) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1PassHint) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1Display) },
		{TYPE_FLOAT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1FPS) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1Frames) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1AnimMode) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0AlphaBitmap) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1AlphaBitmap) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::AlphaTest) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::BlendMode) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::ObsoleteParam) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel) },
		{TYPE_INT, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1Mapping) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage0NoLOD) },
		{TYPE_BOOL, nullptr, 0, (DWORD)enum_to_value(W3DMaterialParamID::Stage1NoLOD) }
	};

	class W3DMaterialPostLoad : public PostLoadCallback
	{
	public:
		W3DMaterial *mat;
		char *mapperargs[4][2];
		int surfacetype;
		int sortlevel;
		bool old;
		W3DMaterialPostLoad(W3DMaterial *m) : mat(m), mapperargs{}, surfacetype(0), sortlevel(0), old(false)
		{
		}

		void proc(ILoad *iload)
		{
			if (old)
			{
				mat->ReplaceReference(enum_to_value(W3DMaterialRefID::PassCountBlock), UpdateParameterBlock2(passcount, sizeof(passcount) / sizeof(passcount[0]), (IParamBlock *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassCountBlock)), &s_W3DMatPassCountParamBlock));
				mat->ReplaceReference(enum_to_value(W3DMaterialRefID::PassOneBlock), UpdateParameterBlock2(materialpass, sizeof(materialpass) / sizeof(materialpass[0]), (IParamBlock *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassOneBlock)), &s_W3DMatPassOneParamBlock));
				mat->ReplaceReference(enum_to_value(W3DMaterialRefID::PassTwoBlock), UpdateParameterBlock2(materialpass, sizeof(materialpass) / sizeof(materialpass[0]), (IParamBlock *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassTwoBlock)), &s_W3DMatPassTwoParamBlock));
				mat->ReplaceReference(enum_to_value(W3DMaterialRefID::PassThreeBlock), UpdateParameterBlock2(materialpass, sizeof(materialpass) / sizeof(materialpass[0]), (IParamBlock *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassThreeBlock)), &s_W3DMatPassThreeParamBlock));
				mat->ReplaceReference(enum_to_value(W3DMaterialRefID::PassFourBlock), UpdateParameterBlock2(materialpass, sizeof(materialpass) / sizeof(materialpass[0]), (IParamBlock *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassFourBlock)), &s_W3DMatPassFourParamBlock));
				BitmapTex *tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap1Block));
				if (tex)
				{
					mat->SetSubTexmap(0, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap1Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(0).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap2Block));
				if (tex)
				{
					mat->SetSubTexmap(1, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap2Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(0).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap3Block));
				if (tex)
				{
					mat->SetSubTexmap(2, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap3Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(1).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap4Block));
				if (tex)
				{
					mat->SetSubTexmap(3, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap4Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(1).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap5Block));
				if (tex)
				{
					mat->SetSubTexmap(5, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap5Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(2).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap6Block));
				if (tex)
				{
					mat->SetSubTexmap(4, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap6Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(2).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap7Block));
				if (tex)
				{
					mat->SetSubTexmap(7, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap7Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(3).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel)));
					}
				}
				tex = (BitmapTex *)mat->GetReference(enum_to_value(W3DMaterialRefID::Texmap8Block));
				if (tex)
				{
					mat->SetSubTexmap(6, tex);
					mat->ReplaceReference(enum_to_value(W3DMaterialRefID::Texmap8Block), nullptr);
					auto uvgen = tex->GetTheUVGen();
					if (uvgen)
					{
						uvgen->SetMapChannel(mat->GetMaterialPass(3).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel)));
					}
				}
				if (mapperargs[0][0])
				{
					WideStringClass str = mapperargs[0][0];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassOneBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage0MappingArgs), 0, str);
				}
				if (mapperargs[0][1])
				{
					WideStringClass str = mapperargs[0][1];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassOneBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage1MappingArgs), 0, str);
				}
				if (mapperargs[1][0])
				{
					WideStringClass str = mapperargs[1][0];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassTwoBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage0MappingArgs), 0, str);
				}
				if (mapperargs[1][1])
				{
					WideStringClass str = mapperargs[1][1];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassTwoBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage1MappingArgs), 0, str);
				}
				if (mapperargs[2][0])
				{
					WideStringClass str = mapperargs[2][0];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassThreeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage0MappingArgs), 0, str);
				}
				if (mapperargs[2][1])
				{
					WideStringClass str = mapperargs[2][1];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassThreeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage1MappingArgs), 0, str);
				}
				if (mapperargs[3][0])
				{
					WideStringClass str = mapperargs[3][0];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassFourBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage0MappingArgs), 0, str);
				}
				if (mapperargs[3][1])
				{
					WideStringClass str = mapperargs[3][1];
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::PassFourBlock)))->SetValue(enum_to_value(W3DMaterialParamID::Stage1MappingArgs), 0, str);
				}
				((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::SurfaceType), 0, surfacetype);
				((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::StaticSortLevel), 0, sortlevel);
				if (sortlevel != 0)
				{
					((IParamBlock2 *)mat->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::StaticSortOn), 0, true);
				}
				mat->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, true);
				mat->NotifyChanged();
			}
			delete this;
		}
		int Priority() { return 0; }
		INT_PTR Execute(int cmd, ULONG_PTR arg1 = 0, ULONG_PTR arg2 = 0, ULONG_PTR arg3 = 0)
		{
			return 0;
		}
	};

	W3DMaterial::W3DMaterial()
		: m_ActiveTextureHandle(nullptr)
		, m_SurfaceTypeBlock(nullptr)
		, m_PassCountBlock(nullptr)
		, m_MasterDialog(nullptr)
		, m_MtlDlgHandle(nullptr)
		, m_MtlParams(nullptr)
		, m_ValidInterval()
		, m_TexHandleValidity()
		, m_Passes({IDS_PASS1, IDS_PASS2, IDS_PASS3, IDS_PASS4})
	{
		W3DMaterialClassDesc::Instance()->MakeAutoParamBlocks(this);
		SetMtlFlag(MTL_HW_TEX_ENABLED | MTL_DISPLAY_ENABLE_FLAGS, TRUE);

		W3DMaterial::Reset();
	}

	W3DMaterial::W3DMaterial(BOOL loading)
		: m_ActiveTextureHandle(nullptr)
		, m_SurfaceTypeBlock(nullptr)
		, m_PassCountBlock(nullptr)
		, m_MasterDialog(nullptr)
		, m_MtlDlgHandle(nullptr)
		, m_MtlParams(nullptr)
		, m_ValidInterval()
		, m_TexHandleValidity()
		, m_Passes({ IDS_PASS1, IDS_PASS2, IDS_PASS3, IDS_PASS4 })
	{
		W3DMaterialClassDesc::Instance()->MakeAutoParamBlocks(this);
		SetMtlFlag(MTL_HW_TEX_ENABLED | MTL_DISPLAY_ENABLE_FLAGS, TRUE);

		if (!loading)
			W3DMaterial::Reset();
	}

	W3DMaterial::~W3DMaterial()
	{
		DeleteAllRefs();
	}

	int W3DMaterial::NumActivePasses()
	{
		return m_PassCountBlock->GetInt(enum_to_value(W3DMaterialParamID::PassCount));
	}

	int W3DMaterial::GetSurfaceType()
	{
		return m_SurfaceTypeBlock->GetInt(enum_to_value(W3DMaterialParamID::SurfaceType));
	}

	int W3DMaterial::GetSortLevel()
	{
		return m_SurfaceTypeBlock->GetInt(enum_to_value(W3DMaterialParamID::StaticSortLevel));
	}

	void W3DMaterial::Reset()
	{
		m_ValidInterval.SetEmpty();
			
		W3DMaterialClassDesc::Instance()->Reset(this, true, true);
		RefreshPasses();
		m_OldTexMaps[0] = nullptr;
		m_OldTexMaps[1] = nullptr;
		m_OldTexMaps[2] = nullptr;
		m_OldTexMaps[3] = nullptr;
		m_OldTexMaps[4] = nullptr;
		m_OldTexMaps[5] = nullptr;
		m_OldTexMaps[6] = nullptr;
	}

	void W3DMaterial::RefreshPasses()
	{
		if (m_MasterDialog)
		{
			const int numActivePasses = NumActivePasses();

			for (int i = 0; i < m_Passes.size(); ++i)
			{
				W3DMaterialPass& pass = m_Passes[i];

				if (i < numActivePasses)
				{
					if (!pass.Dialog)
					{
						pass.Dialog = CreateAutoMParamDlg(m_MtlDlgHandle, m_MtlParams, this, pass.ParamBlock, W3DMaterialClassDesc::Instance(), hInstance, MAKEINTRESOURCE(IDD_W3D_MAT_PASS), s_W3DMatPassBlocks[i]->GetString(pass.DialogTitleResourceID), 0, new W3DMaterialPassDlgProc());
						m_MasterDialog->AddDlg(pass.Dialog);
						pass.Dialog->SetThing(this);
					}
				}
				else
				{
					if (pass.Dialog)
					{
						m_MasterDialog->DeleteDlg(pass.Dialog);
						DestroyMParamMap2(pass.Dialog->GetMap());
						pass.Dialog = nullptr;
					}
				}
			}
		}
	}

	void W3DMaterial::ClearDisplayFlags()
	{
		for (W3DMaterialPass& pass : m_Passes)
		{
			if (pass.ParamBlock)
			{
				if (pass.ParamBlock->GetMap())
				{
					W3DMaterialPassDlgProc* passDlgProc = static_cast<W3DMaterialPassDlgProc*>(pass.ParamBlock->GetMap()->GetUserDlgProc());
					passDlgProc->ClearDisplayFlags();
				}
				pass.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage0Display), 0, FALSE);
				pass.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage1Display), 0, FALSE);
			}
		}
	}

	void W3DMaterial::InvalidateDisplayTexture()
	{
		m_TexHandleValidity.SetEmpty();

		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		SetActiveTexmap(activeDisplay.second);

		GetCOREInterface()->RedrawViews(0);
	}

	ParamDlg* W3DMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
		m_MasterDialog = W3DMaterialClassDesc::Instance()->CreateParamDlgs(hwMtlEdit, imp, this);
		m_MtlParams = imp;
		m_MtlDlgHandle = hwMtlEdit;

		PassCountParamBlockProc::Instance().SetThing(this);

		for (W3DMaterialPass& pass : m_Passes)
		{
			pass.Dialog = nullptr;
		}

		RefreshPasses();

		return m_MasterDialog;
	}

	BOOL W3DMaterial::SetDlgThing(ParamDlg* /*dlg*/)
	{
		if (m_MasterDialog)
		{
			RefreshPasses();
		}

		return FALSE;
	}

	Interval W3DMaterial::Validity(TimeValue t)
	{
		Interval valid = FOREVER;

		m_SurfaceTypeBlock->GetValidity(t, valid);
		m_PassCountBlock->GetValidity(t, valid);
			
		for (W3DMaterialPass& pass : m_Passes)
		{
			if (pass.ParamBlock)
			{
				pass.ParamBlock->GetValidity(t, valid);
			}
		}

		return valid;
	}

	/*===========================================================================*\
	|	Sub-anim & References support
	\*===========================================================================*/

	RefTargetHandle W3DMaterial::GetReference(int i)
	{
		switch (static_cast<W3DMaterialRefID>(i))
		{
		case W3DMaterialRefID::SurfaceTypeBlock:
			return m_SurfaceTypeBlock;
		case W3DMaterialRefID::PassCountBlock:
			return m_PassCountBlock;
		case W3DMaterialRefID::PassOneBlock:
			return m_Passes[0].ParamBlock;
		case W3DMaterialRefID::PassTwoBlock:
			return m_Passes[1].ParamBlock;
		case W3DMaterialRefID::PassThreeBlock:
			return m_Passes[2].ParamBlock;
		case W3DMaterialRefID::PassFourBlock:
			return m_Passes[3].ParamBlock;
		case W3DMaterialRefID::Texmap1Block:
			return m_OldTexMaps[0];
		case W3DMaterialRefID::Texmap2Block:
			return m_OldTexMaps[1];
		case W3DMaterialRefID::Texmap3Block:
			return m_OldTexMaps[2];
		case W3DMaterialRefID::Texmap4Block:
			return m_OldTexMaps[3];
		case W3DMaterialRefID::Texmap5Block:
			return m_OldTexMaps[4];
		case W3DMaterialRefID::Texmap6Block:
			return m_OldTexMaps[5];
		case W3DMaterialRefID::Texmap7Block:
			return m_OldTexMaps[6];
		case W3DMaterialRefID::Texmap8Block:
			return m_OldTexMaps[7];
		case 0:
		case 14:
			break;
		default:
			MSTR str;
			str.printf(L"Unknown Reference %d", i);
			OutputDebugString(str);
			TT_INTERRUPT;
			break;
		}

		return nullptr;
	}

	IParamBlock2 * W3DMaterial::GetParamBlockByID(BlockID id)
	{
		switch (static_cast<W3DMaterialBlockID>(id))
		{
		case W3DMaterialBlockID::SurfaceType:
			return m_SurfaceTypeBlock;
		case W3DMaterialBlockID::PassCount:
			return m_PassCountBlock;
		case W3DMaterialBlockID::PassOne:
			return m_Passes[0].ParamBlock;
		case W3DMaterialBlockID::PassTwo:
			return m_Passes[1].ParamBlock;
		case W3DMaterialBlockID::PassThree:
			return m_Passes[2].ParamBlock;
		case W3DMaterialBlockID::PassFour:
			return m_Passes[3].ParamBlock;
		default:
			MSTR str;
			str.printf(L"Unknown Param Block %d", id);
			OutputDebugString(str);
			break;
		}

		return nullptr;
	}

	void W3DMaterial::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (static_cast<W3DMaterialRefID>(i))
		{
		case W3DMaterialRefID::SurfaceTypeBlock:
			m_SurfaceTypeBlock = (IParamBlock2 *)rtarg;
			break;
		case W3DMaterialRefID::PassCountBlock:
			m_PassCountBlock = (IParamBlock2 *)rtarg;
			break;
		case W3DMaterialRefID::PassOneBlock:
			m_Passes[0].ParamBlock = (IParamBlock2*)rtarg;
			break;
		case W3DMaterialRefID::PassTwoBlock:
			m_Passes[1].ParamBlock = (IParamBlock2*)rtarg;
			break;
		case W3DMaterialRefID::PassThreeBlock:
			m_Passes[2].ParamBlock = (IParamBlock2*)rtarg;
			break;
		case W3DMaterialRefID::PassFourBlock:
			m_Passes[3].ParamBlock = (IParamBlock2*)rtarg;
			break;
		case W3DMaterialRefID::Texmap1Block:
			m_OldTexMaps[0] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap2Block:
			m_OldTexMaps[1] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap3Block:
			m_OldTexMaps[2] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap4Block:
			m_OldTexMaps[3] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap5Block:
			m_OldTexMaps[4] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap6Block:
			m_OldTexMaps[5] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap7Block:
			m_OldTexMaps[6] = (Texmap *)rtarg;
			break;
		case W3DMaterialRefID::Texmap8Block:
			m_OldTexMaps[7] = (Texmap *)rtarg;
			break;
		default:
			MSTR str;
			str.printf(L"Unknown Reference Set %d", i);
			OutputDebugString(str);
			break;
		}
			
	}

	void W3DMaterial::DiscardTextureHandle()
	{
		if (m_ActiveTextureHandle)
		{
			m_ActiveTextureHandle->DeleteThis();
			m_ActiveTextureHandle = nullptr;
		}
	}

	std::pair<W3DMaterialPass&, BitmapTex*> W3DMaterial::GetDisplayPassAndBitmap()
	{
		for (W3DMaterialPass& pass : m_Passes)
		{
			if (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0Display)))
			{
				return { pass, static_cast<BitmapTex*>(pass.ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage0TextureMap))) };
			}
			else if (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1Display)))
			{
				return { pass, static_cast<BitmapTex*>(pass.ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage1TextureMap))) };
			}
		}

		return { m_Passes[0], nullptr };
	}

	TSTR W3DMaterial::SubAnimName(int i, bool localized)
	{
		return TSTR(_T(""));
	}

	Animatable* W3DMaterial::SubAnim(int i)
	{
		return nullptr;
	}

	RefResult W3DMaterial::NotifyRefChanged(const Interval& /*changeInt*/, RefTargetHandle hTarget,
		PartID& /*partID*/, RefMessage message, BOOL /*propagate*/)
	{
		switch (message) 
		{
		case REFMSG_CHANGE:
		{
			m_ValidInterval.SetEmpty();
			if (hTarget == m_SurfaceTypeBlock)
			{
				ParamID changing_param = m_SurfaceTypeBlock->LastNotifyParamID();
				s_W3DMatSurfaceTypeParamBlock.InvalidateUI(changing_param);
			}
			else if (hTarget == m_PassCountBlock)
			{
				ParamID changing_param = m_PassCountBlock->LastNotifyParamID();
				s_W3DMatSurfaceTypeParamBlock.InvalidateUI(changing_param);
			}
			else
			{
				for (int i = 0; i < m_Passes.size(); ++i)
				{
					if (hTarget == m_Passes[i].ParamBlock)
					{
						ParamID changing_param = m_Passes[i].ParamBlock->LastNotifyParamID();
						s_W3DMatPassBlocks[i]->InvalidateUI(changing_param);
					}
				}
			}
			break;
		}
		case REFMSG_TARGET_DELETED:
		{
			if (hTarget == m_SurfaceTypeBlock)
			{
				m_SurfaceTypeBlock = nullptr;
			}
			else if (hTarget == m_PassCountBlock)
			{
				m_PassCountBlock = nullptr;
			}
			else
			{
				for (int i = 0; i < m_Passes.size(); ++i)
				{
					if (hTarget == m_Passes[i].ParamBlock)
						m_Passes[i].ParamBlock = nullptr;
				}
			}
			break;
		}
		}
		return REF_SUCCEED;
	}

	/*===========================================================================*\
	|	Texmap get and set
	\*===========================================================================*/

	Texmap* W3DMaterial::GetSubTexmap(int i)
	{
		int passIdx = i / 2;
		if (passIdx >= 0 && passIdx < m_Passes.size())
		{
			W3DMaterialPass& pass = m_Passes[passIdx];
			return pass.ParamBlock->GetTexmap(enum_to_value((i % 2) != 0 ? W3DMaterialParamID::Stage1TextureMap : W3DMaterialParamID::Stage0TextureMap));
		}

		return nullptr;
	}

	void W3DMaterial::SetSubTexmap(int i, Texmap* m)
	{
		int passIdx = i / 2;
		if (passIdx >= 0 && passIdx < m_Passes.size())
		{
			W3DMaterialPass& pass = m_Passes[passIdx];
			pass.ParamBlock->SetValue(enum_to_value((i % 2) != 0 ? W3DMaterialParamID::Stage1TextureMap : W3DMaterialParamID::Stage0TextureMap), 0, m);
		}
	}

	TSTR W3DMaterial::GetSubTexmapSlotName(int /*i*/, bool localized)
	{
		return _T("");
	}

	TSTR W3DMaterial::GetSubTexmapTVName(int i)
	{
		// Return i'th sub-texture name
		return GetSubTexmapSlotName(i, false);
	}

	ULONG W3DMaterial::Requirements(int subMtlNum)
	{
		int ret = 0;
		for (int i = 0; i < NumSubTexmaps(); i++)
		{
			if (GetSubTexmap(i))
			{
				ret |= GetSubTexmap(i)->Requirements(subMtlNum);
			}
		}
		return ret | (MTLREQ_BGCOL & MTLREQ_NOATMOS);
	}

	/*===========================================================================*\
	|	Standard IO
	\*===========================================================================*/
#define MTL_HDR_CHUNK 0x4000

	IOResult W3DMaterial::Save(ISave* isave)
	{
		isave->BeginChunk(MTL_HDR_CHUNK);
		IOResult res = MtlBase::Save(isave);
		if (res != IO_OK)
			return res;
		isave->EndChunk();

		return IO_OK;
	}

	Class_ID W3DMaterial::ClassID()
	{
		return W3DMaterialClassDesc::Instance()->ClassID();
	}

	void W3DMaterial::GetClassName(TSTR & s, bool localized)
	{
		s = W3DMaterialClassDesc::Instance()->ClassName();
	}

	IOResult W3DMaterial::Load(ILoad* iload)
	{
		IOResult res;

		W3DMaterialPostLoad *postload = new W3DMaterialPostLoad(this);

		while (IO_OK == (res = iload->OpenChunk()))
		{
			int id = iload->CurChunkID();
			switch (id)
			{
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case 0:
			case 96:
			case 97:
			case 98:
			case 99:
				postload->old = true;
				break;
			case 112:
				{
					ULONG read;
					postload->mapperargs[0][0] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[0][0], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 113:
				{
					ULONG read;
					postload->mapperargs[1][0] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[1][0], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 114:
				{
					ULONG read;
					postload->mapperargs[2][0] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[2][0], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 115:
				{
					ULONG read;
					postload->mapperargs[3][0] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[3][0], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 256:
				{
					ULONG read;
					postload->mapperargs[0][1] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[0][1], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 257:
				{
					ULONG read;
					postload->mapperargs[1][1] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[1][1], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 258:
				{
					ULONG read;
					postload->mapperargs[2][1] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[2][1], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 259:
				{
					ULONG read;
					postload->mapperargs[3][1] = new char[iload->CurChunkLength()];
					iload->Read(postload->mapperargs[3][1], (ULONG)iload->CurChunkLength(), &read);
				}
				break;
			case 128:
				{
					ULONG read;
					iload->Read(&postload->surfacetype, 4, &read);
				}
				break;
			case 144:
				{
					ULONG read;
					iload->Read(&postload->sortlevel, 4, &read);
				}
				break;
			default:
				MSTR str;
				str.printf(L"Unknown Chunk %d", id);
				OutputDebugString(str);
				break;
			}

			iload->CloseChunk();
			if (res != IO_OK)
				return res;
		}
		iload->RegisterPostLoadCallback(postload);
		return IO_OK;
	}


	/*===========================================================================*\
	|	Updating and cloning
	\*===========================================================================*/

	RefTargetHandle W3DMaterial::Clone(RemapDir &remap)
	{
		W3DMaterial *mnew = new W3DMaterial(FALSE);
		*((MtlBase*)mnew) = *((MtlBase*)this);

		// First clone the parameter blocks
		mnew->ReplaceReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock), remap.CloneRef(m_SurfaceTypeBlock));
		mnew->ReplaceReference(enum_to_value(W3DMaterialRefID::PassCountBlock), remap.CloneRef(m_PassCountBlock));
			
		for (int i = 0; i < mnew->m_Passes.size(); ++i)
		{
			if (m_Passes[i].ParamBlock)
			{
				mnew->m_Passes[i] = m_Passes[i];
				mnew->ReplaceReference(enum_to_value(W3DMaterialRefID::PassOneBlock) + i, remap.CloneRef(m_Passes[i].ParamBlock));
			}
		}

		mnew->m_ValidInterval.SetEmpty();

		BaseClone(this, mnew, remap);
		return (RefTargetHandle)mnew;
	}

	void W3DMaterial::NotifyChanged()
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

	void W3DMaterial::MaterialDirty()
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		if (m_MtlParams)
		{
			m_MtlParams->MtlChanged();
		}
		else
		{
			GetCOREInterface()->RedrawViews(0);
		}
	}

	void W3DMaterial::Update(TimeValue t, Interval& valid)
	{
		if (!m_ValidInterval.InInterval(t))
		{
			valid &= m_ValidInterval;
			m_ValidInterval.SetInfinite();
		}

		for (W3DMaterialPass& pass : m_Passes)
		{
			BitmapTex* tex = static_cast<BitmapTex*>(pass.ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage0TextureMap)));
			if (tex)
				tex->Update(t, valid);

			tex = static_cast<BitmapTex*>(pass.ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage1TextureMap)));
			if (tex)
				tex->Update(t, valid);
		}

		valid &= m_ValidInterval;
	}

	/*===========================================================================*\
	|	Determine the characteristics of the material
	\*===========================================================================*/

	BaseInterface * W3DMaterial::GetInterface(Interface_ID id)
	{
		if (id == ITEXTURE_DISPLAY_INTERFACE_ID)
		{
			return static_cast<ITextureDisplay*>(this);
		}

		return Mtl::GetInterface(id);
	}

	void W3DMaterial::SetAmbient(Color c, TimeValue t)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		activeDisplay.first.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::AmbientColour),t,c);
	}

	void W3DMaterial::SetDiffuse(Color c, TimeValue t)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		activeDisplay.first.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DiffuseColour), t, c);
	}

	void W3DMaterial::SetSpecular(Color c, TimeValue t)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		activeDisplay.first.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SpecularColour), t, c);
	}

	void W3DMaterial::SetShininess(float f, TimeValue t)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		activeDisplay.first.ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Shininess), t, f);
	}

	Color W3DMaterial::GetAmbient(int mtlNum, BOOL backFace)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		return activeDisplay.first.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::AmbientColour));
	}

	Color W3DMaterial::GetDiffuse(int mtlNum, BOOL backFace)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		return activeDisplay.first.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::DiffuseColour));
	}

	Color W3DMaterial::GetSpecular(int mtlNum, BOOL backFace)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		return activeDisplay.first.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour));
	}

	float W3DMaterial::GetXParency(int mtlNum, BOOL backFace)
	{
		return 0.0f;
	}

	float W3DMaterial::GetShininess(int mtlNum, BOOL backFace)
	{
		const std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();
		return activeDisplay.first.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Shininess));
	}

	float W3DMaterial::GetShinStr(int mtlNum, BOOL backFace)
	{
		return 1.0f;
	}

	void W3DMaterial::SetupTextures(TimeValue t, MaxSDK::Graphics::DisplayTextureHelper & updater)
	{
		using namespace MaxSDK::Graphics;

		ISimpleMaterial* simpleMtl = reinterpret_cast<ISimpleMaterial*>(GetProperty(PROPID_SIMPLE_MATERIAL));

		//Initialise textures if required
		if (!m_TexHandleValidity.InInterval(t))
		{
			if (m_ActiveTextureHandle)
				DiscardTextureHandle();

			//Clear existing textures
			simpleMtl->ClearTextures();

			std::pair<W3DMaterialPass&, BitmapTex*> activeDisplay = GetDisplayPassAndBitmap();

			if (activeDisplay.second)
			{
				m_ActiveTextureHandle = updater.MakeHandle(activeDisplay.second->GetVPDisplayDIB(t, updater, m_TexHandleValidity));

				//Setup maps
				if (m_ActiveTextureHandle)
				{
					updater.UpdateTextureMapInfo(t, ISimpleMaterial::UsageDiffuse, activeDisplay.second);
					simpleMtl->SetTexture(m_ActiveTextureHandle, ISimpleMaterial::UsageDiffuse);
				}
			}
		}
	}

	/*===========================================================================*\
	|	Actual shading takes place
	\*===========================================================================*/

	void W3DMaterial::Shade(ShadeContext& sc)
	{
		if (gbufID)
		{
			sc.SetGBufferID(gbufID);
		}
		if (sc.mode == SCMODE_SHADOW)
		{
			sc.out.t.r = 0.0f;
			sc.out.t.g = 0.0f;
			sc.out.t.b = 0.0f;
		}
		else
		{
			Color bgcol;
			Color transp;
			sc.GetBGColor(bgcol, transp);
			for (int i = 0; i < NumActivePasses(); i++)
			{
				W3DMaterialPass &pass = GetMaterialPass(i);
				Color ambient = sc.ambientLight;
				Color diffuse(0, 0, 0);
				Color specular(0, 0, 0);
				for (int j = 0; j < sc.nLights; j++)
				{
					LightDesc *ld = sc.Light(j);
					Point3 normal = sc.Normal();
					Color light;
					Point3 dir;
					float dot_nl;
					float diffuseCoef;
					if (ld->Illuminate(sc, normal, light, dir, dot_nl, diffuseCoef))
					{
						if (dot_nl > 0.0f)
						{
							diffuse = light * dot_nl + diffuse;
						}
						Point3 reflect = sc.ReflectVector();
						float ref = dir.x * reflect.x + dir.y * reflect.y + dir.z * reflect.z;
						if (ref > 0.0f)
						{
							float shininess = pass.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Shininess), sc.CurTime());
							specular = light * pow(ref, shininess) + specular;
						}
					}
				}
				ambient = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::AmbientColour), sc.CurTime()) * ambient;
				diffuse = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::DiffuseColour), sc.CurTime()) * diffuse;
				specular = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour), sc.CurTime()) * specular;
				Color emissive = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::EmissiveColour), sc.CurTime());
				Color color = diffuse + ambient + emissive;
				if (color.r > 1.0f)
				{
					color.r = 1.0f;
				}
				if (color.g > 1.0f)
				{
					color.g = 1.0f;
				}
				if (color.b > 1.0f)
				{
					color.b = 1.0f;
				}
				Color specular2 = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour), sc.CurTime()) * specular;
				AColor map(1, 1, 1, 1);
				if (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0TextureEnabled)))
				{
					Texmap *t = GetSubTexmap(i * 2);
					if (t)
					{
						map = t->EvalColor(sc);
					}
				}
				if (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1TextureEnabled)))
				{
					Texmap *t = GetSubTexmap((i * 2) + 1);
					if (t)
					{
						AColor map2 = t->EvalColor(sc);
						switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailColour)))
						{
						case W3DSHADER_DETAILCOLORFUNC_DETAIL:
							map = map2;
							break;
						case W3DSHADER_DETAILCOLORFUNC_SCALE:
							map.r = map.r * map2.r;
							map.g = map.g * map2.g;
							map.b = map.b * map2.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_INVSCALE:
							map.r = (1.0f - map.r) * map2.r + map.r;
							map.g = (1.0f - map.g) * map2.g + map.g;
							map.b = (1.0f - map.b) * map2.b + map.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_ADD:
							map.r = map.r + map2.r;
							map.g = map.g + map2.g;
							map.b = map.b + map2.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_SUB:
							map.r = map.r - map2.r;
							map.g = map.g - map2.g;
							map.b = map.b - map2.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_SUBR:
							map.r = map2.r - map.r;
							map.g = map2.g - map.g;
							map.b = map2.b - map.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_BLEND:
						{
							float alpha = 1.0f - map.a;
							map.r = map2.r * alpha + map.a * map.r;
							map.g = map2.g * alpha + map.a * map.g;
							map.b = map2.b * alpha + map.a * map.b;
						}
						break;
						case W3DSHADER_DETAILCOLORFUNC_DETAILBLEND:
						{
							float alpha = 1.0f - map2.a;
							map.r = map2.r * alpha + map2.a * map.r;
							map.g = map2.g * alpha + map2.a * map.g;
							map.b = map2.b * alpha + map2.a * map.b;
						}
						break;
						case W3DSHADER_DETAILCOLORFUNC_ADDSIGNED:
							map.r = map.r * map2.r - 0.5f;
							map.g = map.g * map2.g - 0.5f;
							map.b = map.b * map2.b - 0.5f;
							break;
						case W3DSHADER_DETAILCOLORFUNC_ADDSIGNED2X:
							map.r = map.r * map2.r - 0.5f + map.r * map2.r - 0.5f;
							map.g = map.g * map2.g - 0.5f + map.g * map2.g - 0.5f;
							map.b = map.b * map2.b - 0.5f + map.b * map2.b - 0.5f;
							break;
						case W3DSHADER_DETAILCOLORFUNC_SCALE2X:
							map.r = map.r * map2.r + map.r * map2.r;
							map.g = map.g * map2.g + map.g * map2.g;
							map.b = map.b * map2.b + map.b * map2.b;
							break;
						case W3DSHADER_DETAILCOLORFUNC_MODALPHAADDCLR:
							map.r = map.a * map2.r + map.r;
							map.g = map.a * map2.g + map.g;
							map.b = map.a * map2.b + map.b;
							break;
						}
						switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailAlpha)))
						{
						case W3DSHADER_DETAILALPHAFUNC_DETAIL:
							map.a = map2.a;
							break;
						case W3DSHADER_DETAILALPHAFUNC_SCALE:
							map.a = map2.a * map.a;
							break;
						case W3DSHADER_DETAILALPHAFUNC_INVSCALE:
							map.a = (1.0f - map.a) * map2.a + map.a;
							break;
						}
					}
				}
				AColor map3;
				map3.r = map.r;
				map3.g = map.g;
				map3.b = map.b;
				map3.a = pass.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Opacity)) * map.a;
				switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PriGradient)))
				{
				case W3DSHADER_PRIGRADIENT_MODULATE:
					map3.r = map.r * color.r;
					map3.g = map.g * color.g;
					map3.b = map.b * color.b;
					break;
				case W3DSHADER_PRIGRADIENT_ADD:
					map3.r = map.r + color.r;
					map3.g = map.g + color.g;
					map3.b = map.b + color.b;
					break;
				case W3DSHADER_PRIGRADIENT_MODULATE2X:
					map3.r = (map.r * color.r) + (map.r * color.r);
					map3.g = (map.g * color.g) + (map.g * color.g);
					map3.b = (map.b * color.b) + (map.b * color.b);
					break;
				}
				switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SecGradient)))
				{
				case W3DSHADER_SECGRADIENT_ENABLE:
					map3.r = specular2.r + map3.r;
					map3.g = specular2.g + map3.g;
					map3.b = specular2.b + map3.b;
					break;
				}
				if (map3.r > 1.0f)
				{
					map3.r = 1.0f;
				}
				if (map3.g > 1.0f)
				{
					map3.g = 1.0f;
				}
				if (map3.b > 1.0f)
				{
					map3.b = 1.0f;
				}
				if (map3.r < 0.0f)
				{
					map3.r = 0.0f;
				}
				if (map3.g < 0.0f)
				{
					map3.g = 0.0f;
				}
				if (map3.b < 0.0f)
				{
					map3.b = 0.0f;
				}
				Color destblend;
				switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomDestMode)))
				{
				case W3DSHADER_DESTBLENDFUNC_ZERO:
					destblend.r = 0.0f;
					destblend.g = 0.0f;
					destblend.b = 0.0f;
					break;
				case W3DSHADER_DESTBLENDFUNC_ONE:
					destblend.r = 1.0f;
					destblend.g = 1.0f;
					destblend.b = 1.0f;
					break;
				case W3DSHADER_DESTBLENDFUNC_SRC_COLOR:
				case W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG:
					destblend.r = map3.r;
					destblend.g = map3.g;
					destblend.b = map3.b;
					break;
				case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR:
					destblend.r = 1.0f - map3.r;
					destblend.g = 1.0f - map3.g;
					destblend.b = 1.0f - map3.b;
					break;
				case W3DSHADER_DESTBLENDFUNC_SRC_ALPHA:
					destblend.r = map3.a;
					destblend.g = map3.a;
					destblend.b = map3.a;
					break;
				case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA:
					destblend.r = 1.0f - map3.a;
					destblend.g = 1.0f - map3.a;
					destblend.b = 1.0f - map3.a;
					break;
				}
				Color srcblend;
				switch (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomSrcMode)))
				{
				case W3DSHADER_SRCBLENDFUNC_ZERO:
					srcblend.r = 0.0;
					srcblend.g = 0.0;
					srcblend.b = 0.0;
					break;
				case W3DSHADER_SRCBLENDFUNC_ONE:
					srcblend.r = 1.0;
					srcblend.g = 1.0;
					srcblend.b = 1.0;
					break;
				case W3DSHADER_SRCBLENDFUNC_SRC_ALPHA:
					srcblend.r = map3.a;
					srcblend.g = map3.a;
					srcblend.b = map3.a;
					break;
				case W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA:
					srcblend.r = 1.0f - map3.a;
					srcblend.g = 1.0f - map3.a;
					srcblend.b = 1.0f - map3.a;
					break;
				}
				bgcol = srcblend * map3 + bgcol * destblend;
			}
			sc.out.t.r = 0;
			sc.out.t.g = 0;
			sc.out.t.b = 0;
			sc.out.c = bgcol;
		}
	}

	float W3DMaterial::EvalDisplacement(ShadeContext& sc)
	{
		return 0.0f;
	}

	// Class Desc
	W3DMaterialClassDesc * W3DMaterialClassDesc::Instance()
	{
		static W3DMaterialClassDesc s_instance;
		return &s_instance;
	}

	//String Lookups
	const TSTR& W3DMaterialMappingTypeString(W3DMaterialMappingType mappingType)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialMappingType::Num)> s_strings
		{
			_T("UV"),
			_T("Environment"),
			_T("ClassicEnvironment"),
			_T("Screen"),
			_T("LinearOffset"),
			_T("Silhouette"),
			_T("Scale"),
			_T("Grid"),
			_T("Rotate"),
			_T("Sine"),
			_T("Step"),
			_T("ZigZag"),
			_T("WSClassicEnv"),
			_T("WSEnvironment"),
			_T("GridClassicEnv"),
			_T("GridEnvironment"),
			_T("Random"),
			_T("Edge"),
			_T("BumpEnv"),
			_T("GridWSClassicEnv"),
			_T("GridWSEnv")
		};

		return s_strings[enum_to_value(mappingType)];
	}

	const TSTR& W3DMaterialBlendModeString(W3DMaterialBlendMode blendMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialBlendMode::Num)> s_strings
		{
			_T("Opaque"),
			_T("Add"),
			_T("Multiply"),
			_T("MultiplyAndAdd"),
			_T("Screen"),
			_T("AlphaBlend"),
			_T("AlphaTest"),
			_T("AlphaTestAndBlend"),
			_T("Custom")
		};

		return s_strings[enum_to_value(blendMode)];
	}

	const TSTR& W3DMaterialBlendModeSrcString(W3DMaterialBlendModeSrcType srcType)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialBlendModeSrcType::Num)> s_strings
		{
			_T("Zero"),
			_T("One"),
			_T("SrcAlpha"),
			_T("OneMinusSrcAlpha")
		};

		return s_strings[enum_to_value(srcType)];
	}

	const TSTR & W3DMaterialBlendModeDestString(W3DMaterialBlendModeDestType destType)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialBlendModeDestType::Num)> s_strings
		{
			_T("Zero"),
			_T("One"),
			_T("SrcColour"),
			_T("OneMinusSrcColour"),
			_T("SrcAlpha"),
			_T("OneMinusSrcAlpha"),
			_T("SrcColourPreFog")
		};

		return s_strings[enum_to_value(destType)];
	}

	const TSTR & W3DMaterialPrimaryGradientModeString(W3DMaterialPrimaryGradientMode gradientMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialPrimaryGradientMode::Num)> s_strings
		{
			_T("Disable"),
			_T("Modulate"),
			_T("Add"),
			_T("BumpEnvironment"),
			_T("BumpEnvLum"),
			_T("Mod2X")
		};

		return s_strings[enum_to_value(gradientMode)];
	}

	const TSTR & W3DMaterialDepthCompModeString(W3DMaterialDepthCompMode depthCompMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialDepthCompMode::Num)> s_strings
		{
			_T("PassNever"),
			_T("PassLess"),
			_T("PassEqual"),
			_T("PassLEqual"),
			_T("PassGreater"),
			_T("PassNEqual"),
			_T("PassGEqual"),
			_T("PassAlways")
		};

		return s_strings[enum_to_value(depthCompMode)];
	}

	const TSTR & W3DMaterialDetailColourModeString(W3DMaterialDetailColourMode detailColourMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialDetailColourMode::Num)> s_strings
		{
			_T("Disable"),
			_T("Detail"),
			_T("Scale"),
			_T("InvScale"),
			_T("Add"),
			_T("Sub"),
			_T("SubR"),
			_T("Blend"),
			_T("DetailBlend"),
			_T("AddSigned"),
			_T("AddSigned2X"),
			_T("Scale2X"),
			_T("ModAlphaAddClr")
		};

		return s_strings[enum_to_value(detailColourMode)];
	}

	const TSTR & W3DMaterialDetailAlphaModeString(W3DMaterialDetailAlphaMode detailAlphaMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialDetailAlphaMode::Num)> s_strings
		{
			_T("Disable"),
			_T("Detail"),
			_T("Scale"),
			_T("InvScale")
		};

		return s_strings[enum_to_value(detailAlphaMode)];
	}

	const TSTR & W3DMaterialTextureAnimModeString(W3DMaterialTextureAnimMode textureAnimMode)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialTextureAnimMode::Num)> s_strings
		{
			_T("Loop"),
			_T("PingPong"),
			_T("Once"),
			_T("Manual")
		};

		return s_strings[enum_to_value(textureAnimMode)];
	}

	const TSTR & W3DMaterialTexturePassHintString(W3DMaterialTexturePassHint texturePassHint)
	{
		static const std::array<const TSTR, enum_to_value(W3DMaterialTexturePassHint::Num)> s_strings
		{
			_T("BaseTexture"),
			_T("EmissiveLightMap"),
			_T("EnvironmentMap"),
			_T("ShinynessMap")
		};

		return s_strings[enum_to_value(texturePassHint)];
	}
}
