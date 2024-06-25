#include "general.h"
#include <unordered_set>
#include <iInstanceMgr.h>
#include "Dialog/w3dmiscutilitiesdlg.h"
#include "resource.h"
#ifndef W3X
#include "w3dappdatachunk.h"
#include "w3dutilities.h"
#else
#include "w3xappdatachunk.h"
#include "w3xutilities.h"
#endif
#include "w3dmaterial.h"
#include "w3dmaxcommonhelpers.h"


namespace W3D::MaxTools
{
	W3DMiscUtilitiesDlg::W3DMiscUtilitiesDlg(W3DUtilities & utilities)
		: m_Utilities(utilities)
		, m_RollupRoot(nullptr)
		, m_DialogRoot(nullptr)
	{ }

	void W3DMiscUtilitiesDlg::Initialise(Interface * ip)
	{
		if (m_RollupRoot == nullptr)
		{
			m_RollupRoot = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_W3D_UTIL_TOOLS), DlgProc, L"W3D Tools", reinterpret_cast<LPARAM>(this));
		}
	}

	void W3DMiscUtilitiesDlg::Close(Interface * ip)
	{
		if (m_RollupRoot)
			ip->DeleteRollupPage(m_RollupRoot);
	}

	INT_PTR W3DMiscUtilitiesDlg::DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		W3DMiscUtilitiesDlg* dlg = reinterpret_cast<W3DMiscUtilitiesDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_INITDIALOG:
			dlg = reinterpret_cast<W3DMiscUtilitiesDlg*>(lparam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lparam);
			dlg->ConnectControls(hWnd);
			break;
		case WM_COMMAND:
			dlg->HandleCommand(LOWORD(wparam), HIWORD(wparam));
			break;
		}
		return FALSE;
	}

	void W3DMiscUtilitiesDlg::ConnectControls(HWND dialogRoot)
	{
		m_DialogRoot = dialogRoot;
	}

	INT_PTR W3DMiscUtilitiesDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		switch (controlID)
		{
		case IDC_SELECT_GEOMETRY:
			SetSelectionOnExportFlags(W3DExportFlags::ExportGeometry);
			return TRUE;
		case IDC_SELECT_BONES:
			SetSelectionOnExportFlags(W3DExportFlags::ExportTransform);
			return TRUE;
		case IDC_SELECT_ALPHA_MESHES:
			SelectAlphaObjects();
			return TRUE;
#ifndef W3X
		case IDC_SELECT_PHYS:
			SetSelectionOnCollisionFlags(W3DCollisionFlags::Physical);
			return TRUE;
		case IDC_SELECT_PROJ:
			SetSelectionOnCollisionFlags(W3DCollisionFlags::Projectile);
			return TRUE;
		case IDC_SELECT_VIS:
			SetSelectionOnCollisionFlags(W3DCollisionFlags::Vis);
			return TRUE;
#endif
		case IDC_ASSIGN_NODE_NAMES:
			DoNodeNameAssignment();
			m_Utilities.RefreshExportSettings();
			return TRUE;
		case IDC_ASSIGN_MATERIAL_NAMES:
			DoMaterialNameAssignment();
			m_Utilities.RefreshExportSettings();
			return TRUE;
		case IDC_ASSIGN_EXTENSIONS:
			DoExtensionNameAssignment();
			m_Utilities.RefreshExportSettings();
			return TRUE;
		}

		return FALSE;
	}

	void W3DMiscUtilitiesDlg::SetSelectionOnGeometryType(W3DGeometryType type)
	{
		INodeTab selectedNodes;
		VisitSceneNodes([&selectedNodes, type](INode& node)
		{
			if (W3DUtilities::GetOrCreateW3DAppDataChunk(node).GeometryType == type)
			{
				selectedNodes.AppendNode(&node, true);
			}
		});

		SetSelectedSceneNodes(selectedNodes);
	}

	void W3DMiscUtilitiesDlg::SetSelectionOnExportFlags(W3DExportFlags flags)
	{
		INodeTab selectedNodes;
		VisitSceneNodes([&selectedNodes, flags](INode& node)
		{
			if (!node.IsGroupHead())
			{
				if (enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(node).ExportFlags, flags))
				{
					selectedNodes.AppendNode(&node, true);
				}
			}
		});

		SetSelectedSceneNodes(selectedNodes);
	}

