#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_UTILITIES_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_UTILITIES_H

#include <vector>

#include <utilapi.h>
#include <iparamb2.h>

#include "Dialog/w3dexportsettingsdlg.h"
#include "Dialog/w3dmiscutilitiesdlg.h"

namespace W3D::MaxTools
{
	enum class W3DUtilityAppDataSubID
	{
		ExportSettings = 2,
		DazzleName,

		Num
	};

	class W3DUtilities
		: public UtilityObj
	{
	public:
		W3DUtilities();

		// Inherited via UtilityObj
		virtual void BeginEditParams(Interface * ip, IUtil * iu) override;
		virtual void EndEditParams(Interface * ip, IUtil * iu) override;
		virtual void SelectionSetChanged(Interface *ip, IUtil *iu) override;
		virtual void DeleteThis() override;
		void RefreshExportSettings() { m_ExportSettings.RefreshUI(); }

		const std::vector<INode*>& SelectedNodes() const { return m_SelectedNodes; }

		static W3DAppDataChunk& GetOrCreateW3DAppDataChunk(INode& node);
		static AppDataChunk& GetOrCreateChunk(INode& node, W3DUtilityAppDataSubID subID);
		static AppDataChunk& GetOrCreateExportSettingsChunk(INode& node);
		static void SetDazzleTypeInAppData(INode* node, const CStr& text);
		static const char* GetDazzleTypeFromAppData(INode* node);

	private:
		std::vector<INode*>  m_SelectedNodes;
		W3DExportSettingsDlg m_ExportSettings;
		W3DMiscUtilitiesDlg  m_MiscUtilities;
	};

	class W3DUtilitiesClassDesc
		: public ClassDesc2
	{
	public:
		virtual int           IsPublic() override { return TRUE; }
		virtual void*         Create(BOOL loading = FALSE) override { return new W3DUtilities(); }
		virtual const MCHAR*  ClassName() override { return _M("W3D Tools"); }
		virtual const MCHAR*  NonLocalizedClassName() override { return _M("W3D Tools"); }
		virtual SClass_ID     SuperClassID() override { return UTILITY_CLASS_ID; }
		virtual const MCHAR * Category() override { return _M("Westwood Tools"); }

		virtual Class_ID ClassID() override { return Class_ID(0x3C362C97, 0x5FC73AB0); }
		static ClassDesc2* Instance();

	private:
		W3DUtilitiesClassDesc();
		~W3DUtilitiesClassDesc();

		static void NotifyPreNodesCloned(void* param, NotifyInfo* info);
		static void NotifyPostNodesCloned(void* param, NotifyInfo* info);
		static void NotifyPreSave(void* param, NotifyInfo* info);
	};
}

#endif //W3D_MAX_TOOLS_INCLUDE_W3D_UTILITIES_H