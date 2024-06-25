#pragma once
#include <simpobj.h>
class IUtil;

namespace W3D::MaxTools
{
	class SkinWSMObjectClass : public SimpleWSMObject
	{
	public:
		SkinWSMObjectClass();
		virtual void GetClassName(TSTR& s, bool localized);
		virtual Class_ID ClassID();
		virtual SClass_ID SuperClassID() { return WSM_OBJECT_CLASS_ID; }
		virtual ~SkinWSMObjectClass();
		virtual void DeleteThis() { delete this; }
		virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev = nullptr);
		virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next = nullptr);
		virtual IOResult Save(ISave *isave);
		virtual IOResult Load(ILoad *iload);
		virtual int NumRefs();
		virtual RefTargetHandle GetReference(int i);
		virtual void SetReference(int i, RefTargetHandle rtarg);
		virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
		virtual RefTargetHandle Clone(RemapDir &remap);
		virtual CreateMouseCallBack* GetCreateMouseCallBack();
		virtual const MCHAR *GetObjectName(bool localized) { return _M("WWSkin"); }
		virtual Modifier *CreateWSMMod(INode *node);
		virtual void BuildMesh(TimeValue t);
		Tab<INode *> nodelist;
	};

	class SkinWSMObjectClassDesc : public ClassDesc2
	{
	public:
		virtual int IsPublic() { return FALSE; }
		virtual void* Create(BOOL loading = FALSE) { return new SkinWSMObjectClass(); }
		virtual const TCHAR *	ClassName() { return _T("WWSkin"); }
		virtual const TCHAR *	NonLocalizedClassName() { return _T("WWSkin"); }
		virtual SClass_ID SuperClassID() { return WSM_OBJECT_CLASS_ID; }
		virtual Class_ID ClassID() { return Class_ID(0x32B37E0C, 0x5A9612E4); }
		virtual const TCHAR* Category() { return _T("Westwood Space Warps"); }
		static SkinWSMObjectClassDesc* Instance();

	private:
		SkinWSMObjectClassDesc() {}
	};

	class SkinModifierClass : public Modifier
	{
	public:
		SkinModifierClass();
		SkinModifierClass(INode *node, SkinWSMObjectClass *wsmobject);
		virtual void GetClassName(TSTR& s, bool localized);
		virtual Class_ID ClassID();
		virtual SClass_ID SuperClassID() { return WSM_OBJECT_CLASS_ID; }
		virtual ~SkinModifierClass();
		virtual void DeleteThis() { delete this; }
		virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev = nullptr);
		virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next = nullptr);
		virtual IOResult Save(ISave *isave);
		virtual IOResult Load(ILoad *iload);
		virtual int NumRefs();
		virtual RefTargetHandle GetReference(int i);
		virtual void SetReference(int i, RefTargetHandle rtarg);
		virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
		virtual RefTargetHandle Clone(RemapDir &remap);
		virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		virtual CreateMouseCallBack* GetCreateMouseCallBack() { return nullptr; };
		virtual const MCHAR *GetObjectName(bool localized) { return _M("WWSkin Binding"); }
		virtual void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert = FALSE);
		virtual void ClearSelection(int selLevel);
		virtual void SelectAll(int selLevel);
		virtual void InvertSelection(int selLevel);
		virtual void ActivateSubobjSel(int level, XFormModes& modes);
		virtual BOOL SupportsNamedSubSels();
		virtual void ActivateSubSelSet(MSTR &setName);
		virtual void NewSetFromCurSel(MSTR &setName);
		virtual void RemoveSubSelSet(MSTR &setName);
		virtual int NumSubObjTypes();
		virtual ISubObjType *GetSubObjType(int i);
		virtual ChannelMask ChannelsUsed();
		virtual ChannelMask ChannelsChanged();
		virtual void NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc);
		virtual void ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node);
		virtual BOOL DependOnTopology(ModContext &mc);
		virtual Class_ID InputType();
		virtual IOResult SaveLocalData(ISave *isave, LocalModData *ld);
		virtual IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
		int SubObjSelLevel;
		SkinWSMObjectClass *WSMObject;
		INode *Node;
	};

	class SkinModifierClassDesc : public ClassDesc2
	{
	public:
		virtual int IsPublic() { return FALSE; }
		virtual void* Create(BOOL loading = FALSE) { return new SkinModifierClass(); }
		virtual const TCHAR *	ClassName() { return _T("WWSkin"); }
		virtual const TCHAR *	NonLocalizedClassName() { return _T("WWSkin"); }
		virtual SClass_ID SuperClassID() { return WSM_CLASS_ID; }
		virtual Class_ID ClassID() { return Class_ID(0x6BAD4898, 0xD1D6CED); }
		virtual const TCHAR* Category() { return _T("Westwood Space Warps"); }
		static SkinModifierClassDesc* Instance();

	private:
		SkinModifierClassDesc() {}
	};
};
