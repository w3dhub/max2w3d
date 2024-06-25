#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_DLG_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_DLG_H

#include "w3dexport.h"

namespace W3D::MaxTools
{

	struct W3DExportSettings;

	class W3DExportDlg
	{
		using TabsArray = std::array<HWND, static_cast<size_t>(W3DExportType::Num)>;

	public:
		W3DExportDlg(W3DExportSettings& settings);
		INT_PTR ShowDialog();

	private:
		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK TabProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void ConnectControls(HWND root);
		void PopulateControls(HWND root);
		void SetTab(W3DExportType tab);
		void SelectExternalSkeleton(HWND buttonHandle);
		void RefreshExternalSkeletonButton();
		INT_PTR HandleCommand(uint16 controlID, uint16 commandID);
		INT_PTR HandleSpinner(uint16 controlID);

		W3DExportSettings & m_Settings;
		HWND                m_DialogRoot;
		HWND                m_SettingsPanel;
		HWND                m_ActiveTab;
		TabsArray           m_Tabs;
	};

#ifdef W3X
	class W3DCompressionDlg
	{
	public:
		W3DCompressionDlg(W3DExportSettings& settings, HWND parent);
		INT_PTR ShowDialog();

	private:
		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void ConnectControls(HWND root);
		void PopulateControls(HWND root);
		INT_PTR HandleCommand(HWND dialogRoot, uint16 controlID, uint16 commandID);
		void Initialise(HWND dialogRoot);
		void UpdateControls(HWND dialogRoot);
		void OnOk(HWND dialogRoot);

		W3DExportSettings& m_Settings;
		HWND               m_Parent;
		ICustEdit        * m_MaxTranslation;
		ICustEdit        * m_MaxRotation;
		ICustEdit        * m_MaxVisibility;
		ICustEdit        * m_MaxError;
	};
#endif

} // namespace W3D::MaxTools

#endif //W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_DLG_H