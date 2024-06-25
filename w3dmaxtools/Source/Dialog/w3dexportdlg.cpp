#include "general.h"
#include <unordered_map>
#include "Dialog/w3dexportdlg.h"
#include "EnumUtilities.h"
#include "resource.h"

extern HINSTANCE hInstance;

namespace
{
	W3D::MaxTools::W3DExportType RadioButtonIDToW3DExportType(uint16 controlID)
	{
		using namespace W3D::MaxTools;

		static const std::unordered_map<uint16, W3DExportType> s_dlg_id_map 
		{
			{ IDC_HIERARCHICAL_ANIMATED_MODEL,          W3DExportType::HierarchicalAnimatedModel},
			{ IDC_HIERARCHICAL_MODEL, W3DExportType::HierarchicalModel},
			{ IDC_PURE_ANIMATION,              W3DExportType::PureAnimation},
			{ IDC_SKELETON,                    W3DExportType::Skeleton},
#ifndef W3X
			{ IDC_TERRAIN,                     W3DExportType::Terrain},
#endif
			{ IDC_SIMPLE_MESH,                 W3DExportType::SimpleMesh}
		};

		std::unordered_map<uint16, W3D::MaxTools::W3DExportType>::const_iterator search_it = s_dlg_id_map.find(controlID);

		return search_it == s_dlg_id_map.end() ? W3DExportType::Num : search_it->second;
	}

	uint16 W3DExportTypeToDialogID(W3D::MaxTools::W3DExportType exportType)
	{
		using namespace W3D::MaxTools;

		static const std::array<uint16, enum_to_value(W3DExportType::Num)> s_dlg_ids
		{
			IDD_W3D_EXPORT_HIERARCHICAL_MODEL,
			IDD_W3D_EXPORT_HIERARCHICAL_ANIMATED_MODEL,
			IDD_W3D_EXPORT_ANIMATION,
			IDD_W3D_EXPORT_SKELETON,
			IDD_W3D_EXPORT_TERRAIN,
			IDD_W3D_EXPORT_SIMPLE_MESH
		};

		return s_dlg_ids[enum_to_value(exportType)];
	}


	uint16 W3DExportTypeToRadioButtonID(W3D::MaxTools::W3DExportType exportType)
	{
		using namespace W3D::MaxTools;

		static const std::array<uint16, enum_to_value(W3DExportType::Num)> s_ctrl_ids
		{
			IDC_HIERARCHICAL_MODEL,
			IDC_HIERARCHICAL_ANIMATED_MODEL,
			IDC_PURE_ANIMATION,
			IDC_SKELETON,
#ifndef W3X
			IDC_TERRAIN,
#else
			0,
#endif
			IDC_SIMPLE_MESH
		};

		return s_ctrl_ids[enum_to_value(exportType)];
	}
}

namespace W3D::MaxTools
{
	W3DExportDlg::W3DExportDlg(W3DExportSettings & settings)
		: m_Settings(settings)
		, m_DialogRoot()
		, m_SettingsPanel()
		, m_ActiveTab()
		, m_Tabs()
	{ }

