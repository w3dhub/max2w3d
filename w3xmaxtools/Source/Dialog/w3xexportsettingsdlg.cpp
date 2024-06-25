#include "general.h"
#include <maxheapdirect.h>
#include <iInstanceMgr.h>
#include <sstream>
#include "Dialog/w3xexportsettingsdlg.h"
#include "filefactoryclass.h"
#include "fileclass.h"
#include "engine_string.h"
#include "resource.h"
#include "w3xutilities.h"
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
			IDC_GEOM_CAM_ORIENT
		};

		return s_ids;
	}


	const std::array<uint16, 7>& GeometryFlagCheckIDs()
	{
		static const std::array<uint16, 7> s_ids 
		{
			IDC_HIDE,
			IDC_TWO_SIDED,
			IDC_SHADOW,
			IDC_VALPHA,
			IDC_Z_NORMAL,
			IDC_KEEP_NORMAL,
			IDC_JOYPADPICK
		};

		return s_ids;
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

	W3DExportSettingsDlg::W3DExportSettingsDlg(W3DUtilities& utilities)
		: m_Utilities(utilities)
		  , m_RollupRoot(nullptr)
		  , m_DialogRoot(nullptr)
		  , m_SelectionEdit(nullptr)
		  , m_StaticSortingSpinner(nullptr)
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

		m_SelectionEdit = GetDlgItem(m_DialogRoot, IDC_SELECTED_EDIT);

		m_StaticSortingSpinner = SetupIntSpinner(m_DialogRoot, IDC_STATIC_SORT_LEVEL_SPIN, IDC_STATIC_SORT_LEVEL_EDIT, 0, 32, 0);
	}

	void W3DExportSettingsDlg::ReleaseControls()
	{
		ReleaseISpinner(m_StaticSortingSpinner);
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
			case IDC_JOYPADPICK:
				ModifyGeometryFlags(W3DGeometryFlags::JoypadPick, IsDlgButtonChecked(m_DialogRoot, controlID));
				return TRUE;
			}
			break;
		} //BN_Clicked
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
			int geoType = enum_to_value(w3dData.GeometryType);

			str->GeometryType[geoType - 1] = true;
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
	}

	void W3DExportSettingsDlg::RefreshUI()
	{
		EnableWindow(GetDlgItem(m_DialogRoot, IDC_SELECTED_EDIT), FALSE);
		if (m_Utilities.SelectedNodes().empty())
		{
			SetWindowText(m_SelectionEdit, L"( Nothing Selected )");
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_TRANSFORM), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_EXPORT_GEOMETRY), FALSE);
			for (int i = 0; i < GeometryTypeRadioIDs().size(); i++)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, GeometryTypeRadioIDs()[i]), FALSE);
			}
			for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
			{
				EnableWindow(GetDlgItem(m_DialogRoot, GeometryFlagCheckIDs()[i]), FALSE);
			}
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORTING), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_LABEL), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_EDIT), FALSE);
			EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORT_LEVEL_SPIN), FALSE);
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
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_STATIC_SORTING), FALSE);
			}
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_TRANSFORM, flags.ExportBone);
			CheckDlgButton(m_DialogRoot, IDC_EXPORT_GEOMETRY, flags.ExportGeometry);
			for (int i = 0; i < GeometryFlagCheckIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryFlagCheckIDs()[i], flags.GeometryFlags[i]);
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
			for (unsigned int i = 0; i < GeometryTypeRadioIDs().size(); i++)
			{
				CheckDlgButton(m_DialogRoot, GeometryTypeRadioIDs()[i], flags.GeometryType[i]);
			}
		}
	}

	void W3DExportSettingsDlg::SetGeometryType(W3DGeometryType type)
	{
		VisitNodeAppData(m_Utilities.SelectedNodes(), [type](W3DAppDataChunk& chunk) { chunk.GeometryType = type; });
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

}