#include "general.h"
#include <stdmat.h>
#include "Dialog/w3dmaterialpassdlg.h"
#include "w3dmaterial.h"
#include "resource.h"

extern HINSTANCE hInstance;

namespace
{
	void UpdateTexmapButtonText(ICustButton* button, IParamBlock2& pb, W3D::MaxTools::W3DMaterialParamID param)
	{
		BitmapTex* tm = static_cast<BitmapTex*>(pb.GetTexmap(enum_to_value(param)));
		if (tm && tm->GetMapName())
		{
			MSTR path;
			MSTR fname;
			SplitPathFile(tm->GetMapName(), &path, &fname);
			button->SetText(fname.data());
		}
		else
		{
			button->SetText(_T("None"));
		}
	}

	ICustButton* InitCheckButton(HWND handle, IParamBlock2& pb, W3D::MaxTools::W3DMaterialParamID param)
	{
		ICustButton* btn = GetICustButton(handle);
		btn->SetType(CBT_CHECK);
		btn->SetCheck(pb.GetInt(enum_to_value(param)));

		return btn;
	}
}

namespace W3D::MaxTools
{
	W3DMaterialPassDlgProc::W3DMaterialPassDlgProc()
		: m_Material(nullptr), m_ParamBlock(nullptr)
		  , m_TabControlHandle(nullptr)
		  , m_Tabs()
	{
	}

	void W3DMaterialPassDlgProc::SetDisplayFlag(ICustButton& control, W3DMaterialParamID param)
	{
		const BOOL checked = control.IsChecked();
		if (checked)
		{
			m_Material->ClearDisplayFlags();
			control.SetCheck(checked);
		}

		SetValue(param, checked);
		m_Material->InvalidateDisplayTexture();
		m_Material->MaterialDirty();
	}

	INT_PTR W3DMaterialPassDlgProc::DlgProc(TimeValue t, IParamMap2 * map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			m_TabControlHandle = GetDlgItem(hWnd, IDC_PASS_TABS);
			m_ParamBlock = map->GetParamBlock();

			if (m_TabControlHandle)
			{

				TCITEM item;
				item.mask = TCIF_TEXT;

				item.pszText = L"Vertex Material";
				TabCtrl_InsertItem(m_TabControlHandle, 0, &item);
				item.pszText = L"Shader";
				TabCtrl_InsertItem(m_TabControlHandle, 1, &item);
				item.pszText = L"Textures";
				TabCtrl_InsertItem(m_TabControlHandle, 2, &item);

				m_Tabs[0] = std::make_unique<VertexTab>(CreateDialog(hInstance, MAKEINTRESOURCE(IDD_W3D_MAT_PASS_VERTEX), m_TabControlHandle, (DLGPROC)VertexTab::DlgProc), *this);
				m_Tabs[1] = std::make_unique<ShaderTab>(CreateDialog(hInstance, MAKEINTRESOURCE(IDD_W3D_MAT_PASS_SHADER), m_TabControlHandle, (DLGPROC)ShaderTab::DlgProc), *this);
				m_Tabs[2] = std::make_unique<TexturesTab>(CreateDialog(hInstance, MAKEINTRESOURCE(IDD_W3D_MAT_PASS_TEXTURES), m_TabControlHandle, (DLGPROC)TexturesTab::DlgProc), *this);

				RECT tabRect;
				GetClientRect(m_TabControlHandle, &tabRect);
				TabCtrl_AdjustRect(m_TabControlHandle, FALSE, &tabRect);

				for (auto& tab : m_Tabs)
				{
					MoveWindow(tab->m_Root, tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
				}

				SetTabIndex(TabCtrl_GetCurSel(m_TabControlHandle));

				return TRUE;
			}
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
			case TCN_SELCHANGE:
				SetTabIndex(TabCtrl_GetCurSel(m_TabControlHandle));
				return TRUE;
			}
			break;
		}