	INT_PTR W3DExportDlg::ShowDialog()
	{
		//TT_PROFILER_SCOPE("Export Settings Dialog");
		return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_EXPORT), GetCOREInterface()->GetMAXHWnd(), W3DExportDlg::DlgProc, reinterpret_cast<LPARAM>(this));
	}

	INT_PTR W3DExportDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		W3DExportDlg* dlg = reinterpret_cast<W3DExportDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_INITDIALOG:
			dlg = reinterpret_cast<W3DExportDlg*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			dlg->ConnectControls(hWnd);

			RECT rc, rcDlg, rcOwner;
			GetWindowRect(GetCOREInterface()->GetMAXHWnd(), &rcOwner);
			GetWindowRect(hWnd, &rcDlg);
			CopyRect(&rc, &rcOwner);

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			SetWindowPos(hWnd,
				HWND_TOP,
				rcOwner.left + (rc.right / 2),
				rcOwner.top + (rc.bottom / 2),
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE);

			return TRUE;
		case WM_DESTROY:
			return TRUE;
		case WM_COMMAND:
			return dlg->HandleCommand(LOWORD(wParam), HIWORD(wParam));
		}
		return FALSE;
	}

	INT_PTR W3DExportDlg::TabProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		W3DExportDlg* dlg = reinterpret_cast<W3DExportDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_INITDIALOG:
			dlg = reinterpret_cast<W3DExportDlg*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			dlg->PopulateControls(hWnd);
			return TRUE;
		case WM_COMMAND:
			return dlg->HandleCommand(LOWORD(wParam), HIWORD(wParam));
		case CC_SPINNER_CHANGE:
			return dlg->HandleSpinner(LOWORD(wParam));
		}
		return FALSE;
	}

	void W3DExportDlg::ConnectControls(HWND root)
	{
		m_DialogRoot = root;
		m_SettingsPanel = GetDlgItem(root, IDC_SETTINGS_PANEL);
		
		RECT panelRect;
		GetClientRect(m_SettingsPanel, &panelRect);

		const uint32 dlu = GetDialogBaseUnits();
		const uint16 dlwidth = LOWORD(dlu);
		const uint16 dlheight = HIWORD(dlu);

		using BASE_TYPE = std::underlying_type_t<W3DExportType>;
		for (BASE_TYPE i = 0; i < m_Tabs.size(); ++i)
		{
			m_Tabs[i] = CreateDialogParam(hInstance, MAKEINTRESOURCE(W3DExportTypeToDialogID(static_cast<W3DExportType>(i))), m_SettingsPanel, TabProc, reinterpret_cast<LPARAM>(this));
			MoveWindow(m_Tabs[i], panelRect.left + dlwidth, panelRect.top + dlheight, (panelRect.right - panelRect.left) - dlwidth * 2, (panelRect.bottom - panelRect.top) - dlheight * 2, FALSE);
		}
		
		CheckRadioButton(m_DialogRoot, IDC_HIERARCHICAL_MODEL, IDC_SIMPLE_MESH, W3DExportTypeToRadioButtonID(m_Settings.ExportType));

		SetTab(m_Settings.ExportType);
	}

	void W3DExportDlg::PopulateControls(HWND root)
	{
		Interval animRange = GetCOREInterface()->GetAnimRange();
		int ticksPerFrame = GetTicksPerFrame();
		if (m_Settings.AnimFramesStart < animRange.Start() / ticksPerFrame)
		{
			m_Settings.AnimFramesStart = animRange.Start() / ticksPerFrame;
		}
		if (m_Settings.AnimFramesEnd > animRange.End() / ticksPerFrame)
		{
			m_Settings.AnimFramesEnd = animRange.End() / ticksPerFrame;
		}
		if (GetDlgItem(root, IDC_FRAMES_SPIN))
		{
			ReleaseISpinner(SetupIntSpinner(root, IDC_FRAMES_SPIN, IDC_FRAMES_EDIT, animRange.Start() / ticksPerFrame, animRange.End() / ticksPerFrame, m_Settings.AnimFramesStart));
		}
		if (GetDlgItem(root, IDC_FRAMES_TO_SPIN))
		{
			ReleaseISpinner(SetupIntSpinner(root, IDC_FRAMES_TO_SPIN, IDC_FRAMES_TO_EDIT, animRange.Start() / ticksPerFrame, animRange.End() / ticksPerFrame, m_Settings.AnimFramesEnd));
		}

		SetCheckBox(root, IDC_SMOOTH_VERTICES, m_Settings.SmoothVertexNormals);
		SetCheckBox(root, IDC_OPT_COLLISONS, m_Settings.OptimiseCollisions);
#ifndef W3X
		SetCheckBox(root, IDC_DEDUPLICATE, m_Settings.MeshDeduplication);
		SetCheckBox(root, IDC_NEWAABTREE, m_Settings.NewAABTree);
#endif
		SetCheckBox(root, IDC_USE_EXT_SKELETON, m_Settings.UseExistingSkeleton);
		SetCheckBox(m_DialogRoot, IDC_REVIEW_LOG, m_Settings.ReviewLog);
		
		RefreshExternalSkeletonButton();
	}

	void W3DExportDlg::SetTab(W3DExportType tab)
	{
		using BASE_TYPE = std::underlying_type_t<W3DExportType>;
		for (BASE_TYPE i = 0; i < m_Tabs.size(); ++i)
		{
			ShowWindow(m_Tabs[i], i == enum_to_value(tab) ? SW_SHOW : SW_HIDE);
		}

		m_ActiveTab = m_Tabs[enum_to_value(tab)];
		PopulateControls(m_ActiveTab);
	}

	void W3DExportDlg::SelectExternalSkeleton(HWND buttonHandle)
	{
		ICustButton* button = GetICustButton(buttonHandle);
#ifndef W3X
		static FilterList s_fl = []() { FilterList fl; fl.Append(_M("W3D File (*.w3d)")); fl.Append(_M("*.w3d")); return
			fl; }();
#else
		static FilterList s_fl = []() { FilterList fl; fl.Append(_M("W3D File (*.w3x)")); fl.Append(_M("*.w3x")); return
			fl; }();
#endif
		MSTR fname(m_Settings.ExistingSkeletonFileName);
		MSTR dir(m_Settings.ExistingSkeletonFileDirectory);
		if (GetCOREInterface16()->DoMaxOpenDialog(GetCOREInterface()->GetMAXHWnd(), _M("Existing Skeleton"), fname, dir, s_fl))
		{
			wcscpy(m_Settings.ExistingSkeletonFileName, fname);
			wcscpy(m_Settings.ExistingSkeletonFileDirectory, dir);
			RefreshExternalSkeletonButton();
		}

		ReleaseICustButton(button);
	}

	void W3DExportDlg::RefreshExternalSkeletonButton()
	{
		HWND skelBrowse = GetDlgItem(m_ActiveTab, IDC_BROWSE);
		if (skelBrowse)
		{
			ICustButton* btn = GetICustButton(skelBrowse);

			if (m_Settings.ExistingSkeletonFileName[0])
			{
				MSTR filename;
				SplitFilename(m_Settings.ExistingSkeletonFileName, nullptr, &filename, nullptr);
				btn->SetText(filename);
			}
			else
			{
				btn->SetText(_M("Browse..."));
			}
			btn->Enable(m_Settings.UseExistingSkeleton);
			ReleaseICustButton(btn);
		}
	}

	INT_PTR W3DExportDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		switch (controlID)
		{
		case IDC_SMOOTH_VERTICES:
			m_Settings.SmoothVertexNormals = IsDlgButtonChecked(m_ActiveTab, controlID);
			return TRUE;
		case IDC_OPT_COLLISONS:
			m_Settings.OptimiseCollisions = IsDlgButtonChecked(m_ActiveTab, controlID);
			return TRUE;
#ifndef W3X
		case IDC_DEDUPLICATE:
			m_Settings.MeshDeduplication = IsDlgButtonChecked(m_ActiveTab, controlID);
			return TRUE;
		case IDC_NEWAABTREE:
			m_Settings.NewAABTree = IsDlgButtonChecked(m_ActiveTab, controlID);
			return TRUE;
#endif
		case IDC_USE_EXT_SKELETON:
			m_Settings.UseExistingSkeleton = IsDlgButtonChecked(m_ActiveTab, controlID);
			RefreshExternalSkeletonButton();
			return TRUE;
		case IDC_BROWSE:
			SelectExternalSkeleton(GetDlgItem(m_ActiveTab, controlID));
			return TRUE;
		case IDC_REVIEW_LOG:
			m_Settings.ReviewLog = IsDlgButtonChecked(m_DialogRoot, controlID);
			return TRUE;
		
		//Tab Selection
		case IDC_HIERARCHICAL_MODEL:
		case IDC_HIERARCHICAL_ANIMATED_MODEL:
		case IDC_PURE_ANIMATION:
		case IDC_SKELETON:
#ifndef W3X
		case IDC_TERRAIN:
#endif
		case IDC_SIMPLE_MESH:
			m_Settings.ExportType = RadioButtonIDToW3DExportType(controlID);
			SetTab(m_Settings.ExportType);
			return TRUE;

		//Close Commands
		case IDOK:
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(m_DialogRoot, controlID);
			return TRUE;

#ifdef W3X
		case IDC_COMPRESSIONSETTINGS:
			{
				W3DCompressionDlg dlg(m_Settings, m_DialogRoot);
				dlg.ShowDialog();
			}
			return TRUE;
#endif
		}


		return FALSE;
	}

	INT_PTR W3DExportDlg::HandleSpinner(uint16 controlID)
	{
		switch (controlID)
		{
		case IDC_FRAMES_SPIN:
		{
			ISpinnerControl* sc = GetISpinner(GetDlgItem(m_ActiveTab, controlID));
			m_Settings.AnimFramesStart = sc->GetIVal();
			ReleaseISpinner(sc);
			return TRUE;
		}
		case IDC_FRAMES_TO_SPIN:
		{
			ISpinnerControl* sc = GetISpinner(GetDlgItem(m_ActiveTab, controlID));
			m_Settings.AnimFramesEnd = sc->GetIVal();
			ReleaseISpinner(sc);
			return TRUE;
		}
		}
		return FALSE;
	}

