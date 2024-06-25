#include "general.h"
#include <maxheapdirect.h>
#include <iInstanceMgr.h>
#include <sstream>
#include "Dialog/w3dexportsettingsdlg.h"
#include "filefactoryclass.h"
#include "fileclass.h"
#include "engine_string.h"
#include "resource.h"
#include "w3dutilities.h"
#include "iniclass.h"
#include <charconv>

extern HINSTANCE hInstance;

namespace
{
	const std::array<uint16, enum_to_value(W3D::MaxTools::W3DGeometryType::Num)>& GeometryTypeRadioIDs()
	{
		static const std::array<uint16, enum_to_value(W3D::MaxTools::W3DGeometryType::Num)> s_ids
		{
			IDC_GEOM_CAM_PARAL,
			IDC_GEOM_NORMAL,
			IDC_GEOM_OBBOX,
			IDC_GEOM_AABOX,
			IDC_GEOM_CAM_ORIENT,
			IDC_GEOM_NULL_LOD,
			IDC_GEOM_DAZZLE,
			IDC_GEOM_AGGREGATE,
			IDC_GEOM_CAMERA_Z_ORIENTED
		};

		return s_ids;
	}


	const std::array<uint16, 10>& GeometryFlagCheckIDs()
	{
		static const std::array<uint16, 10> s_ids 
		{
			IDC_HIDE,
			IDC_TWO_SIDED,
			IDC_SHADOW,
			IDC_VALPHA,
			IDC_Z_NORMAL,
			IDC_SHATTER,
			IDC_TANGENTS,
			IDC_KEEP_NORMAL,
			IDC_PRELIT,
			IDC_ALWAYSDYNLIGHT
		};

		return s_ids;
	}

	const std::array<uint16, 5>& CollisionFlagCheckIDs()
	{
		static const std::array<uint16, 5> s_ids
		{
			IDC_PHYSICAL,
			IDC_PROJECTILE,
			IDC_VIS,
			IDC_CAMERA,
			IDC_VEHICLE
		};

		return s_ids;
	}

	const std::vector<TSTR>& DazzleStrings()
	{
		static const std::vector<TSTR> s_strings = []() -> std::vector<TSTR>
		{
			std::vector<TSTR> result;
			FileClass* iniFile = _TheFileFactory->Get_File("dazzle.ini");
			assert(iniFile != nullptr);

			if (iniFile)
			{
				INIClass dazzleINI(*iniFile);
				INISection* dazzleList = dazzleINI.Find_Section("Dazzles_List");
				assert(dazzleList);

				if (dazzleList)
				{
					result.reserve(dazzleList->EntryList.Get_Valid_Count());
					for (INIEntry* it = dazzleList->EntryList.First(); it->Is_Valid(); it = it->Next())
					{
						result.emplace_back(CStr(it->Value).ToWStr());
					}
				}
			}

			return result;
		}();

		return s_strings;
	}

	int IndexOfDazzleString(const TCHAR* str)
	{
		auto search_it = std::find(DazzleStrings().cbegin(), DazzleStrings().cend(), str);
		return static_cast<int>(search_it == DazzleStrings().cend() ? 0 : search_it - DazzleStrings().cbegin());
	}

	template <typename FUNC>
	void VisitNodeAppData(const std::vector<INode*>& selectedNodes, FUNC&& f)
	{
		using namespace W3D::MaxTools;

		for (INode* node : selectedNodes)
		{
			INodeTab nodes;
			IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
			for (int i = 0; i < nodes.Count(); ++i)
			{
				f(W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]));
			}
		}
	}

}

