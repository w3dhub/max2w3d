#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_MISC_UTILITIES_DLG_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_MISC_UTILITIES_DLG_H

#include <max.h>

namespace W3D::MaxTools
{
	class W3DUtilities;
	enum class W3DGeometryType : uint32;
	enum class W3DExportFlags : uint32;
	enum class W3DCollisionFlags : uint32;

	class W3DMiscUtilitiesDlg
	{
	public:
		W3DMiscUtilitiesDlg(W3DUtilities& utilities);

		void Initialise(Interface* ip);
		void Close(Interface* ip);

	private:
		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
		
		void ConnectControls(HWND dialogRoot);
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);

		void SetSelectionOnGeometryType(W3DGeometryType type);
		void SetSelectionOnExportFlags(W3DExportFlags flags);
		void SetSelectionOnCollisionFlags(W3DCollisionFlags flags);
		void SelectAlphaObjects();
		void DoNodeNameAssignment();
		void DoMaterialNameAssignment();
		void DoExtensionNameAssignment();

		W3DUtilities&     m_Utilities;
		HWND              m_RollupRoot;
		HWND              m_DialogRoot;
	};

	struct W3DNodeNameParamDlg
	{
		W3DNodeNameParamDlg();

		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

		bool m_AssignNames;
		bool m_AssignPrefix;
		bool m_AssignSuffix;
		bool m_AssignCollisionFlags;
		bool m_AffectAll;
		W3DCollisionFlags m_CollisionFlags;
		int  m_StartingIndex;
		TSTR m_RootName;
		TSTR m_Prefix;
		TSTR m_Suffix;

	private:
		void Initialise(HWND dialogRoot);
		void ReleaseControls();
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);
		INT_PTR HandleSpinner(uint16 controlID);

		HWND m_DialogRoot;
		ICustEdit*       m_RootNameEdit;
		ICustEdit*       m_PrefixEdit;
		ICustEdit*       m_SuffixEdit;
		ISpinnerControl* m_StartingIndexSpinner;
	};

	struct W3DMaterialNameParamDlg
	{
		W3DMaterialNameParamDlg();

		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		bool m_AffectAll;
		int  m_StartingIndex;
		TSTR m_RootName;

	private:
		void Initialise(HWND dialogRoot);
		void ReleaseControls();
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);
		INT_PTR HandleSpinner(uint16 controlID);

		HWND             m_DialogRoot;
		ICustEdit*       m_RootNameEdit;
		ISpinnerControl* m_StartingIndexSpinner;
	};

	struct W3DNameExtensionParamDlg
	{
		W3DNameExtensionParamDlg();
		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		int m_ExtensionNumber;

	private:
		void Initialise(HWND dialogRoot);
		void ReleaseControls();
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);
		INT_PTR HandleSpinner(uint16 controlID);

		HWND m_DialogRoot;
		ISpinnerControl* m_ExtensionNumberSpinner;
	};

} //W3D::MaxTools

#endif //W3D_MAX_TOOLS_INCLUDE_W3D_MISC_UTILITIES_DLG_H