#ifdef W3X
	W3DCompressionDlg::W3DCompressionDlg(W3DExportSettings& settings, HWND parent)
		: m_Settings(settings)
		, m_Parent(parent)
		, m_MaxError(nullptr)
		, m_MaxTranslation(nullptr)
		, m_MaxVisibility(nullptr)
		, m_MaxRotation(nullptr)
	{ }

	INT_PTR W3DCompressionDlg::ShowDialog()
	{
		return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_COMPRESSION), m_Parent, W3DCompressionDlg::DlgProc, reinterpret_cast<LPARAM>(this));
	}

	INT_PTR W3DCompressionDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		W3DCompressionDlg* dlg = reinterpret_cast<W3DCompressionDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_INITDIALOG:
			dlg = reinterpret_cast<W3DCompressionDlg*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			dlg->Initialise(hWnd);
			dlg->UpdateControls(hWnd);
			return TRUE;
		case WM_DESTROY:
			return TRUE;
		case WM_COMMAND:
			return dlg->HandleCommand(hWnd, LOWORD(wParam), HIWORD(wParam));
		}
		return FALSE;
	}

	void W3DCompressionDlg::Initialise(HWND dialogRoot)
	{
		SetCheckBox(dialogRoot, IDC_DEFAULTSETTINGS, m_Settings.NonDefaultCompressionSettings == 0);
		SetCheckBox(dialogRoot, IDC_TIMECODED, m_Settings.CompressionTypes & 1);
		SetCheckBox(dialogRoot, IDC_ADAPTIVEDELTA, (m_Settings.CompressionTypes >> 1) & 1);
		SetCheckBox(dialogRoot, IDC_FORCEKEYREDUCTION, m_Settings.ForceKeyReduction);
		wchar_t buf[128];
		memset(buf, 0, sizeof(buf));
		HWND combo = GetDlgItem(dialogRoot, IDC_KEYREDUCTION);
		for (int i = 1; i < 100; i++)
		{
			swprintf(buf, 128, L"%d", i);
			SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)buf);
		}
		if (m_Settings.KeyReduction < 1 || m_Settings.KeyReduction > 99)
		{
			m_Settings.KeyReduction = 50;
		}
		int reduction = m_Settings.KeyReduction - 1;
		SendMessage(combo, CB_SETCURSEL, reduction, 0);
		swprintf(buf, 128, L"%f", m_Settings.MaxTranslationError);
		m_MaxTranslation = GetICustEdit(GetDlgItem(dialogRoot, IDC_MAXTRANSLATION));
		m_MaxTranslation->SetText(buf);
		swprintf(buf, 128, L"%f", m_Settings.MaxRotationError);
		m_MaxRotation = GetICustEdit(GetDlgItem(dialogRoot, IDC_MAXROTATION));
		m_MaxRotation->SetText(buf);
		float f = m_Settings.MaxVisibilityError * 100.0f;
		swprintf(buf, 128, L"%f", f);
		m_MaxVisibility = GetICustEdit(GetDlgItem(dialogRoot, IDC_MAXVISIBILITY));
		m_MaxVisibility->SetText(buf);
		swprintf(buf, 128, L"%f", m_Settings.MaxAdaptiveDeltaError);
		m_MaxError = GetICustEdit(GetDlgItem(dialogRoot, IDC_MAXERROR));
		m_MaxError->SetText(buf);
	}

	void W3DCompressionDlg::UpdateControls(HWND dialogRoot)
	{
		bool usedefault = IsDlgButtonChecked(dialogRoot, IDC_DEFAULTSETTINGS);
		EnableWindow(GetDlgItem(dialogRoot, IDC_TIMECODED), !usedefault);
		EnableWindow(GetDlgItem(dialogRoot, IDC_ADAPTIVEDELTA), !usedefault);
		bool timecoded = !usedefault && IsDlgButtonChecked(dialogRoot, IDC_TIMECODED);
		EnableWindow(GetDlgItem(dialogRoot, IDC_MAXTRANSLATION), timecoded);
		EnableWindow(GetDlgItem(dialogRoot, IDC_MAXROTATION), timecoded);
		EnableWindow(GetDlgItem(dialogRoot, IDC_MAXVISIBILITY), timecoded);
		EnableWindow(GetDlgItem(dialogRoot, IDC_FORCEKEYREDUCTION), timecoded);
		bool keyreduction = timecoded && IsDlgButtonChecked(dialogRoot, IDC_FORCEKEYREDUCTION);
		EnableWindow(GetDlgItem(dialogRoot, IDC_KEYREDUCTION), keyreduction);
		bool adaptivedelta = !usedefault && IsDlgButtonChecked(dialogRoot, IDC_ADAPTIVEDELTA);
		EnableWindow(GetDlgItem(dialogRoot, IDC_MAXERROR), adaptivedelta);
	}

	INT_PTR W3DCompressionDlg::HandleCommand(HWND dialogRoot, uint16 controlID, uint16 commandID)
	{
		switch (controlID)
		{
		case IDOK:
			OnOk(dialogRoot);
			EndDialog(dialogRoot, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(dialogRoot, 2);
			return TRUE;
		case IDC_FORCEKEYREDUCTION:
		case IDC_TIMECODED:
		case IDC_ADAPTIVEDELTA:
		case IDC_DEFAULTSETTINGS:
			UpdateControls(dialogRoot);
			return TRUE;
		}

		return FALSE;
	}

	void W3DCompressionDlg::OnOk(HWND dialogRoot)
	{
		m_Settings.NonDefaultCompressionSettings = IsDlgButtonChecked(dialogRoot, IDC_DEFAULTSETTINGS) != 1;
		if (!m_Settings.NonDefaultCompressionSettings)
		{
			m_Settings.CompressionTypes = 3;
			m_Settings.MaxTranslationError = 0.002f;
			m_Settings.MaxRotationError = 0.003f;
			m_Settings.MaxVisibilityError = 0.01f;
			m_Settings.ForceKeyReduction = false;
			m_Settings.KeyReduction = 50;
			m_Settings.MaxAdaptiveDeltaError = 0.001f;
		}
		else
		{
			wchar_t buf[128];
			m_Settings.CompressionTypes &= ~1;
			m_Settings.CompressionTypes |= IsDlgButtonChecked(dialogRoot, IDC_TIMECODED) == 1;
			m_Settings.CompressionTypes &= ~2;
			m_Settings.CompressionTypes |= IsDlgButtonChecked(dialogRoot, IDC_ADAPTIVEDELTA) != 1 ? 0 : 2;
			m_Settings.ForceKeyReduction = IsDlgButtonChecked(dialogRoot, IDC_FORCEKEYREDUCTION) == 1;
			m_Settings.KeyReduction = (int)SendMessage(GetDlgItem(dialogRoot, IDC_KEYREDUCTION), CB_GETCURSEL, 0, 0) + 1;
			m_MaxTranslation->GetText(buf, 128);
			m_Settings.MaxTranslationError = (float)_wtof(buf);
			m_MaxRotation->GetText(buf, 128);
			m_Settings.MaxRotationError = (float)_wtof(buf);
			m_MaxVisibility->GetText(buf, 128);
			m_Settings.MaxVisibilityError = (float)_wtof(buf) / 100.0f;
			m_MaxError->GetText(buf, 128);
			m_Settings.MaxAdaptiveDeltaError = (float)_wtof(buf);
		}
	}
#endif
}