#ifndef W3X
	void W3DMiscUtilitiesDlg::SetSelectionOnCollisionFlags(W3DCollisionFlags flags)
	{
		INodeTab selectedNodes;
		VisitSceneNodes([&selectedNodes, flags](INode& node)
		{
			if (enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(node).CollisionFlags, flags))
			{
				selectedNodes.AppendNode(&node, true);
			}
		});

		SetSelectedSceneNodes(selectedNodes);
	}
#endif

	void W3DMiscUtilitiesDlg::SelectAlphaObjects()
	{
		INodeTab selectedNodes;
		VisitSceneNodes([&selectedNodes](INode& node)
		{

			for (int i = 0; i < node.NumMtls(); ++i)
			{
				bool push = false;
				Mtl* mtl = node.GetMtl();
				if (mtl && mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
				{
					W3DMaterial* w3dmtl = static_cast<W3DMaterial*>(mtl);
					const int numActivePasses = w3dmtl->NumActivePasses();
					for (int j = 0; j < numActivePasses; ++j)
					{
						IParamBlock2* pb = w3dmtl->GetMaterialPass(j).ParamBlock;
						if (pb && (pb->GetInt(enum_to_value(W3DMaterialParamID::Stage0AlphaBitmap)) || pb->GetInt(enum_to_value(W3DMaterialParamID::Stage1AlphaBitmap))))
						{
							push = true;
							break;
						}
					}

					if (push)
					{
						selectedNodes.AppendNode(&node, true);
						break;
					}
				}
			}
		});

		SetSelectedSceneNodes(selectedNodes);
	}

	void W3DMiscUtilitiesDlg::DoNodeNameAssignment()
	{
		static W3DNodeNameParamDlg params;
		if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_UTIL_NODE_NAMES), GetCOREInterface()->GetMAXHWnd(), W3DNodeNameParamDlg::DlgProc, reinterpret_cast<LPARAM>(&params)) == IDOK)
		{
			//Early out if there's nothing to do
#ifndef W3X
			if (!(params.m_AssignNames || params.m_Suffix || params.m_AssignPrefix || params.m_AssignCollisionFlags))
				return;
#else
			if (!(params.m_AssignNames || params.m_Suffix || params.m_AssignPrefix))
				return;
#endif
			std::vector<INode*> targetNodes;
			if (params.m_AffectAll)
			{
				VisitSceneNodes([&targetNodes](INode& node)
				{
					targetNodes.emplace_back(&node);
				});
			}
			else if (!m_Utilities.SelectedNodes().empty())
			{
				targetNodes.assign(m_Utilities.SelectedNodes().cbegin(), m_Utilities.SelectedNodes().cend());
			}

			if (!targetNodes.empty())
			{
				TSTR nodeText;
				for (int i = 0; i < targetNodes.size(); ++i)
				{
					INode& curNode = *targetNodes[i];

					if (params.m_AssignNames || params.m_Suffix || params.m_AssignPrefix)
					{
						if (params.m_AssignNames)
						{
							nodeText.printf(_T("%s%s%03d%s"),
								(params.m_AssignPrefix ? params.m_Prefix.data() : _T("")),
								params.m_RootName.data(),
								params.m_StartingIndex + i,
								(params.m_AssignSuffix ? params.m_Suffix.data() : _T(""))
							);
						}
						else
						{
							nodeText = (params.m_AssignPrefix ? params.m_Prefix : _T("")) + curNode.NodeName() + (params.m_AssignSuffix ? params.m_Suffix : _T(""));
						}

						curNode.SetName(nodeText);
					}

#ifndef W3X
					if (params.m_AssignCollisionFlags)
					{
						INodeTab nodes;
						IInstanceMgr::GetInstanceMgr()->GetInstances(curNode, nodes);
						for (int i = 0; i < nodes.Count(); ++i)
						{
							W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags = params.m_CollisionFlags;
						}
					}
#endif
				}
			}
		}
	}

	void W3DMiscUtilitiesDlg::DoMaterialNameAssignment()
	{
		static W3DMaterialNameParamDlg params;
		if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_UTIL_MTL_NAMES), GetCOREInterface()->GetMAXHWnd(), W3DMaterialNameParamDlg::DlgProc, reinterpret_cast<LPARAM>(&params)) == IDOK)
		{
			std::unordered_set<Mtl*> targetMtls;
			if (params.m_AffectAll)
			{
				VisitSceneNodes([&targetMtls](INode& node)
				{
					if (node.GetMtl())
						targetMtls.emplace(node.GetMtl());
				});
			}
			else
			{
				for (INode * node : m_Utilities.SelectedNodes())
				{
					if (node->GetMtl())
						targetMtls.emplace(node->GetMtl());
				}
			}

			if (!targetMtls.empty())
			{
				TSTR matName;
				int currentIdx = params.m_StartingIndex;
				for (Mtl* mtl : targetMtls)
				{
					matName.printf(_T("%s.%02d"), params.m_RootName.data(), currentIdx++);
					mtl->SetName(matName);
				}
			}
		}
	}

	void W3DMiscUtilitiesDlg::DoExtensionNameAssignment()
	{
		static W3DNameExtensionParamDlg params;

		if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_UTIL_NAME_EXTENSION), GetCOREInterface()->GetMAXHWnd(), W3DNameExtensionParamDlg::DlgProc, reinterpret_cast<LPARAM>(&params)) == IDOK)
		{
			TSTR nodeText;
			for (INode* curNode : m_Utilities.SelectedNodes())
			{
				nodeText.printf(_T("%s.%02d"), curNode->GetName(), params.m_ExtensionNumber);
				curNode->SetName(nodeText);
				
			}
		}
	}

	// W3DNodeNameParamDlg ////////////////////////
	W3DNodeNameParamDlg::W3DNodeNameParamDlg()
		: m_AssignNames(true)
		  , m_AssignPrefix(false)
		  , m_AssignSuffix(false)
		  , m_AssignCollisionFlags(false), m_AffectAll(false)
		  , m_CollisionFlags(
			  W3DCollisionFlags::Physical | W3DCollisionFlags::Projectile | W3DCollisionFlags::Vehicle |
			  W3DCollisionFlags::Vis | W3DCollisionFlags::Camera)
		  , m_StartingIndex(0)
		  , m_RootName()
		  , m_Prefix()
		  , m_Suffix()
		  , m_DialogRoot(nullptr)
		  , m_RootNameEdit(nullptr), m_PrefixEdit(nullptr), m_SuffixEdit(nullptr), m_StartingIndexSpinner(nullptr)
	{
		GetCurrentFilename(m_RootName);
	}

	INT_PTR W3DNodeNameParamDlg::DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		W3DNodeNameParamDlg* params = reinterpret_cast<W3DNodeNameParamDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (message)
		{
		case WM_INITDIALOG:
			params = reinterpret_cast<W3DNodeNameParamDlg*>(lparam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lparam);
			params->Initialise(hWnd);
			break;
		case WM_DESTROY:
			params->ReleaseControls();
			break;
		case WM_COMMAND:
			params->HandleCommand(LOWORD(wparam), HIWORD(wparam));
			break;
		case CC_SPINNER_CHANGE:
			params->HandleSpinner(LOWORD(wparam));
			break;
		}

		return FALSE;
	}

	void W3DNodeNameParamDlg::Initialise(HWND dialogRoot)
	{
		m_DialogRoot = dialogRoot;
		CheckDlgButton(dialogRoot, IDC_ASSIGN_NAMES, m_AssignNames);
		m_RootNameEdit = GetICustEdit(GetDlgItem(dialogRoot, IDC_ROOT_NAME));
		m_RootNameEdit->SetText(m_RootName);
		EnableWindow(GetDlgItem(dialogRoot, IDC_ROOT_NAME), m_AssignNames);

		m_StartingIndexSpinner = SetupIntSpinner(dialogRoot, IDC_STARTING_INDEX_SPIN, IDC_STARTING_INDEX_EDIT, 0, 999, m_StartingIndex);
		m_StartingIndexSpinner->Enable(m_AssignNames);

		CheckDlgButton(dialogRoot, IDC_ASSIGN_PREFIX, m_AssignPrefix);
		m_PrefixEdit = GetICustEdit(GetDlgItem(dialogRoot, IDC_PREFIX));
		m_PrefixEdit->SetText(m_Prefix);

		CheckDlgButton(dialogRoot, IDC_ASSIGN_SUFFIX, m_AssignSuffix);
		m_SuffixEdit = GetICustEdit(GetDlgItem(dialogRoot, IDC_SUFFIX));
		m_SuffixEdit->SetText(m_Suffix.data());
		EnableWindow(GetDlgItem(dialogRoot, IDC_SUFFIX), m_AssignSuffix);

		CheckDlgButton(dialogRoot, IDC_ASSIGN_COLLISION_FLAGS, m_AssignCollisionFlags);
		CheckDlgButton(dialogRoot, IDC_PHYSICAL, enum_has_flags(m_CollisionFlags, W3DCollisionFlags::Physical));
		CheckDlgButton(dialogRoot, IDC_PROJECTILE, enum_has_flags(m_CollisionFlags, W3DCollisionFlags::Projectile));
#ifndef W3X
		CheckDlgButton(dialogRoot, IDC_VEHICLE, enum_has_flags(m_CollisionFlags, W3DCollisionFlags::Vehicle));
#endif
		CheckDlgButton(dialogRoot, IDC_VIS, enum_has_flags(m_CollisionFlags, W3DCollisionFlags::Vis));
#ifndef W3X
		CheckDlgButton(dialogRoot, IDC_CAMERA, enum_has_flags(m_CollisionFlags, W3DCollisionFlags::Camera));
#endif
		
		EnableWindow(GetDlgItem(dialogRoot, IDC_PHYSICAL), m_AssignCollisionFlags);
		EnableWindow(GetDlgItem(dialogRoot, IDC_PROJECTILE), m_AssignCollisionFlags);
#ifndef W3X
		EnableWindow(GetDlgItem(dialogRoot, IDC_VEHICLE), m_AssignCollisionFlags);
#endif
		EnableWindow(GetDlgItem(dialogRoot, IDC_VIS), m_AssignCollisionFlags);
#ifndef W3X
		EnableWindow(GetDlgItem(dialogRoot, IDC_CAMERA), m_AssignCollisionFlags);
#endif

		CheckRadioButton(dialogRoot, IDC_AFFECT_ALL, IDC_AFFECT_SELECTED, m_AffectAll ? IDC_AFFECT_ALL : IDC_AFFECT_SELECTED);
	}

	void W3DNodeNameParamDlg::ReleaseControls()
	{
		ReleaseICustEdit(m_RootNameEdit);
		ReleaseICustEdit(m_PrefixEdit);
		ReleaseICustEdit(m_SuffixEdit);
		ReleaseISpinner(m_StartingIndexSpinner);
	}

	INT_PTR W3DNodeNameParamDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		if (commandID == EN_CHANGE)
		{
			switch (controlID)
			{
			case IDC_ROOT_NAME:
				m_RootNameEdit->GetText(m_RootName);
				return TRUE;
			case IDC_SUFFIX:
				m_SuffixEdit->GetText(m_Suffix);
				return TRUE;
			case IDC_PREFIX:
				m_PrefixEdit->GetText(m_Prefix);
				return TRUE;
			}
		}
		else
		{
			switch (controlID)
			{
			case IDC_ASSIGN_NAMES:
				m_AssignNames = IsDlgButtonChecked(m_DialogRoot, controlID);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_ROOT_NAME), m_AssignNames);
				m_StartingIndexSpinner->Enable(m_AssignNames);
				return TRUE;
			case IDC_ASSIGN_PREFIX:
				m_AssignPrefix = IsDlgButtonChecked(m_DialogRoot, controlID);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_PREFIX), m_AssignPrefix);
				return TRUE;
			case IDC_ASSIGN_SUFFIX:
				m_AssignSuffix = IsDlgButtonChecked(m_DialogRoot, controlID);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_SUFFIX), m_AssignSuffix);
				return TRUE;
			case IDC_ASSIGN_COLLISION_FLAGS:
				m_AssignCollisionFlags = IsDlgButtonChecked(m_DialogRoot, controlID);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_PHYSICAL), m_AssignCollisionFlags);
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_PROJECTILE), m_AssignCollisionFlags);
#ifndef W3X
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_VEHICLE), m_AssignCollisionFlags);
#endif
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_VIS), m_AssignCollisionFlags);
#ifndef W3X
				EnableWindow(GetDlgItem(m_DialogRoot, IDC_CAMERA), m_AssignCollisionFlags);
