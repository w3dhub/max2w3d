#pragma once

#include <iparamm2.h>
#include "EnumUtilities.h"

namespace W3D::MaxTools
{
	class W3DMaterial;

	enum class W3DMaterialParamID : ParamID;

	class W3DMaterialPassDlgProc
		: public ParamMap2UserDlgProc
	{
	public:
		W3DMaterialPassDlgProc();

		void ClearDisplayFlags() { static_cast<TexturesTab*>(m_Tabs[2].get())->ClearDisplayFlags(); }
	private:
		void SetTabIndex(const int index);

		template <typename T>
		BOOL SetValue(W3DMaterialParamID param, T&& value)
		{
			return m_ParamBlock->SetValue(enum_to_value(param), 0, value);
		}

		void SetDisplayFlag(ICustButton& control, W3DMaterialParamID param);

		// Inherited via ParamMap2UserDlgProc
		virtual INT_PTR DlgProc(TimeValue t, IParamMap2 * map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
		virtual void SetThing(ReferenceTarget *m) override;
		virtual void SetParamBlock(IParamBlock2 *pb) override;
		virtual void DeleteThis() override;

		struct TabBase
		{
			TabBase(HWND root, W3DMaterialPassDlgProc& dialog)
				: m_Root(root)
				, m_Dialog(dialog)
			{
				SetWindowLongPtr(root, DWLP_USER, (LONG_PTR)this);
			}

			virtual ~TabBase() {}

			HWND m_Root;
			W3DMaterialPassDlgProc& m_Dialog;
		};

		struct VertexTab
			: public TabBase
		{
			VertexTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog);
			~VertexTab();

			static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

			IColorSwatch*    m_AmbientColour;
			IColorSwatch*    m_DiffuseColour;
			IColorSwatch*    m_SpecularColour;
			IColorSwatch*    m_EmissiveColour;
			ISpinnerControl* m_OpacitySpin;
			ISpinnerControl* m_TranslucencySpin;
			ISpinnerControl* m_ShininessSpin;
			ISpinnerControl* m_Stage0UVChanSpin;
			ISpinnerControl* m_Stage1UVChanSpin;
			HWND             m_Stage0MappingType;
			HWND             m_Stage1MappingType;
			HWND             m_Stage0Args;
			HWND             m_Stage1Args;
		};

		struct ShaderTab
			: public TabBase
		{
			ShaderTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog);
			~ShaderTab();

			void ResetBlendGradientDefaults();
			void UpdateFromBlendMode(HWND hwndDlg,int mode);
			void UpdateBlendMode(HWND hwndDlg);

			static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

			HWND m_BlendMode;
			HWND m_CustomSrcMode;
			HWND m_CustomDestMode;
			HWND m_Defaults;
			HWND m_PriGradient;
			HWND m_SecGradient;
			HWND m_DepthCmp;
			HWND m_DetailColour;
			HWND m_DetailAlpha;
		};

		struct TexturesTab
			: public TabBase
		{
			TexturesTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog);
			~TexturesTab();

			void UpdateTexture(ICustButton* textureButton, W3DMaterialParamID param);
			void SetStage0Enabled(bool enabled);
			void SetStage1Enabled(bool enabled);
			void ClearDisplayFlags();

			static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

			HWND             m_Stage0TextureEnabled;
			ICustButton*     m_Stage0TextureMap;
			ICustButton*     m_Stage0Publish;
			ICustButton*     m_Stage0Display;
			ICustButton*     m_Stage0ClampU;
			ICustButton*     m_Stage0ClampV;
			ICustButton*     m_Stage0NoLOD;
			ICustButton*     m_Stage0AlphaBitmap;
			ISpinnerControl* m_Stage0Frames;
			ISpinnerControl* m_Stage0FPS;
			HWND             m_Stage0PassHint;
			HWND             m_Stage0AnimMode;

			HWND             m_Stage1TextureEnabled;
			ICustButton*     m_Stage1TextureMap;
			ICustButton*     m_Stage1Publish;
			ICustButton*     m_Stage1Display;
			ICustButton*     m_Stage1ClampU;
			ICustButton*     m_Stage1ClampV;
			ICustButton*     m_Stage1NoLOD;
			ICustButton*     m_Stage1AlphaBitmap;
			ISpinnerControl* m_Stage1Frames;
			ISpinnerControl* m_Stage1FPS;
			HWND             m_Stage1PassHint;
			HWND             m_Stage1AnimMode;
		};

		W3DMaterial*     m_Material;
		IParamBlock2*    m_ParamBlock;
		HWND             m_TabControlHandle;

		std::array<std::unique_ptr<TabBase>, 3> m_Tabs;
	};
}