		return FALSE;
	}

	void W3DMaterialPassDlgProc::SetTabIndex(const int index)
	{
		for (int i = 0; i < m_Tabs.size(); ++i)
		{
			ShowWindow(m_Tabs[i]->m_Root, index == i ? SW_SHOW : SW_HIDE);
		}
	}

	void W3DMaterialPassDlgProc::SetThing(ReferenceTarget *m)
	{
		m_Material = static_cast<W3DMaterial*>(m);
	}

	void W3DMaterialPassDlgProc::SetParamBlock(IParamBlock2 * pb)
	{
		m_ParamBlock = pb;
	}

	void W3DMaterialPassDlgProc::DeleteThis()
	{
		delete this;
	}

	//// Vertex Tab ///////////////////////////////////////
	W3DMaterialPassDlgProc::VertexTab::VertexTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog)
		: TabBase(tabRoot, dialog)
	{
		m_AmbientColour = GetIColorSwatch(GetDlgItem(tabRoot, IDC_AMBIENT_COLOUR), dialog.m_ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::AmbientColour)), _T("Ambient Colour"));
		m_AmbientColour->SetNotifyAfterAccept(TRUE);

		m_DiffuseColour = GetIColorSwatch(GetDlgItem(tabRoot, IDC_DIFFUSE_COLOUR), dialog.m_ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::DiffuseColour)), _T("Diffuse Colour"));
		m_DiffuseColour->SetNotifyAfterAccept(TRUE);

		m_SpecularColour = GetIColorSwatch(GetDlgItem(tabRoot, IDC_SPECULAR_COLOUR), dialog.m_ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour)), _T("Specular Colour"));
		m_SpecularColour->SetNotifyAfterAccept(TRUE);

		m_EmissiveColour = GetIColorSwatch(GetDlgItem(tabRoot, IDC_EMISSIVE_COLOUR), dialog.m_ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::EmissiveColour)), _T("Emissive Colour"));
		m_EmissiveColour->SetNotifyAfterAccept(TRUE);

		m_OpacitySpin = SetupFloatSpinner(tabRoot, IDC_OPACITY_SPIN, IDC_OPACITY_EDIT, 0.0f, 1.0f, dialog.m_ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Opacity)));
		m_TranslucencySpin = SetupFloatSpinner(tabRoot, IDC_TRANSLUCENCY_SPIN, IDC_TRANSLUCENCY_EDIT, 0.0f, 1.0f, dialog.m_ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Translucency)));
		m_ShininessSpin = SetupFloatSpinner(tabRoot, IDC_SHININESS_SPIN, IDC_SHININESS_EDIT, 1.0f, 1000.0f, dialog.m_ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Shininess)));

		m_Stage0UVChanSpin = SetupIntSpinner(tabRoot, IDC_STAGE_0_UV_CHAN_SPIN, IDC_STAGE_0_UV_CHAN_EDIT, 1, 99, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0MappingUVChannel)));
		m_Stage1UVChanSpin = SetupIntSpinner(tabRoot, IDC_STAGE_1_UV_CHAN_SPIN, IDC_STAGE_1_UV_CHAN_EDIT, 1, 99, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1MappingUVChannel)));

		m_Stage0MappingType = GetDlgItem(tabRoot, IDC_STAGE_0_MAPPING_TYPE);
		m_Stage1MappingType = GetDlgItem(tabRoot, IDC_STAGE_1_MAPPING_TYPE);
		for (int i = 0; i < enum_to_value(W3DMaterialMappingType::Num); ++i)
		{
			ComboBox_AddString(m_Stage0MappingType, W3DMaterialMappingTypeString(static_cast<W3DMaterialMappingType>(i)));
			ComboBox_AddString(m_Stage1MappingType, W3DMaterialMappingTypeString(static_cast<W3DMaterialMappingType>(i)));
		}

		ComboBox_SetCurSel(m_Stage0MappingType, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0Mapping)));
		ComboBox_SetCurSel(m_Stage1MappingType, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1Mapping)));


		m_Stage0Args = GetDlgItem(tabRoot, IDC_STAGE_0_MAPPING_ARGS);
		SetWindowText(m_Stage0Args, dialog.m_ParamBlock->GetStr(enum_to_value(W3DMaterialParamID::Stage0MappingArgs)));

		m_Stage1Args = GetDlgItem(tabRoot, IDC_STAGE_1_MAPPING_ARGS);
		SetWindowText(m_Stage1Args, dialog.m_ParamBlock->GetStr(enum_to_value(W3DMaterialParamID::Stage1MappingArgs)));

		SetCheckBox(tabRoot, IDC_SPECULAR_TO_DIFFUSE, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SpecularToDiffuse)));
	}

	W3DMaterialPassDlgProc::VertexTab::~VertexTab()
	{
	}

	const wchar_t *defmapperargs[] = {
	L"", //UV
	L"", //Environment
	L"", //Classic Environment
	L"UPerSec=0.0\r\nVPerSec=0.0\r\nUOffset=0.0\r\nVOffset=0.0\r\nClampFix=false\r\nUScale=1.0\r\nVScale=1.0", //Screen
	L"UPerSec=0.0\r\nVPerSec=0.0\r\nUOffset=0.0\r\nVOffset=0.0\r\nClampFix=false\r\nUScale=1.0\r\nVScale=1.0", //Linear Offset
	L"Not Supported", //Silhouette
	L"UScale=1.0\r\nVScale=1.0", //Scale
	L"FPS=1.0; The frames per second\r\nLog2Width=1; So 0=width 1, 1=width 2, 2=width 4. The default means animate using a texture divided up into quarters.\r\nLast=GridWidth*GridWidth; The last frame to use.\r\nOffset=0", //Grid
	L"Speed=0.1; In Hertz. 1 = 1 rotate per second\r\nUCenter=0.0\r\nVCenter=0.0\r\nUScale=1.0\r\nVScale=1.0", //Rotate
	L"UAmp=1.0\r\nUFreq=1.0\r\nUPhase=0.0\r\nVAmp=1.0\r\nVFreq=1.0\r\nVPhase=0.0\r\nUScale=1.0\r\nVScale=1.0", //Sine Linear Offset
	L"UStep=0.0\r\nVStep=0.0\r\nSPS=0.0; Steps per second\r\nClampFix=false\r\nUScale=1.0\r\nVScale=1.0", //Step Linear Offset
	L"UPerSec=0.0\r\nVPerSec=0.0\r\nPeriod=0.0; Time it takes to make a zigzag in seconds\r\nUScale=1.0\r\nVScale=1.0", //ZigZag Linear Offset
	L"Axis=Z; Axis to use for this, X, Y, Z", //World Space Classic Environment
	L"Axis=Z; Axis to use for this, X, Y, Z", //World Space Environment
	L"FPS=1.0; The frames per second\r\nLog2Width=1; So 0=width 1, 1=width 2, 2=width 4. The default means animate using a texture divided up into quarters.\r\nLast=GridWidth*GridWidth; The last frame to use.\r\nOffset=0", //Grid Classic Environment
	L"FPS=1.0; The frames per second\r\nLog2Width=1; So 0=width 1, 1=width 2, 2=width 4. The default means animate using a texture divided up into quarters.\r\nLast=GridWidth*GridWidth; The last frame to use.\r\nOffset=0", //Grid Environment
	L"FPS=0.0; Frames per second\r\nUPerSec=0.0\r\nVPerSec=0.0\r\nUScale=1.0\r\nVScale=1.0", //Random
	L"VPerSec=0.0\r\nUseReflect=false\r\nVStart=0.0", //Edge
	L"BumpRotation = 0.1; In Hertz. 1 = 1 rotate per second  (DEFAULT = 0.0)\r\nBumpScale = scale factor applied to the bumps\t(DEFAULT = 1.0)\r\nUPerSec=0.0\r\nVPerSec=0.0\r\nUScale=1.0\r\nVScale=1.0", //Bump Environment
	L"FPS=1.0; The frames per second\r\nLog2Width=1; So 0=width 1, 1=width 2, 2=width 4. The default means animate using a texture divided up into quarters.\r\nLast=GridWidth*GridWidth; The last frame to use.\r\nOffset=0\r\nAxis=Z; Axis to use for this, X, Y, Z", //Grid World Space Classic Environment
	L"FPS=1.0; The frames per second\r\nLog2Width=1; So 0=width 1, 1=width 2, 2=width 4. The default means animate using a texture divided up into quarters.\r\nLast=GridWidth*GridWidth; The last frame to use.\r\nOffset=0\r\nAxis=Z; Axis to use for this, X, Y, Z" //Grid World Space Environment
	};

	BOOL CALLBACK W3DMaterialPassDlgProc::VertexTab::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		VertexTab* dlg = (VertexTab*)GetWindowLongPtr(hwndDlg, DWLP_USER);

		switch (message)
		{
			case WM_COMMAND:
			{
				const uint16 controlID = LOWORD(wParam);
				const uint16 commandID = HIWORD(wParam);
				switch (controlID)
				{
				case IDC_STAGE_0_MAPPING_ARGS:
				case IDC_STAGE_1_MAPPING_ARGS:
					switch (commandID)
					{
					case EN_SETFOCUS:
						DisableAccelerators();
						break;
					case EN_KILLFOCUS:
						EnableAccelerators();
						break;
					case EN_CHANGE:
					{
						MSTR str = GetWindowText(hwndDlg, controlID);
						dlg->m_Dialog.SetValue(controlID == IDC_STAGE_0_MAPPING_ARGS ? W3DMaterialParamID::Stage0MappingArgs : W3DMaterialParamID::Stage1MappingArgs, str);
						return TRUE;
					}
					}
					break;
				case IDC_STAGE_0_MAPPING_TYPE:
				case IDC_STAGE_1_MAPPING_TYPE:
					switch (commandID)
					{
					case CBN_SELCHANGE:
						if (IDC_STAGE_0_MAPPING_TYPE == controlID)
						{
							dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0Mapping, ComboBox_GetCurSel(dlg->m_Stage0MappingType));
							dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0MappingArgs, defmapperargs[ComboBox_GetCurSel(dlg->m_Stage0MappingType)]);
							SetDlgItemText(hwndDlg, IDC_STAGE_0_MAPPING_ARGS, defmapperargs[ComboBox_GetCurSel(dlg->m_Stage0MappingType)]);
						}
						else
						{
							dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1Mapping, ComboBox_GetCurSel(dlg->m_Stage1MappingType));
							dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1MappingArgs, defmapperargs[ComboBox_GetCurSel(dlg->m_Stage1MappingType)]);
							SetDlgItemText(hwndDlg, IDC_STAGE_1_MAPPING_ARGS, defmapperargs[ComboBox_GetCurSel(dlg->m_Stage1MappingType)]);
						}
						return TRUE;
					}
					break;
				case IDC_SPECULAR_TO_DIFFUSE:
					if (commandID == BN_CLICKED)
						dlg->m_Dialog.SetValue(W3DMaterialParamID::SpecularToDiffuse, GetCheckBox(hwndDlg, IDC_SPECULAR_TO_DIFFUSE));
					break;
				}
				break;
			}
			case CC_COLOR_CHANGE:
			{

				switch (LOWORD(wParam))
				{
				case IDC_AMBIENT_COLOUR:
				{
					Color c = dlg->m_AmbientColour->GetAColor();
					dlg->m_Dialog.SetValue(W3DMaterialParamID::AmbientColour, c);
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				}
				case IDC_DIFFUSE_COLOUR:
				{
					Color c = dlg->m_DiffuseColour->GetAColor();
					dlg->m_Dialog.SetValue(W3DMaterialParamID::DiffuseColour, c);
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				}
				case IDC_SPECULAR_COLOUR:
				{
					Color c = dlg->m_SpecularColour->GetAColor();
					dlg->m_Dialog.SetValue(W3DMaterialParamID::SpecularColour, c);
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				}
				case IDC_EMISSIVE_COLOUR:
				{
					Color c = dlg->m_EmissiveColour->GetAColor();
					dlg->m_Dialog.SetValue(W3DMaterialParamID::EmissiveColour, c);
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				}
				}
				break;
			}
			case CC_SPINNER_CHANGE:
			{
				switch (LOWORD(wParam))
				{
				case IDC_OPACITY_SPIN:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Opacity, dlg->m_OpacitySpin->GetFVal());
					return TRUE;
				case IDC_TRANSLUCENCY_SPIN:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Translucency, dlg->m_TranslucencySpin->GetFVal());
					return TRUE;
				case IDC_SHININESS_SPIN:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Shininess, dlg->m_ShininessSpin->GetFVal());
					return TRUE;
				case IDC_STAGE_0_UV_CHAN_SPIN:
					{
						int value = dlg->m_Stage0UVChanSpin->GetIVal();
						dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0MappingUVChannel, value);
						auto texmap = dlg->m_Dialog.m_ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage0TextureMap));
						if (texmap)
						{
							auto uvgen = texmap->GetTheUVGen();
							if (uvgen)
							{
								uvgen->SetMapChannel(value);
							}
						}
						dlg->m_Dialog.m_Material->InvalidateDisplayTexture();
						dlg->m_Dialog.m_Material->MaterialDirty();
					}
					return TRUE;
				case IDC_STAGE_1_UV_CHAN_SPIN:
					{
						int value = dlg->m_Stage1UVChanSpin->GetIVal();
						dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1MappingUVChannel, value);
						auto texmap = dlg->m_Dialog.m_ParamBlock->GetTexmap(enum_to_value(W3DMaterialParamID::Stage1TextureMap));
						if (texmap)
						{
							auto uvgen = texmap->GetTheUVGen();
							if (uvgen)
							{
								uvgen->SetMapChannel(value);
							}
						}
						dlg->m_Dialog.m_Material->InvalidateDisplayTexture();
						dlg->m_Dialog.m_Material->MaterialDirty();
					}
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	//// Shader Tab ///////////////////////////////////////
	W3DMaterialPassDlgProc::ShaderTab::ShaderTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog)
		: TabBase(tabRoot, dialog)
	{
		m_BlendMode = GetDlgItem(tabRoot, IDC_BLEND_MODE);
		for (int i = 0; i < enum_to_value(W3DMaterialBlendMode::Num); ++i)
		{
			ComboBox_AddString(m_BlendMode, W3DMaterialBlendModeString(static_cast<W3DMaterialBlendMode>(i)));
		}
		ComboBox_SetCurSel(m_BlendMode, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::BlendMode)));

		m_CustomSrcMode = GetDlgItem(tabRoot, IDC_CUSTOM_BLEND_MODE_SRC);
		for (int i = 0; i < enum_to_value(W3DMaterialBlendModeSrcType::Num); ++i)
		{
			ComboBox_AddString(m_CustomSrcMode, W3DMaterialBlendModeSrcString(static_cast<W3DMaterialBlendModeSrcType>(i)));
		}
		ComboBox_SetCurSel(m_CustomSrcMode, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomSrcMode)));

		m_CustomDestMode = GetDlgItem(tabRoot, IDC_CUSTOM_BLEND_MODE_DEST);
		for (int i = 0; i < enum_to_value(W3DMaterialBlendModeDestType::Num); ++i)
		{
			ComboBox_AddString(m_CustomDestMode, W3DMaterialBlendModeDestString(static_cast<W3DMaterialBlendModeDestType>(i)));
		}
		ComboBox_SetCurSel(m_CustomDestMode, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomDestMode)));

		SetCheckBox(tabRoot, IDC_BLEND_WRITE_ZBUFFER, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::BlendWriteZBuffer)));
		SetCheckBox(tabRoot, IDC_BLEND_ALPHA_TEST, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::AlphaTest)));

		m_Defaults = GetDlgItem(tabRoot, IDC_BTN_BLEND_DEFAULTS);

		m_PriGradient = GetDlgItem(tabRoot, IDC_SHADER_PRI_GRADIENT);
		for (int i = 0; i < enum_to_value(W3DMaterialPrimaryGradientMode::Num); ++i)
		{
			ComboBox_AddString(m_PriGradient, W3DMaterialPrimaryGradientModeString(static_cast<W3DMaterialPrimaryGradientMode>(i)));
		}
		ComboBox_SetCurSel(m_PriGradient, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PriGradient)));

		m_SecGradient = GetDlgItem(tabRoot, IDC_SHADER_SEC_GRADIENT);
		ComboBox_AddString(m_SecGradient, _T("Disabled"));
		ComboBox_AddString(m_SecGradient, _T("Enabled"));
		ComboBox_SetCurSel(m_SecGradient, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SecGradient)));

		m_DepthCmp = GetDlgItem(tabRoot, IDC_SHADER_DEPTH_CMP);
		for (int i = 0; i < enum_to_value(W3DMaterialDepthCompMode::Num); ++i)
		{
			ComboBox_AddString(m_DepthCmp, W3DMaterialDepthCompModeString(static_cast<W3DMaterialDepthCompMode>(i)));
		}
		ComboBox_SetCurSel(m_DepthCmp, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DepthCmp)));

		m_DetailColour = GetDlgItem(tabRoot, IDC_SHADER_DETAIL_COLOUR);
		for (int i = 0; i < enum_to_value(W3DMaterialDetailColourMode::Num); ++i)
		{
			ComboBox_AddString(m_DetailColour, W3DMaterialDetailColourModeString(static_cast<W3DMaterialDetailColourMode>(i)));
		}
		ComboBox_SetCurSel(m_DetailColour, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailColour)));

		m_DetailAlpha = GetDlgItem(tabRoot, IDC_SHADER_DETAIL_ALPHA);
		for (int i = 0; i < enum_to_value(W3DMaterialDetailAlphaMode::Num); ++i)
		{
			ComboBox_AddString(m_DetailAlpha, W3DMaterialDetailAlphaModeString(static_cast<W3DMaterialDetailAlphaMode>(i)));
		}
		ComboBox_SetCurSel(m_DetailAlpha, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailAlpha)));
		UpdateBlendMode(tabRoot);
	}

	W3DMaterialPassDlgProc::ShaderTab::~ShaderTab()
	{
	}

	void W3DMaterialPassDlgProc::ShaderTab::ResetBlendGradientDefaults()
	{
		int def_index = m_Dialog.m_ParamBlock->GetParamDef(enum_to_value(W3DMaterialParamID::PriGradient)).def.i;
		ComboBox_SetCurSel(m_PriGradient, def_index);
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::PriGradient), 0, def_index);

		def_index = m_Dialog.m_ParamBlock->GetParamDef(enum_to_value(W3DMaterialParamID::SecGradient)).def.i;
		ComboBox_SetCurSel(m_SecGradient, def_index);
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SecGradient), 0, def_index);

		def_index = m_Dialog.m_ParamBlock->GetParamDef(enum_to_value(W3DMaterialParamID::DepthCmp)).def.i;
		ComboBox_SetCurSel(m_DepthCmp, def_index);
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DepthCmp), 0, def_index);

		def_index = m_Dialog.m_ParamBlock->GetParamDef(enum_to_value(W3DMaterialParamID::DetailColour)).def.i;
		ComboBox_SetCurSel(m_DetailColour, def_index);
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DetailColour), 0, def_index);

		def_index = m_Dialog.m_ParamBlock->GetParamDef(enum_to_value(W3DMaterialParamID::DetailAlpha)).def.i;
		ComboBox_SetCurSel(m_DetailAlpha, def_index);
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DetailAlpha), 0, def_index);
		m_Dialog.m_Material->MaterialDirty();
	}

	struct BlendStr
	{
		int SrcBlend;
		int DestBlend;
		bool WriteZBuffer;
		bool AlphaTest;
	};

	BlendStr blends[] = {
	{1, 0, true, false},
	{1, 1, false, false},
	{0, 2, false, false},
	{1, 2, false, false},
	{1, 3, false, false},
	{2, 5, false, false},
	{1, 0, true, true},
	{2, 5, true, true},
	};

	void W3DMaterialPassDlgProc::ShaderTab::UpdateFromBlendMode(HWND hwndDlg,int mode)
	{
		if (mode >= 0 && mode < 8)
		{
			m_Dialog.SetValue(W3DMaterialParamID::CustomSrcMode, blends[mode].SrcBlend);
			ComboBox_SetCurSel(m_CustomSrcMode, blends[mode].SrcBlend);
			m_Dialog.SetValue(W3DMaterialParamID::CustomDestMode, blends[mode].DestBlend);
			ComboBox_SetCurSel(m_CustomDestMode, blends[mode].DestBlend);
			m_Dialog.SetValue(W3DMaterialParamID::BlendWriteZBuffer, blends[mode].WriteZBuffer);
			SetCheckBox(hwndDlg, IDC_BLEND_WRITE_ZBUFFER, blends[mode].WriteZBuffer);
			m_Dialog.SetValue(W3DMaterialParamID::AlphaTest, blends[mode].AlphaTest);
			SetCheckBox(hwndDlg, IDC_BLEND_ALPHA_TEST, blends[mode].AlphaTest);
			m_Dialog.m_Material->MaterialDirty();
		}
	}

	void W3DMaterialPassDlgProc::ShaderTab::UpdateBlendMode(HWND hwndDlg)
	{
		int i;
		int src = ComboBox_GetCurSel(m_CustomSrcMode);
		int dest = ComboBox_GetCurSel(m_CustomDestMode);
		bool zbuffer = (bool)GetCheckBox(hwndDlg, IDC_BLEND_WRITE_ZBUFFER);
		bool alpha = (bool)GetCheckBox(hwndDlg, IDC_BLEND_ALPHA_TEST);
		for (i = 0; i < 8; i++)
		{
			if (src == blends[i].SrcBlend &&
				dest == blends[i].DestBlend &&
				zbuffer == blends[i].WriteZBuffer &&
				alpha == blends[i].AlphaTest)
			{
				break;
			}
		}
		m_Dialog.SetValue(W3DMaterialParamID::BlendMode, i);
		ComboBox_SetCurSel(m_BlendMode, i);
	}

	BOOL CALLBACK W3DMaterialPassDlgProc::ShaderTab::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ShaderTab* dlg = (ShaderTab*)GetWindowLongPtr(hwndDlg, DWLP_USER);

		switch (message)
		{
		case WM_COMMAND:
		{
			const uint16 controlID = LOWORD(wParam);
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				switch (controlID)
				{
				case IDC_BLEND_MODE:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::BlendMode, ComboBox_GetCurSel(dlg->m_BlendMode));
					dlg->UpdateFromBlendMode(hwndDlg, ComboBox_GetCurSel(dlg->m_BlendMode));
					return TRUE;
				case IDC_CUSTOM_BLEND_MODE_SRC:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::CustomSrcMode, ComboBox_GetCurSel(dlg->m_CustomSrcMode));
					dlg->m_Dialog.m_Material->MaterialDirty();
					dlg->UpdateBlendMode(hwndDlg);
					return TRUE;
				case IDC_CUSTOM_BLEND_MODE_DEST:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::CustomDestMode, ComboBox_GetCurSel(dlg->m_CustomDestMode));
					dlg->m_Dialog.m_Material->MaterialDirty();
					dlg->UpdateBlendMode(hwndDlg);
					return TRUE;
				case IDC_SHADER_PRI_GRADIENT:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::PriGradient, ComboBox_GetCurSel(dlg->m_PriGradient));
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				case IDC_SHADER_SEC_GRADIENT:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::SecGradient, ComboBox_GetCurSel(dlg->m_SecGradient));
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				case IDC_SHADER_DEPTH_CMP:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::DepthCmp, ComboBox_GetCurSel(dlg->m_DepthCmp));
					return TRUE;
				case IDC_SHADER_DETAIL_COLOUR:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::DetailColour, ComboBox_GetCurSel(dlg->m_DetailColour));
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				case IDC_SHADER_DETAIL_ALPHA:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::DetailAlpha, ComboBox_GetCurSel(dlg->m_DetailAlpha));
					dlg->m_Dialog.m_Material->MaterialDirty();
					return TRUE;
				}
				break;
			}
			case BN_CLICKED:
				switch (controlID)
				{
				case IDC_BLEND_WRITE_ZBUFFER:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::BlendWriteZBuffer, GetCheckBox(hwndDlg, IDC_BLEND_WRITE_ZBUFFER));
					dlg->UpdateBlendMode(hwndDlg);
					return TRUE;
				case IDC_BLEND_ALPHA_TEST:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::AlphaTest, GetCheckBox(hwndDlg, IDC_BLEND_ALPHA_TEST));
					dlg->UpdateBlendMode(hwndDlg);
					return TRUE;
				case IDC_BTN_BLEND_DEFAULTS:
					dlg->ResetBlendGradientDefaults();
					return true;
				}
			}
		}
		}

		return FALSE;
	}

	//// Textures Tab ///////////////////////////////////////
	W3DMaterialPassDlgProc::TexturesTab::TexturesTab(HWND tabRoot, W3DMaterialPassDlgProc& dialog)
		: TabBase(tabRoot, dialog)
	{
		m_Stage0TextureEnabled = GetDlgItem(tabRoot, IDC_STAGE_0_ENABLED);
		SetCheckBox(tabRoot, IDC_STAGE_0_ENABLED, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0TextureEnabled)));

		m_Stage0TextureMap = GetICustButton(GetDlgItem(tabRoot, IDC_STAGE_0_TEXTURE_MAP));
		m_Stage0TextureMap->SetType(CBT_PUSH);
		UpdateTexmapButtonText(m_Stage0TextureMap, *dialog.m_ParamBlock, W3DMaterialParamID::Stage0TextureMap);

		m_Stage0Publish = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_PUBLISH), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0Publish);
		m_Stage0Display = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_DISPLAY), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0Display);
		m_Stage0ClampU = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_CLAMP_U), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0ClampU);
		m_Stage0ClampV = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_CLAMP_V), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0ClampV);
		m_Stage0NoLOD = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_NO_LOD), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0NoLOD);
		m_Stage0AlphaBitmap = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_0_ALPHA_BITMAP), *dialog.m_ParamBlock, W3DMaterialParamID::Stage0AlphaBitmap);
		m_Stage0Frames = SetupIntSpinner(tabRoot, IDC_STAGE_0_FRAMES_SPIN, IDC_STAGE_0_FRAMES_EDIT, 1, 999, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0Frames)));
		m_Stage0FPS = SetupFloatSpinner(tabRoot, IDC_STAGE_0_FPS_SPIN, IDC_STAGE_0_FPS_EDIT, 0.0f, 60.0f, dialog.m_ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Stage0FPS)));
		m_Stage0PassHint = GetDlgItem(tabRoot, IDC_STAGE_0_PASS_HINT);
		m_Stage0AnimMode = GetDlgItem(tabRoot, IDC_STAGE_0_ANIM_MODE);

		m_Stage1TextureEnabled = GetDlgItem(tabRoot, IDC_STAGE_1_ENABLED);
		SetCheckBox(tabRoot, IDC_STAGE_1_ENABLED, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1TextureEnabled)));

		m_Stage1TextureMap = GetICustButton(GetDlgItem(tabRoot, IDC_STAGE_1_TEXTURE_MAP));
		m_Stage1TextureMap->SetType(CBT_PUSH);
		UpdateTexmapButtonText(m_Stage1TextureMap, *dialog.m_ParamBlock, W3DMaterialParamID::Stage1TextureMap);

		m_Stage1Publish = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_PUBLISH), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1Publish);
		m_Stage1Display = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_DISPLAY), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1Display);
		m_Stage1ClampU = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_CLAMP_U), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1ClampU);
		m_Stage1ClampV = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_CLAMP_V), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1ClampV);
		m_Stage1NoLOD = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_NO_LOD), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1NoLOD);
		m_Stage1AlphaBitmap = InitCheckButton(GetDlgItem(tabRoot, IDC_STAGE_1_ALPHA_BITMAP), *dialog.m_ParamBlock, W3DMaterialParamID::Stage1AlphaBitmap);
		m_Stage1Frames = SetupIntSpinner(tabRoot, IDC_STAGE_1_FRAMES_SPIN, IDC_STAGE_1_FRAMES_EDIT, 1, 999, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1Frames)));
		m_Stage1FPS = SetupFloatSpinner(tabRoot, IDC_STAGE_1_FPS_SPIN, IDC_STAGE_1_FPS_EDIT, 0.0f, 60.0f, dialog.m_ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Stage1FPS)));
		m_Stage1PassHint = GetDlgItem(tabRoot, IDC_STAGE_1_PASS_HINT);
		m_Stage1AnimMode = GetDlgItem(tabRoot, IDC_STAGE_1_ANIM_MODE);

		for (int i = 0; i < enum_to_value(W3DMaterialTexturePassHint::Num); ++i)
		{
			ComboBox_AddString(m_Stage0PassHint, W3DMaterialTexturePassHintString(static_cast<W3DMaterialTexturePassHint>(i)));
			ComboBox_AddString(m_Stage1PassHint, W3DMaterialTexturePassHintString(static_cast<W3DMaterialTexturePassHint>(i)));
		}
		ComboBox_SetCurSel(m_Stage0PassHint, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0PassHint)));
		ComboBox_SetCurSel(m_Stage1PassHint, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1PassHint)));

		for (int i = 0; i < enum_to_value(W3DMaterialTextureAnimMode::Num); ++i)
		{
			ComboBox_AddString(m_Stage0AnimMode, W3DMaterialTextureAnimModeString(static_cast<W3DMaterialTextureAnimMode>(i)));
			ComboBox_AddString(m_Stage1AnimMode, W3DMaterialTextureAnimModeString(static_cast<W3DMaterialTextureAnimMode>(i)));
		}
		ComboBox_SetCurSel(m_Stage0AnimMode, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0AnimMode)));
		ComboBox_SetCurSel(m_Stage1AnimMode, dialog.m_ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1AnimMode)));

		//Must happen last so all controls are valid
		SetStage0Enabled(GetCheckBox(tabRoot, IDC_STAGE_0_ENABLED));
		SetStage1Enabled(GetCheckBox(tabRoot, IDC_STAGE_1_ENABLED));
	}

	W3DMaterialPassDlgProc::TexturesTab::~TexturesTab()
	{
	}

	void W3DMaterialPassDlgProc::TexturesTab::UpdateTexture(ICustButton * textureButton, W3DMaterialParamID param)
	{
		BitmapInfo info;
		if (TheManager->SelectFileInput(&info, GetCOREInterface()->GetMAXHWnd()))
		{
			Bitmap* bm = TheManager->Load(&info);
			if (bm)
			{
				BitmapTex* tm = static_cast<BitmapTex*>(m_Dialog.m_ParamBlock->GetTexmap(enum_to_value(param)));
				if (!tm)
				{
					tm = NewDefaultBitmapTex();
				}

				tm->SetBitmapInfo(info);
				tm->ActivateTexDisplay(TRUE);

				BOOL setResult = m_Dialog.m_ParamBlock->SetValue(enum_to_value(param), 0, tm);
				assert(setResult == TRUE);

				UpdateTexmapButtonText(textureButton, *m_Dialog.m_ParamBlock, param);
				m_Dialog.m_Material->MaterialDirty();
			}
		}
	}

	void W3DMaterialPassDlgProc::TexturesTab::SetStage0Enabled(bool enabled)
	{
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage0TextureEnabled), 0, enabled ? TRUE : FALSE);

		m_Stage0TextureMap->Enable(enabled);
		m_Stage0Publish->Enable(enabled);
		m_Stage0Display->Enable(enabled);
		m_Stage0ClampU->Enable(enabled);
		m_Stage0ClampV->Enable(enabled);
		m_Stage0NoLOD->Enable(enabled);
		m_Stage0AlphaBitmap->Enable(enabled);
		m_Stage0Frames->Enable(enabled);
		m_Stage0FPS->Enable(enabled);
		ComboBox_Enable(m_Stage0PassHint, enabled);
		ComboBox_Enable(m_Stage0AnimMode, enabled);
		if (!enabled && m_Dialog.m_Material)
		{
			m_Stage0Display->SetCheck(enabled);
			m_Dialog.SetDisplayFlag(*m_Stage0Display, W3DMaterialParamID::Stage0Display);
		}
	}

	void W3DMaterialPassDlgProc::TexturesTab::SetStage1Enabled(bool enabled)
	{
		m_Dialog.m_ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage1TextureEnabled), 0, enabled ? TRUE : FALSE);

		m_Stage1TextureMap->Enable(enabled);
		m_Stage1Publish->Enable(enabled);
		m_Stage1Display->Enable(enabled);
		m_Stage1ClampU->Enable(enabled);
		m_Stage1ClampV->Enable(enabled);
		m_Stage1NoLOD->Enable(enabled);
		m_Stage1AlphaBitmap->Enable(enabled);
		m_Stage1Frames->Enable(enabled);
		m_Stage1FPS->Enable(enabled);
		ComboBox_Enable(m_Stage1PassHint, enabled);
		ComboBox_Enable(m_Stage1AnimMode, enabled);
		if (!enabled && m_Dialog.m_Material)
		{
			m_Stage1Display->SetCheck(enabled);
			m_Dialog.SetDisplayFlag(*m_Stage1Display, W3DMaterialParamID::Stage1Display);
		}
	}

	void W3DMaterialPassDlgProc::TexturesTab::ClearDisplayFlags()
	{
		m_Stage0Display->SetCheck(FALSE);
		m_Stage1Display->SetCheck(FALSE);
	}

	BOOL CALLBACK W3DMaterialPassDlgProc::TexturesTab::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		TexturesTab* dlg = (TexturesTab*)GetWindowLongPtr(hwndDlg, DWLP_USER);

		switch (message)
		{
		case WM_COMMAND:
		{
			const uint16 controlID = LOWORD(wParam);
			const uint16 commandID = HIWORD(wParam);
			switch (commandID)
			{
			case BN_CLICKED:
			{
				switch (controlID)
				{
				case IDC_STAGE_0_ENABLED:
					dlg->SetStage0Enabled(GetCheckBox(hwndDlg, IDC_STAGE_0_ENABLED));
					return TRUE;
				case IDC_STAGE_1_ENABLED:
					dlg->SetStage1Enabled(GetCheckBox(hwndDlg, IDC_STAGE_1_ENABLED));
					return TRUE;
				case IDC_STAGE_0_TEXTURE_MAP:
					dlg->UpdateTexture(dlg->m_Stage0TextureMap, W3DMaterialParamID::Stage0TextureMap);
					return TRUE;
				case IDC_STAGE_1_TEXTURE_MAP:
					dlg->UpdateTexture(dlg->m_Stage1TextureMap, W3DMaterialParamID::Stage1TextureMap);
					return TRUE;
				case IDC_STAGE_0_PUBLISH:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0Publish, dlg->m_Stage0Publish->IsChecked());
					return TRUE;
				case IDC_STAGE_1_PUBLISH:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1Publish, dlg->m_Stage1Publish->IsChecked());
					return TRUE;
				case IDC_STAGE_0_CLAMP_U:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0ClampU, dlg->m_Stage0ClampU->IsChecked());
					return TRUE;
				case IDC_STAGE_1_CLAMP_U:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1ClampU, dlg->m_Stage1ClampU->IsChecked());
					return TRUE;
				case IDC_STAGE_0_CLAMP_V:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0ClampV, dlg->m_Stage0ClampV->IsChecked());
					return TRUE;
				case IDC_STAGE_1_CLAMP_V:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1ClampV, dlg->m_Stage1ClampV->IsChecked());
					return TRUE;
				case IDC_STAGE_0_NO_LOD:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0NoLOD, dlg->m_Stage0NoLOD->IsChecked());
					return TRUE;
				case IDC_STAGE_1_NO_LOD:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1NoLOD, dlg->m_Stage1NoLOD->IsChecked());
					return TRUE;
				case IDC_STAGE_0_ALPHA_BITMAP:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0AlphaBitmap, dlg->m_Stage0AlphaBitmap->IsChecked());
					return TRUE;
				case IDC_STAGE_1_ALPHA_BITMAP:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1AlphaBitmap, dlg->m_Stage1AlphaBitmap->IsChecked());
					return TRUE;
				case IDC_STAGE_0_DISPLAY:
					dlg->m_Dialog.SetDisplayFlag(*dlg->m_Stage0Display, W3DMaterialParamID::Stage0Display);
					return TRUE;
				case IDC_STAGE_1_DISPLAY:
					dlg->m_Dialog.SetDisplayFlag(*dlg->m_Stage1Display, W3DMaterialParamID::Stage1Display);
					return TRUE;
				} //sw controlID
				break;
			} //BN_CLICKED
			case CBN_SELCHANGE:
			{
				switch (controlID)
				{
				case IDC_STAGE_0_ANIM_MODE:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0AnimMode, ComboBox_GetCurSel(dlg->m_Stage0AnimMode));
					return TRUE;
				case IDC_STAGE_1_ANIM_MODE:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1AnimMode, ComboBox_GetCurSel(dlg->m_Stage1AnimMode));
					return TRUE;
				case IDC_STAGE_0_PASS_HINT:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0PassHint, ComboBox_GetCurSel(dlg->m_Stage0PassHint));
					return TRUE;
				case IDC_STAGE_1_PASS_HINT:
					dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1PassHint, ComboBox_GetCurSel(dlg->m_Stage1PassHint));
					return TRUE;
				} //sw ControlID
				break;
			} //CBN_SELCHANGE
			} //Sw commandID
			break; //WM_COMMAND
		}
		case CC_SPINNER_CHANGE:
		{
			switch (LOWORD(wParam))
			{
			case IDC_STAGE_0_FRAMES_SPIN:
				dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0Frames, dlg->m_Stage0Frames->GetIVal());
				return TRUE;
			case IDC_STAGE_1_FRAMES_SPIN:
				dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1Frames, dlg->m_Stage1Frames->GetIVal());
				return TRUE;
			case IDC_STAGE_0_FPS_SPIN:
				dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage0FPS, dlg->m_Stage0FPS->GetFVal());
				return TRUE;
			case IDC_STAGE_1_FPS_SPIN:
				dlg->m_Dialog.SetValue(W3DMaterialParamID::Stage1FPS, dlg->m_Stage1FPS->GetFVal());
				return TRUE;
			}
			break; //CC_SPINNER_CHANGE
		}
		case CC_SPINNER_BUTTONUP:
		{
			dlg->m_Dialog.m_Material->MaterialDirty();
		}
		} //sw message

		return FALSE;
	}
}