#endif
				return TRUE;
			case IDC_AFFECT_ALL: //Fallthrough
			case IDC_AFFECT_SELECTED:
				m_AffectAll = IsDlgButtonChecked(m_DialogRoot, IDC_AFFECT_ALL);
				return TRUE;
			case IDC_PHYSICAL:
				m_CollisionFlags = (IsDlgButtonChecked(m_DialogRoot, controlID) ? m_CollisionFlags | W3DCollisionFlags::Physical : m_CollisionFlags & ~W3DCollisionFlags::Physical);
				return TRUE;
			case IDC_PROJECTILE:
				m_CollisionFlags = (IsDlgButtonChecked(m_DialogRoot, controlID) ? m_CollisionFlags | W3DCollisionFlags::Projectile : m_CollisionFlags & ~W3DCollisionFlags::Projectile);
				return TRUE;
#ifndef W3X
			case IDC_VEHICLE:
				m_CollisionFlags = (IsDlgButtonChecked(m_DialogRoot, controlID) ? m_CollisionFlags | W3DCollisionFlags::Vehicle : m_CollisionFlags & ~W3DCollisionFlags::Vehicle);
				return TRUE;
#endif
			case IDC_VIS:
				m_CollisionFlags = (IsDlgButtonChecked(m_DialogRoot, controlID) ? m_CollisionFlags | W3DCollisionFlags::Vis : m_CollisionFlags & ~W3DCollisionFlags::Vis);
				return TRUE;