namespace W3D::MaxTools
{
	float GetScreenSizeFromNode(INode* node)
	{
#ifdef _DEBUG
		MSTR buf;
		node->GetUserPropBuffer(buf);
#endif
		// NOTE: GetUserPropFloat is *not* usable in any way since it parses the string using the system locale,
		//        which will break when reading a value saved on other systems. std::from_chars is locale independent.

		MSTR size_str(_M("1.0f"));
		if (!node->GetUserPropString(_M("MaxScreenSize"), size_str))
			return 1.0f;

		// This is just for files that were previously saved with a locale using ',' as decimal separator. Future files will not have this problem.
		// We need a plain ASCII char copy anyways for std::from_chars.
		StringClass size_str_fixed(size_str.data(), true);

		bool fixup = false;
		for (int i = 0; i < size_str_fixed.Get_Length(); i++)
		{
			if (size_str_fixed[i] == ',') {
				size_str_fixed[i] = '.';
				fixup = true;
				break;
			}
		}

		float size = 1.0f;
		std::from_chars_result res = std::from_chars(size_str_fixed.Peek_Buffer(), size_str_fixed.Peek_Buffer() + size_str_fixed.Get_Length(), size);
		(void)res;
		return size;
	}


	W3DExportSettingsDlg::W3DExportSettingsDlg(W3DUtilities& utilities)
		: m_Utilities(utilities)
		  , m_RollupRoot(nullptr)
		  , m_DialogRoot(nullptr)
		  , m_DazzleType(nullptr)
		  , m_SelectionEdit(nullptr)
		  , m_StaticSortingSpinner(nullptr)
		  , m_ScreenSizeSpinner(nullptr)
	{
	}

	void W3DExportSettingsDlg::Initialise(Interface * ip)
	{
		if (m_RollupRoot == nullptr)
		{
			m_RollupRoot = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_W3D_UTIL_EXPORT_SETTINGS), DlgProc, L"W3D Export Settings", reinterpret_cast<LPARAM>(this));

