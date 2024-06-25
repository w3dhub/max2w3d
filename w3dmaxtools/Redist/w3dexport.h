#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_H

#include <max.h>
#include <impexp.h>
#include <iparamb2.h>
#include <vector>
#include "engine_string.h"

class ChunkSaveClass;
class XMLWriter;

namespace W3D::MaxTools
{
	class MeshConnection;
	class INodeListClass;
	class HierarchySave;

	enum class W3DExportType : uint8
	{
		HierarchicalModel = 0,
		HierarchicalAnimatedModel,
		PureAnimation,
		Skeleton,
		Terrain,
		SimpleMesh,

		Num
	};


	struct OldW3DExportSettings
	{
		W3DExportType ExportType = W3DExportType::HierarchicalModel;
		bool ExportSkeleton = true;
		bool UseExistingSkeleton = false;
		bool ExportAnimation = true;
		bool ExportGeometry = true;
		MCHAR ExistingSkeletonFileDirectory[MAX_PATH] = {};
		MCHAR ExistingSkeletonFileName[MAX_PATH] = {};
		int AnimFramesStart = 0u;
		int AnimFramesEnd = 100u;
		bool SmoothVertexNormals = true;
		bool ReviewLog = false;
		bool OptimiseCollisions = true;
		bool ExportAsTerrain = false;
	};

	struct W3DExportSettings
	{
		W3DExportType ExportType = W3DExportType::HierarchicalModel;
		bool ExportSkeleton = true;
		bool UseExistingSkeleton = false;
		bool ExportAnimation = true;
		bool ExportGeometry = true;
		MCHAR ExistingSkeletonFileDirectory[MAX_PATH] = {};
		MCHAR ExistingSkeletonFileName[MAX_PATH] = {};
		int AnimFramesStart = 0u;
		int AnimFramesEnd = 100u;
		bool SmoothVertexNormals = true;
		bool ReviewLog = false;
		bool OptimiseCollisions = true;
		bool ExportAsTerrain = false;
#ifndef W3X
		bool MeshDeduplication = false;
		bool NewAABTree = false;
#else
		bool NonDefaultCompressionSettings = false;
		int CompressionTypes = 3;
		float MaxTranslationError = 0.002f;
		float MaxRotationError = 0.003f;
		float MaxVisibilityError = 0.01f;
		bool ForceKeyReduction = false;
		int KeyReduction = 50;
		float MaxAdaptiveDeltaError = 0.001f;
#endif
	};

	class W3DExport
		: public SceneExport
	{
	public:
		W3DExport();

	private:

		// Inherited via SceneExport
		virtual int   ExtCount() override;
		virtual const MCHAR * Ext(int n) override;
		virtual const MCHAR * LongDesc() override;
		virtual const MCHAR * ShortDesc() override;
		virtual const MCHAR * AuthorName() override;
		virtual const MCHAR * CopyrightMessage() override;
		virtual const MCHAR * OtherMessage1() override;
		virtual const MCHAR * OtherMessage2() override;
		virtual unsigned int Version() override;
		virtual void ShowAbout(HWND hWnd) override;
		virtual int DoExport(const MCHAR * name, ExpInterface * ei, Interface * i, BOOL suppressPrompts = FALSE, DWORD options = 0) override;
		INodeListClass* CreateOriginNodeList();
#ifndef W3X
		void ExportData(char *name, ChunkSaveClass &csave);
		bool ExportHierarchy(const char *name, ChunkSaveClass &csave, INode *node);
		bool ExportAnimation(const char *name, ChunkSaveClass &csave, INode *node);
		bool ExportGeometry(const char *name, ChunkSaveClass &csave, INode *node, MeshConnection **connection);
		bool ExportHlod(const char *name, const char *hierarchyname, ChunkSaveClass &csave, MeshConnection **connections, int nodecount);
#else
		void ExportData(char* name, XMLWriter& csave);
		bool ExportHierarchy(const char* name, XMLWriter& csave, INode* node);
		bool ExportAnimation(const char* name, XMLWriter& csave, INode* node);
		bool ExportGeometry(const char* name, XMLWriter& csave, INode* node, MeshConnection** connection);
		bool ExportHlod(const char* name, const char* hierarchyname, XMLWriter& csave, MeshConnection** connections, int nodecount);
#endif
		HierarchySave *GetHierarchy();
		HierarchySave *ImportHierarchy(const char *filename);

		W3DExportSettings m_Settings;
		char Path[MAX_PATH];
		ExpInterface *ExpInt;
		Interface *Int;
		TimeValue Time;
		int FrameRate;
		INodeListClass *OriginNodeList;
		HierarchySave *HierarchyStruct;
		char SkeletonPath[MAX_PATH];
#ifdef W3X
		std::vector<StringClass> includes;
#endif
	};

	class W3DExportClassDesc
		: public ClassDesc2
	{
	public:
		virtual int           IsPublic() override { return TRUE; }
		virtual void*         Create(BOOL loading = FALSE) override { return new W3DExport(); }
		virtual const MCHAR*  ClassName() override { return _M("W3DExport"); }
		virtual const MCHAR*  NonLocalizedClassName() override { return _M("W3DExport"); }
		virtual SClass_ID     SuperClassID() override { return SCENE_EXPORT_CLASS_ID; }
		virtual const MCHAR * Category() override { return _M("W3D Tools"); }

		virtual Class_ID ClassID() override { return Class_ID(0x19503c91, 0x5a0d2d0d); }
		static ClassDesc2* Instance();
	};
}

#endif //W3D_MAX_TOOLS_INCLUDE_W3D_EXPORT_H