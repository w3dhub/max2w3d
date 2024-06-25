#pragma once

#include <max.h>
#include "w3dappdatachunk.h"

namespace W3D::MaxTools
{
	float GetScreenSizeFromNode(INode* node);

	class W3DUtilities;
	struct W3DExportFlagsStruct;

	class W3DExportSettingsDlg
	{
	public:
		W3DExportSettingsDlg(W3DUtilities& utilities);

		void Initialise(Interface* ip);
		void Close(Interface* ip);
		void RefreshUI();
	private:
		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

		void ReleaseControls();
		void ConnectControls(HWND dialogRoot);
		void GetW3DExportFlags(W3DExportFlagsStruct *str);
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);
		INT_PTR HandleSpinner(uint16 controlID);

		void SetDazzleType(const TSTR& dazzle);
		void SetGeometryType(W3DGeometryType type);
		void SetStaticSortLevel(int sortLevel);
		void SetScreenSize(float size);
		void ModifyExportFlags(W3DExportFlags flags, bool add);
		void ModifyGeometryFlags(W3DGeometryFlags flags, bool add);
		void ModifyCollisionFlags(W3DCollisionFlags flags, bool add);

		W3DUtilities&    m_Utilities;
		HWND             m_RollupRoot;
		HWND             m_DialogRoot;
		HWND             m_DazzleType;
		HWND             m_SelectionEdit;
		ISpinnerControl* m_StaticSortingSpinner;
		ISpinnerControl* m_ScreenSizeSpinner;
	};
}