			if (m_DialogRoot)
			{
				RefreshUI();
			}
		}
	}

	void W3DExportSettingsDlg::ConnectControls(HWND dialogRoot)
	{
		m_DialogRoot = dialogRoot;

		m_DazzleType = GetDlgItem(m_DialogRoot, IDC_DAZZLE_MODE);
		for (const TSTR& str : DazzleStrings())
		{
			ComboBox_AddString(m_DazzleType, str.data());
		}

		m_SelectionEdit = GetDlgItem(m_DialogRoot, IDC_SELECTED_EDIT);

		m_StaticSortingSpinner = SetupIntSpinner(m_DialogRoot, IDC_STATIC_SORT_LEVEL_SPIN, IDC_STATIC_SORT_LEVEL_EDIT, 0, 32, 0);
		m_ScreenSizeSpinner = SetupFloatSpinner(m_DialogRoot, IDC_SCREEN_SPIN, IDC_SCREEN_EDIT, 0, FLT_MAX, 0);
	}

	void W3DExportSettingsDlg::ReleaseControls()
	{
		ReleaseISpinner(m_StaticSortingSpinner);
		ReleaseISpinner(m_ScreenSizeSpinner);
	}

	void W3DExportSettingsDlg::Close(Interface* ip)
	{
		if (m_RollupRoot)
			ip->DeleteRollupPage(m_RollupRoot);
	}

	INT_PTR W3DExportSettingsDlg::DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		W3DExportSettingsDlg* dlg = reinterpret_cast<W3DExportSettingsDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (message)
		{
		case WM_INITDIALOG:
			dlg = reinterpret_cast<W3DExportSettingsDlg*>(lparam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lparam);
			dlg->ConnectControls(hWnd);
			return TRUE;
		case WM_DESTROY:
			dlg->ReleaseControls();
			return TRUE;
		case WM_COMMAND:
			return dlg->HandleCommand(LOWORD(wparam), HIWORD(wparam));
		case CC_SPINNER_CHANGE:
			return dlg->HandleSpinner(LOWORD(wparam));
		}
		return FALSE;
	}

	INT_PTR W3DExportSettingsDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		switch (commandID)
		{
		case BN_CLICKED:
		{
			switch (controlID)
			{
				//Geometry Type Radio Buttons
			case IDC_GEOM_NORMAL:
				SetGeometryType(W3DGeometryType::Normal);
				return TRUE;
			case IDC_GEOM_CAM_PARAL:
				SetGeometryType(W3DGeometryType::CamParal);
				return TRUE;
			case IDC_GEOM_CAM_ORIENT:
				SetGeometryType(W3DGeometryType::CamOrient);
				return TRUE;
			case IDC_GEOM_AABOX:
				SetGeometryType(W3DGeometryType::AABox);
				return TRUE;
			case IDC_GEOM_OBBOX:
				SetGeometryType(W3DGeometryType::OBBox);
				return TRUE;
			case IDC_GEOM_NULL_LOD:
				SetGeometryType(W3DGeometryType::NullLOD);
				return TRUE;
			case IDC_GEOM_AGGREGATE:
				SetGeometryType(W3DGeometryType::Aggregate);
				return TRUE;
			case IDC_GEOM_DAZZLE:
				SetGeometryType(W3DGeometryType::Dazzle);
				return TRUE;
			case IDC_GEOM_CAMERA_Z_ORIENTED:
				SetGeometryType(W3DGeometryType::CamZOrient);
				return TRUE;

				//Export Flags
			case IDC_EXPORT_GEOMETRY:
				ModifyExportFlags(W3DExportFlags::ExportGeometry, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_EXPORT_TRANSFORM:
				ModifyExportFlags(W3DExportFlags::ExportTransform, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_STATIC_SORTING:
				if (IsDlgButtonChecked(m_DialogRoot, controlID))
				{
					SetStaticSortLevel(1);
				}
				else
				{
					SetStaticSortLevel(0);
				}
				return TRUE;

				//Geometry Flags
			case IDC_TWO_SIDED:
				ModifyGeometryFlags(W3DGeometryFlags::TwoSided, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_HIDE:
				ModifyGeometryFlags(W3DGeometryFlags::Hide, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_Z_NORMAL:
				ModifyGeometryFlags(W3DGeometryFlags::ZNormal, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_KEEP_NORMAL:
				ModifyGeometryFlags(W3DGeometryFlags::KeepNml, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_VALPHA:
				ModifyGeometryFlags(W3DGeometryFlags::VAlpha, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_SHADOW:
				ModifyGeometryFlags(W3DGeometryFlags::Shadow, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_SHATTER:
				ModifyGeometryFlags(W3DGeometryFlags::Shatter, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_TANGENTS:
				ModifyGeometryFlags(W3DGeometryFlags::Tangents, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_PRELIT:
				ModifyGeometryFlags(W3DGeometryFlags::Prelit, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_ALWAYSDYNLIGHT:
				ModifyGeometryFlags(W3DGeometryFlags::AlwaysDynLight, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;

				// Collision Flags
			case IDC_PHYSICAL:
				ModifyCollisionFlags(W3DCollisionFlags::Physical, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_PROJECTILE:
				ModifyCollisionFlags(W3DCollisionFlags::Projectile, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_VEHICLE:
				ModifyCollisionFlags(W3DCollisionFlags::Vehicle, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_VIS:
				ModifyCollisionFlags(W3DCollisionFlags::Vis, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			case IDC_CAMERA:
				ModifyCollisionFlags(W3DCollisionFlags::Camera, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			}
			break;
		} //BN_Clicked
		case CBN_SELCHANGE:
		{
			switch (controlID)
			{
			case IDC_DAZZLE_MODE:
				SetDazzleType(DazzleStrings()[ComboBox_GetCurSel(m_DazzleType)]);
				return TRUE;
			}
			break;
		} //CBN_SELCHANGE
		}

		return FALSE;
	}

	INT_PTR W3DExportSettingsDlg::HandleSpinner(uint16 controlID)
	{
		switch (controlID)
		{
		case IDC_STATIC_SORT_LEVEL_SPIN:
			SetStaticSortLevel(m_StaticSortingSpinner->GetIVal());
			return TRUE;
		case IDC_SCREEN_SPIN:
			SetScreenSize(m_ScreenSizeSpinner->GetFVal());
			return TRUE;
		}

		return FALSE;
	}

	struct W3DExportFlagsStruct
	{
		int ExportBone;
		int ExportGeometry;
		bool GeometryType[size_t(W3D::MaxTools::W3DGeometryType::Num)];
		int GeometryFlags[10];
		int CollisionFlags[5];
		int StaticSortLevel;
		int DazzleCount;
		char DazzleTypeName[128];
	};

	void W3DExportSettingsDlg::GetW3DExportFlags(W3DExportFlagsStruct *str)
	{
		str->ExportBone = 0;
		str->ExportGeometry = 0;
		for (int i = 0; i < ARRAYSIZE(str->GeometryFlags); i++)
		{
			str->GeometryFlags[i] = 0;
		}
		for (int i = 0; i < ARRAYSIZE(str->GeometryType); i++)
		{
			str->GeometryType[i] = false;
		}
		for (int i = 0; i < ARRAYSIZE(str->CollisionFlags); i++)
		{
			str->CollisionFlags[i] = 0;
		}
		str->StaticSortLevel = 0;
		str->DazzleCount = 0;
		if (m_Utilities.SelectedNodes().empty())
		{
			strcpy(str->DazzleTypeName, "DEFAULT");
		}
		else
		{
			const char *dazzle = W3DUtilities::GetDazzleTypeFromAppData(m_Utilities.SelectedNodes()[0]);
			if (!dazzle)
			{
				strcpy(str->DazzleTypeName, "DEFAULT");
			}
			else
			{
				strcpy(str->DazzleTypeName, dazzle);
			}
		}
		for (int i = 0;i < m_Utilities.SelectedNodes().size();i++)
		{
			INode* curNode = m_Utilities.SelectedNodes()[i];
			const W3DAppDataChunk &w3dData = W3DUtilities::GetOrCreateW3DAppDataChunk(*curNode);
			str->ExportBone += enum_has_flags(w3dData.ExportFlags, W3DExportFlags::ExportTransform);
			str->ExportGeometry += enum_has_flags(w3dData.ExportFlags, W3DExportFlags::ExportGeometry);
			for (int f = 0; f < GeometryFlagCheckIDs().size(); ++f)
			{
				str->GeometryFlags[f] += (((1 << f) & enum_to_value(w3dData.GeometryFlags)) == (1 << f));
			}
			for (int f = 0; f < CollisionFlagCheckIDs().size(); ++f)
			{
				str->CollisionFlags[f] += (((1 << f) & enum_to_value(w3dData.CollisionFlags)) == (1 << f));
			}
			int geoType = enum_to_value(w3dData.GeometryType);

			// this is a fix for objects that somehow have a Geometry Type of 0. (These tend to be things imported from the Max 8 plugin, for some reason)
			if (geoType == 0)
				geoType = 2;

			str->GeometryType[geoType - 1] = true;
			if (w3dData.GeometryType == W3DGeometryType::Dazzle)
			{
				str->DazzleCount += 1;
			}
			if (i)
			{
				if (str->StaticSortLevel != w3dData.StaticSortLevel)
				{
					str->StaticSortLevel = -1;
				}
			}
			else
			{
				str->StaticSortLevel = w3dData.StaticSortLevel;
			}
			if (!W3DUtilities::GetDazzleTypeFromAppData(curNode) || strcmp(str->DazzleTypeName, W3DUtilities::GetDazzleTypeFromAppData(curNode)))
			{
				strcpy(str->DazzleTypeName, "DEFAULT");
			}
		}
		int count = (int)m_Utilities.SelectedNodes().size();
		if (str->ExportBone)
		{
			str->ExportBone = (str->ExportBone != count) + 1;
		}
		else
		{
			str->ExportBone = 0;
		}
		if (str->ExportGeometry)
		{
			str->ExportGeometry = (str->ExportGeometry != count) + 1;
		}
		else
		{
			str->ExportGeometry = 0;
		}
		for (int f = 0; f < GeometryFlagCheckIDs().size(); ++f)
		{
			if (str->GeometryFlags[f])
			{
				str->GeometryFlags[f] = (str->GeometryFlags[f] != count) + 1;
			}
			else
			{
				str->GeometryFlags[f] = 0;
			}
		}
		for (int f = 0; f < CollisionFlagCheckIDs().size(); ++f)
		{
			if (str->CollisionFlags[f])
			{
				str->CollisionFlags[f] = (str->CollisionFlags[f] != count) + 1;
			}
			else
			{
				str->CollisionFlags[f] = 0;
			}
		}
	}

	void W3DExportSettingsDlg::RefreshUI()
	{
		EnableWindow(GetDlgItem(m_DialogRoot, IDC_SELECTED_EDIT), FALSE);
		if (m_Utilities.SelectedNodes().empty())
		{
			SetWindowText(m_SelectionEdit, L"( Nothing Selected )");
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_TRANSFORM), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_GEOMETRY), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_EDIT), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_SPIN), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_LABEL), FALSE);
			for (int i = 0; i < GeometryTypeRadioIDs().size(); i++)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, GeometryTypeRadioIDs()[i]), FALSE);
			}
			for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, GeometryFlagCheckIDs()[i]), FALSE);
			}
			for (int i = 0; i < CollisionFlagCheckIDs().size(); i++)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, CollisionFlagCheckIDs()[i]), FALSE);
			}
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORTING), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_LABEL), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_EDIT), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_SPIN), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_DAZZLE_MODE), FALSE);
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_TRANSFORM, BST_UNCHECKED);
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_GEOMETRY, BST_UNCHECKED);
			for (int i = 0; i < GeometryTypeRadioIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryTypeRadioIDs()[i], BST_UNCHECKED);
			}
			for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryFlagCheckIDs()[i], BST_UNCHECKED);
			}
			for (int i = 0; i < CollisionFlagCheckIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, CollisionFlagCheckIDs()[i], BST_UNCHECKED);
			}
		}
		else
		{
			if (m_Utilities.SelectedNodes().size() == 1)
			{
				SetWindowText(m_SelectionEdit, m_Utilities.SelectedNodes().front()->GetName());
			}
			else
			{
				std::wstringstream selection_text;
				selection_text << m_Utilities.SelectedNodes().size() << L" - Objects Selected";
				SetWindowText(m_SelectionEdit, selection_text.str().data());
			}
			W3DExportFlagsStruct flags;
			GetW3DExportFlags(&flags);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_TRANSFORM), TRUE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_GEOMETRY), TRUE);
			if (flags.ExportGeometry == 1)
			{
				for (int i = 0; i < GeometryTypeRadioIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, GeometryTypeRadioIDs()[i]), TRUE);
				}
				for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, GeometryFlagCheckIDs()[i]), TRUE);
				}
				for (int i = 0; i < CollisionFlagCheckIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, CollisionFlagCheckIDs()[i]), TRUE);
				}
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORTING), TRUE);
			}
			else
			{
				for (int i = 0; i < GeometryTypeRadioIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, GeometryTypeRadioIDs()[i]), FALSE);
				}
				for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, GeometryFlagCheckIDs()[i]), FALSE);
				}
				for (int i = 0; i < CollisionFlagCheckIDs().size(); i++)
				{
					EnableWindow(GetDlgItem(m_DialogRoot, CollisionFlagCheckIDs()[i]), FALSE);
				}
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORTING), FALSE);
			}
			if (flags.ExportBone && m_Utilities.SelectedNodes().size() == 1 && !flags.ExportGeometry)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_LABEL), TRUE);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_EDIT), TRUE);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_SPIN), TRUE);
				float size = GetScreenSizeFromNode(m_Utilities.SelectedNodes().front());
				m_ScreenSizeSpinner->SetValue(size, 0);
			}
			else
			{
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_LABEL), FALSE);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_EDIT), FALSE);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SCREEN_SPIN), FALSE);
				m_ScreenSizeSpinner->SetValue(0, 0);
			}
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_TRANSFORM, flags.ExportBone);
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_GEOMETRY, flags.ExportGeometry);
			for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryFlagCheckIDs()[i], flags.GeometryFlags[i]);
			}
			for (int i = 0; i < CollisionFlagCheckIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, CollisionFlagCheckIDs()[i], flags.CollisionFlags[i]);
			}
			bool enable = false;
			if (flags.ExportGeometry == 1)
			{
				if (flags.StaticSortLevel == -1)
				{
					m_StaticSortingSpinner->SetIndeterminate(true);
					CheckDlgButton(m_DialogRoot, IDC_STATIC_SORTING, BST_INDETERMINATE);
				}
				else
				{
					if (flags.StaticSortLevel)
					{
						CheckDlgButton(m_DialogRoot, IDC_STATIC_SORTING, BST_CHECKED);
						m_StaticSortingSpinner->SetIndeterminate(false);
						m_StaticSortingSpinner->SetValue(flags.StaticSortLevel, 0);
						enable = true;
					}
					else
					{
						CheckDlgButton(m_DialogRoot, IDC_STATIC_SORTING, BST_UNCHECKED);
						m_StaticSortingSpinner->SetValue(0, 0);
					}
				}
			}
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_LABEL), enable);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_EDIT), enable);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_SPIN), enable);
			bool dazzle = false;
			if (flags.ExportGeometry == 1 && flags.DazzleCount == m_Utilities.SelectedNodes().size())
			{
				dazzle = true;
			}
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_DAZZLE_MODE), dazzle);
			WideStringClass dazzleType = flags.DazzleTypeName;
			ComboBox_SetCurSel(m_DazzleType, IndexOfDazzleString(dazzleType));
			for (unsigned int i = 0; i < GeometryTypeRadioIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryTypeRadioIDs()[i], flags.GeometryType[i]);
			}
		}
	}

	void W3DExportSettingsDlg::SetScreenSize(float size)
	{
		if (m_Utilities.SelectedNodes().size() == 1)
		{
			// NOTE: SetUserPropFloat is *not* usable in any way since it prints the value using the system locale,
			//        which will break reading the value back on other systems. std::to_chars is locale independent.
			constexpr size_t buf_size = 78;
			char buf[buf_size];
			std::to_chars_result res = std::to_chars(buf, buf + buf_size, size);
			*res.ptr = '\0'; // to_chars does not null terminate!

			MSTR str = MSTR::FromCStr(buf);
			m_Utilities.SelectedNodes().front()->SetUserPropString(_M("MaxScreenSize"), str);
#ifdef _DEBUG
			MSTR userpropbuf;
			m_Utilities.SelectedNodes().front()->GetUserPropBuffer(userpropbuf);
#endif
		}
	}

	void W3DExportSettingsDlg::SetDazzleType(const TSTR& dazzle)
	{
		for (INode* node : m_Utilities.SelectedNodes())
		{
			CStr str = dazzle.ToCStr();
			W3DUtilities::SetDazzleTypeInAppData(node, str);
		}
		RefreshUI();
	}

	void W3DExportSettingsDlg::SetGeometryType(W3DGeometryType type)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(), [type](W3DAppDataChunk& chunk) { chunk.GeometryType = type; });
		EnableWindow(m_DazzleType, type == W3DGeometryType::Dazzle);
		RefreshUI();
	}

	void W3DExportSettingsDlg::SetStaticSortLevel(int sortLevel)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(), [sortLevel](W3DAppDataChunk& chunk) { chunk.StaticSortLevel = sortLevel; });
		RefreshUI();
	}

	void W3DExportSettingsDlg::ModifyExportFlags(W3DExportFlags flags, bool add)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(),
			[add, flags](W3DAppDataChunk& chunk)
		{
			chunk.ExportFlags = add ? (chunk.ExportFlags | flags) : (chunk.ExportFlags & ~flags);
		}
		);

		RefreshUI();
	}

	void W3DExportSettingsDlg::ModifyGeometryFlags(W3DGeometryFlags flags, bool add)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(),
			[add, flags](W3DAppDataChunk& chunk)
		{
			chunk.GeometryFlags = add ? (chunk.GeometryFlags | flags) : (chunk.GeometryFlags & ~flags);
		});

		RefreshUI();
	}

	void W3DExportSettingsDlg::ModifyCollisionFlags(W3DCollisionFlags flags, bool add)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(),
			[add, flags](W3DAppDataChunk& chunk)
		{
			chunk.CollisionFlags = add ? (chunk.CollisionFlags | flags) : (chunk.CollisionFlags & ~flags);
		});

		RefreshUI();
	}
}