#ifndef W3X
			case IDC_CAMERA:
				m_CollisionFlags = (IsDlgButtonChecked(m_DialogRoot, controlID) ? m_CollisionFlags | W3DCollisionFlags::Camera : m_CollisionFlags & ~W3DCollisionFlags::Camera);
				return TRUE;
#endif
			case IDOK:
			case IDCANCEL:
			case IDCLOSE:
				EndDialog(m_DialogRoot, controlID);
				return TRUE;
			}
		}

		return FALSE;
	}

	INT_PTR W3DNodeNameParamDlg::HandleSpinner(uint16 controlID)
	{
		switch (controlID)
		{
		case IDC_STARTING_INDEX_SPIN:
			m_StartingIndex = m_StartingIndexSpinner->GetIVal();
			break;
		}

		return FALSE;
	}

	// W3DMaterialNameParamDlg ///////////////////////////////
	W3DMaterialNameParamDlg::W3DMaterialNameParamDlg()
		: m_AffectAll(false)
		, m_StartingIndex(0)
		, m_RootName()
		, m_DialogRoot(nullptr)
		, m_RootNameEdit(nullptr)
		, m_StartingIndexSpinner(nullptr)
	{
		GetCurrentFilename(m_RootName);
	}

	INT_PTR W3DMaterialNameParamDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		W3DMaterialNameParamDlg* params = reinterpret_cast<W3DMaterialNameParamDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (message)
		{
		case WM_INITDIALOG:
			params = reinterpret_cast<W3DMaterialNameParamDlg*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			params->Initialise(hWnd);
			return TRUE;
		case WM_COMMAND:
			return params->HandleCommand(LOWORD(wParam), HIWORD(wParam));
		case CC_SPINNER_CHANGE:
			return params->HandleSpinner(LOWORD(wParam));
		case WM_DESTROY:
			params->ReleaseControls();
			return TRUE;
		}

		return FALSE;
	}
	void W3DMaterialNameParamDlg::Initialise(HWND dialogRoot)
	{
		m_DialogRoot = dialogRoot;
		m_RootNameEdit = GetICustEdit(GetDlgItem(dialogRoot, IDC_ROOT_NAME));
		m_RootNameEdit->SetText(m_RootName);

		m_StartingIndexSpinner = SetupIntSpinner(dialogRoot, IDC_STARTING_INDEX_SPIN, IDC_STARTING_INDEX_EDIT, 0, 999, m_StartingIndex);

		CheckRadioButton(dialogRoot, IDC_AFFECT_ALL, IDC_AFFECT_SELECTED, m_AffectAll ? IDC_AFFECT_ALL : IDC_AFFECT_SELECTED);
	}

	void W3DMaterialNameParamDlg::ReleaseControls()
	{
		ReleaseICustEdit(m_RootNameEdit);
		ReleaseISpinner(m_StartingIndexSpinner);
	}

	INT_PTR W3DMaterialNameParamDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		if (commandID == EN_CHANGE)
		{
			switch (controlID)
			{
			case IDC_ROOT_NAME:
				m_RootNameEdit->GetText(m_RootName);
				return TRUE;
			}
		}
		else
		{
			switch (controlID)
			{
			case IDC_AFFECT_ALL: //Fallthrough
			case IDC_AFFECT_SELECTED:
				m_AffectAll = IsDlgButtonChecked(m_DialogRoot, IDC_AFFECT_ALL);
				return TRUE;
			case IDOK:
			case IDCANCEL:
			case IDCLOSE:
				EndDialog(m_DialogRoot, controlID);
				return TRUE;
			}
		}
		
		return FALSE;
	}

	INT_PTR W3D::MaxTools::W3DMaterialNameParamDlg::HandleSpinner(uint16 controlID)
	{
		switch (controlID)
		{
		case IDC_STARTING_INDEX_SPIN:
			m_StartingIndex = m_StartingIndexSpinner->GetIVal();
			break;
		}

		return FALSE;
	}

	// W3DNameExtensionParamDlg /////////////////////////////////////
	W3DNameExtensionParamDlg::W3DNameExtensionParamDlg()
		: m_ExtensionNumber(0)
		, m_DialogRoot(nullptr)
		, m_ExtensionNumberSpinner(nullptr)
	{ }

	INT_PTR W3DNameExtensionParamDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		W3DNameExtensionParamDlg* params = reinterpret_cast<W3DNameExtensionParamDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (message)
		{
		case WM_INITDIALOG:
			params = reinterpret_cast<W3DNameExtensionParamDlg*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			params->Initialise(hWnd);
			return TRUE;
		case WM_COMMAND:
			return params->HandleCommand(LOWORD(wParam), HIWORD(wParam));
		case CC_SPINNER_CHANGE:
			return params->HandleSpinner(LOWORD(wParam));
		case WM_DESTROY:
			params->ReleaseControls();
			return TRUE;
		}

		return FALSE;
	}

	void W3DNameExtensionParamDlg::Initialise(HWND dialogRoot)
	{
		m_DialogRoot = dialogRoot;
		m_ExtensionNumberSpinner = SetupIntSpinner(dialogRoot, IDC_EXTENSION_NUMBER_SPIN, IDC_EXTENSION_NUMBER_EDIT, 0, 99, m_ExtensionNumber);
	}

	void W3DNameExtensionParamDlg::ReleaseControls()
	{
		ReleaseISpinner(m_ExtensionNumberSpinner);
	}

	INT_PTR W3DNameExtensionParamDlg::HandleCommand(uint16 controlID, uint16 commandID)
	{
		switch (controlID)
		{
		case IDOK:
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(m_DialogRoot, controlID);
			return TRUE;
		}

		return FALSE;
	}

	INT_PTR W3DNameExtensionParamDlg::HandleSpinner(uint16 controlID)
	{
		switch (controlID)
		{
		case IDC_EXTENSION_NUMBER_SPIN:
			m_ExtensionNumber = m_ExtensionNumberSpinner->GetIVal();
			return TRUE;
		}
		return FALSE;
	}
}