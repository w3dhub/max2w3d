#include "general.h"
#include <modstack.h>
#include <iskin.h>
#include "w3dskin.h"
#include "engine_string.h"
//copying definitions here because maxscript.h conflicts with some stuff in our code (StringClass in particular) and we just need the one defintion.
#define ScripterExport __declspec( dllimport )
ScripterExport BOOL ExecuteMAXScriptScript(const MCHAR* s, MAXScript::ScriptSource scriptSource, BOOL quietErrors = FALSE, FPValue* fpv = NULL, BOOL logQuietErrors = TRUE);
namespace W3D::MaxTools
{
	SkinWSMObjectClassDesc * SkinWSMObjectClassDesc::Instance()
	{
		static SkinWSMObjectClassDesc s_instance;
		return &s_instance;
	}

	SkinWSMObjectClass::SkinWSMObjectClass()
	{
		nodelist.SetCount(1);
	}

	void SkinWSMObjectClass::GetClassName(TSTR& s, bool localized)
	{
		s = GetObjectName(false);
	}

	Class_ID SkinWSMObjectClass::ClassID()
	{
		return SkinWSMObjectClassDesc::Instance()->ClassID();
	}

	SkinWSMObjectClass::~SkinWSMObjectClass()
	{
	}

	void SkinWSMObjectClass::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
	{
		SimpleWSMObject::BeginEditParams(ip, flags, prev);
		//not necessary, unimplemented
	}

	void SkinWSMObjectClass::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
	{
		SimpleWSMObject::EndEditParams(ip, flags, next);
		//not necessary, unimplemented
	}

	IOResult SkinWSMObjectClass::Save(ISave *isave)
	{
		ReferenceMaker::Save(isave);
		int count = nodelist.Count();
		if (count)
		{
			ULONG x;
			isave->BeginChunk(1);
			isave->Write(&count, 4, &x);
			isave->EndChunk();
		}
		return IO_OK;
	}

	class NodeEnumProc : public DependentEnumProc
	{
	public:
		virtual int proc(ReferenceMaker *rmaker)
		{
			if (rmaker->SuperClassID() == BASENODE_CLASS_ID)
			{
				node = (INode *)(rmaker);
				return DEP_ENUM_HALT;
			}
			return DEP_ENUM_CONTINUE;
		}
		INode* node;
	};
	
	class SkinWSMObjectPostLoad : public PostLoadCallback
	{
	public:
		SkinWSMObjectClass *skin;
		SkinWSMObjectPostLoad(SkinWSMObjectClass *s) : skin(s)
		{
		}
		int Priority() { return 10; }
		void proc(ILoad *iload)
		{
			NodeEnumProc dep;
			skin->DoEnumDependents(&dep);
			if (dep.node)
			{
				GetCOREInterface()->DeleteNode(dep.node);
			}
		}
	};

	IOResult SkinWSMObjectClass::Load(ILoad *iload)
	{
		ReferenceMaker::Load(iload);
		while (iload->OpenChunk() == IO_OK)
		{
			if (iload->CurChunkID() == 1)
			{
				int count;
				ULONG x;
				iload->Read(&count, 4, &x);
				nodelist.SetCount(count);
				for (int i = 0; i < count; i++)
				{
					nodelist[i] = nullptr;
				}
			}
			iload->CloseChunk();
		}
		iload->RegisterPostLoadCallback(new SkinWSMObjectPostLoad(this));
		return IO_OK;
	}

	int SkinWSMObjectClass::NumRefs()
	{
		return nodelist.Count() + 1;
	}

	RefTargetHandle SkinWSMObjectClass::GetReference(int i)
	{
		if (i > 0)
		{
			return nodelist[i - 1];
		}
		else
		{
			return pblock;
		}
	}

	void SkinWSMObjectClass::SetReference(int i, RefTargetHandle rtarg)
	{
		if (i > 0)
		{
			nodelist[i - 1] = (INode *)rtarg;
		}
		else
		{
			pblock = (IParamBlock *)rtarg;
		}
	}

	RefResult SkinWSMObjectClass::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
	{
		if (message != REFMSG_TARGET_DELETED)
		{
			return REF_SUCCEED;
		}
		int i;
		for (i = 0; i < nodelist.Count(); i++)
		{
			if (nodelist[i] == hTarget)
			{
				break;
			}
		}
		if (i < nodelist.Count())
		{
			nodelist.Delete(i, 1);
		}
		return REF_SUCCEED;
	}

	RefTargetHandle SkinWSMObjectClass::Clone(RemapDir &remap)
	{
		return new SkinWSMObjectClass();
	}

	CreateMouseCallBack* SkinWSMObjectClass::GetCreateMouseCallBack()
	{
		return nullptr;
		//not necessary, unimplemented
	}

	Modifier *SkinWSMObjectClass::CreateWSMMod(INode *node)
	{
		return new SkinModifierClass(node, this);
	}

	void SkinWSMObjectClass::BuildMesh(TimeValue t)
	{
		//not necessary, unimplemented
	}

	SkinModifierClassDesc * SkinModifierClassDesc::Instance()
	{
		static SkinModifierClassDesc s_instance;
		return &s_instance;
	}

	struct SkinDataBoneStr
	{
		int bone[2];
		int weight[2];
	};

	class SkinDataClass : public LocalModData
	{
	public:
		BOOL unk1;
		BOOL unk2;
		BitArray unk3;
		Tab<SkinDataBoneStr> bones;
		SkinDataClass() : unk1(FALSE), unk2(FALSE)
		{
		}
		IOResult Load(ILoad *iload)
		{
			while (iload->OpenChunk() == IO_OK)
			{
				int id = iload->CurChunkID();
				switch (id)
				{
				case 0:
				{
					short flags;
					ULONG x;
					iload->Read(&flags, 2, &x);
					unk1 = flags & 1;
					unk2 = flags & 2;
				}
				break;
				case 0x10:
				{
					unk3.Load(iload);
				}
				break;
				case 0x30:
				{
					int count = (int)iload->CurChunkLength() / 16;
					ULONG x;
					bones.SetCount(count);
					iload->Read((BYTE *)&bones[0], 16 * count, &x);
				}
				break;
				default:
					MSTR str;
					str.printf(L"Unknown Skin Data Chunk %d", id);
					OutputDebugString(str);
					TT_INTERRUPT;
					break;
				}
				iload->CloseChunk();
			}
			return IO_OK;
		}

		IOResult Save(ISave *isave)
		{
			short flags = 0;
			if (unk1)
			{
				flags = 1;
			}
			if (unk2)
			{
				flags |= 2;
			}
			ULONG x;
			isave->BeginChunk(0);
			isave->Write(&flags, 2, &x);
			isave->EndChunk();
			if (!unk3.IsEmpty())
			{
				isave->BeginChunk(0x10);
				unk3.Save(isave);
				isave->EndChunk();
			}
			if (bones.Count())
			{
				isave->BeginChunk(0x30);
				isave->Write((BYTE *)&bones[0], 16 * bones.Count(), &x);
				isave->EndChunk();
			}
			return IO_OK;
		}

		LocalModData *Clone()
		{
			SkinDataClass *data = new SkinDataClass();
			data->unk3 = unk3;
			data->bones = bones;
			return data;
		}
	};

	SkinModifierClass::SkinModifierClass() : SubObjSelLevel(1), WSMObject(nullptr), Node(nullptr)
	{
	}

	SkinModifierClass::SkinModifierClass(INode *node, SkinWSMObjectClass *wsmobject) : SubObjSelLevel(1), WSMObject(nullptr), Node(nullptr)
	{
		ReplaceReference(1, node);
		ReplaceReference(0, wsmobject);
	}

	void SkinModifierClass::GetClassName(TSTR& s, bool localized)
	{
		s = L"WWSkin";
		localized = false;
	}

	Class_ID SkinModifierClass::ClassID()
	{
		return SkinWSMObjectClassDesc::Instance()->ClassID();
	}

	SkinModifierClass::~SkinModifierClass()
	{
	}

	void SkinModifierClass::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
	{
		//not necessary, unimplemented
	}

	IOResult SkinModifierClass::Save(ISave *isave)
	{
		Modifier::Save(isave);
		isave->BeginChunk(43521);
		short s = (unsigned short)SubObjSelLevel;
		ULONG x;
		isave->Write(&s, 2, &x);
		isave->EndChunk();
		return IO_OK;
	}

	class GetModContextProc : public ModContextEnumProc {
	public:
		ModContext *m;
		GetModContextProc() { m = nullptr; }
		BOOL proc(ModContext *mc) {
			this->m = mc;
			return FALSE;
		}
	};

	ModContext *GetModContextFromMod(Modifier *m)
	{
		GetModContextProc mcp;
		m->EnumModContexts(&mcp);
		return mcp.m;
	}

	INode *GetNodeFromMod(Modifier *m)
	{
		ModContext *mc = GetModContextFromMod(m);
		IDerivedObject *obj;
		int i;
		m->GetIDerivedObject(mc, obj, i);
		return GetCOREInterface7()->FindNodeFromBaseObject(obj);
	}

	void ReplaceModifier(Modifier *oldmod, Modifier *newmod)
	{
		ModContext *mc = GetModContextFromMod(oldmod);
		IDerivedObject *d;
		int i;
		oldmod->GetIDerivedObject(mc, d, i);
		INode *n = GetNodeFromMod(oldmod);
		Object* obj = n->GetObjectRef();
		IDerivedObject *dobj;
		if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		{
			dobj = (IDerivedObject*)obj;
		}
		else
		{
			dobj = CreateDerivedObject(obj);
		}
		n->SetObjectRef(dobj);
		oldmod->DeleteAllRefsToMe();
		dobj->DeleteModifier(i);
		dobj->AddModifier(newmod,nullptr,i);
	}

	INode **GetBones(SkinModifierClass *skin, int index)
	{
		ModContext *context = GetModContextFromMod(skin);
		if (context)
		{
			SkinDataClass *data = (SkinDataClass *)context->localData;
			int count = data->bones.Count();
			INode **bones = new INode *[count];
			for (int i = 0; i < count; i++)
			{
				if (index == 0 || index == 1)
				{
					int bone = data->bones[i].bone[index];
					if (bone != -1)
					{
						bones[i] = skin->WSMObject->nodelist[bone];
					}
					else
					{
						bones[i] = nullptr;
					}
				}
			}
			return bones;
		}
		return nullptr;
	}

	float *GetBoneWeights(SkinModifierClass *skin, int index)
	{
		ModContext *context = GetModContextFromMod(skin);
		if (context)
		{
			SkinDataClass *data = (SkinDataClass *)context->localData;
			int count = data->bones.Count();
			float *weights = new float[count];
			for (int i = 0; i < count; i++)
			{
				if (index == 0 || index == 1)
				{
					weights[i] = ((float)data->bones[i].weight[index]) / 100;
				}
			}
			return weights;
		}
		return nullptr;
	}

	int GetBoneCount(SkinModifierClass *skin)
	{
		ModContext *context = GetModContextFromMod(skin);
		if (context)
		{
			SkinDataClass *data = (SkinDataClass *)context->localData;
			return data->bones.Count();
		}
		return -1;
	}

	int GetBoneListCount(SkinModifierClass *skin)
	{
		SkinWSMObjectClass *wsm = skin->WSMObject;
		if (wsm)
		{
			int c = 0;
			int count = wsm->nodelist.Count();
			for (int i = 0; i < count; i++)
			{
				if (wsm->nodelist[i])
				{
					c++;
				}
			}
			return c;
		}
		return -1;
	}

	INode **GetBoneList(SkinModifierClass *skin)
	{
		SkinWSMObjectClass *wsm = skin->WSMObject;
		if (wsm)
		{
			int count = GetBoneListCount(skin);
			INode **nodes = new INode *[count];
			int c = 0;
			for (int i = 0; i < wsm->nodelist.Count(); i++)
			{
				if (wsm->nodelist[i])
				{
					nodes[c] = wsm->nodelist[i];
					c++;
				}
			}
			return nodes;
		}
		return nullptr;
	}

	class SkinModifierPostLoad : public PostLoadCallback
	{
	public:
		SkinModifierClass *skin;
		SkinModifierPostLoad(SkinModifierClass *s) : skin(s)
		{
		}
		int Priority() { return 9; }
		void proc(ILoad *iload)
		{
			auto bone1 = GetBones(skin, 0);
			auto bone2 = GetBones(skin, 1);
			auto bone1weights = GetBoneWeights(skin, 0);
			auto bone2weights = GetBoneWeights(skin, 1);
			auto bonelist = GetBoneList(skin);
			if (bone1 && bone2 && bone1weights && bone2weights && bonelist)
			{
				INode *node = GetNodeFromMod(skin);
				Modifier *skinMod = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
				int bonecount = GetBoneCount(skin);
				int bonelistcount = GetBoneListCount(skin);
				ReplaceModifier(skin, skinMod);
				ISkinImportData *sd = (ISkinImportData*)skinMod->GetInterface(I_SKINIMPORTDATA);
				for (int i = 0; i < bonelistcount; i++)
				{
					if (bonelist[i])
					{
						sd->AddBoneEx(bonelist[i], TRUE);
					}
				}				
				WideStringClass str;
				str.Format(L"select $%s", node->GetName());
				ExecuteMAXScriptScript(str, MAXScript::ScriptSource::NonEmbedded);
				str.Format(L"max modify mode");
				ExecuteMAXScriptScript(str, MAXScript::ScriptSource::NonEmbedded);
				str.Format(L"modPanel.setCurrentObject $.modifiers[#skin]");
				ExecuteMAXScriptScript(str, MAXScript::ScriptSource::NonEmbedded);
				for (int i = 0; i < bonecount; i++)
				{
					str.Format(L"skinOps.unNormalizeVertex $.modifiers[#skin] %d true", i + 1);
					ExecuteMAXScriptScript(str, MAXScript::ScriptSource::NonEmbedded);
					Tab<INode*> nodetab;
					Tab<float> weighttab;
					if (bone1[i])
					{
						nodetab.Append(1, &bone1[i]);
						weighttab.Append(1, &bone1weights[i]);
					}
					else
					{
						nodetab.Append(1, &bonelist[0]);
						float weight = 0;
						weighttab.Append(1, &weight);
					}
					if (bone2[i])
					{
						nodetab.Append(1, &bone2[i]);
						weighttab.Append(1, &bone2weights[i]);
					}
					if (weighttab.Count() > 0)
					{
						sd->AddWeights(node, i, nodetab, weighttab);
					}
				}
				str.Format(L"deselect $%s", node->GetName());
				ExecuteMAXScriptScript(str, MAXScript::ScriptSource::NonEmbedded);
				delete[] bone1;
				delete[] bone2;
				delete[] bone1weights;
				delete[] bone2weights;
				delete[] bonelist;
			}
		}
	};

	IOResult SkinModifierClass::Load(ILoad *iload)
	{
		Modifier::Load(iload);
		while (iload->OpenChunk() == IO_OK)
		{
			if (iload->CurChunkID() == 43521)
			{
				short s;
				ULONG x;
				iload->Read(&s, 2, &x);
				SubObjSelLevel = s;
			}
			iload->CloseChunk();
		}
		iload->RegisterPostLoadCallback(new SkinModifierPostLoad(this));
		return IO_OK;
	}

	int SkinModifierClass::NumRefs()
	{
		return 2;
	}

	RefTargetHandle SkinModifierClass::GetReference(int i)
	{
		if (i == 0)
		{
			return WSMObject;
		}
		else if (i == 1)
		{
			return Node;
		}
		return nullptr;
	}

	void SkinModifierClass::SetReference(int i, RefTargetHandle rtarg)
	{
		if (i == 0)
		{
			WSMObject = (SkinWSMObjectClass *)rtarg;
		}
		else if (i == 1)
		{
			Node = (INode *)rtarg;
		}
	}

	RefResult SkinModifierClass::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
	{
		if (message != REFMSG_TARGET_DELETED)
		{
			return REF_SUCCEED;
		}
		DeleteMe();
		return REF_STOP;
	}

	RefTargetHandle SkinModifierClass::Clone(RemapDir &remap)
	{
		return new SkinModifierClass(Node, WSMObject);
	}

	int SkinModifierClass::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{
		return 0;
		//not necessary, unimplemented
	}

	void SkinModifierClass::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::ClearSelection(int selLevel)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::SelectAll(int selLevel)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::InvertSelection(int selLevel)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::ActivateSubobjSel(int level, XFormModes& modes)
	{
		//not necessary, unimplemented
	}

	BOOL SkinModifierClass::SupportsNamedSubSels()
	{
		return TRUE;
	}

	void SkinModifierClass::ActivateSubSelSet(MSTR &setName)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::NewSetFromCurSel(MSTR &setName)
	{
		//not necessary, unimplemented
	}

	void SkinModifierClass::RemoveSubSelSet(MSTR &setName)
	{
		//not necessary, unimplemented
	}

	int SkinModifierClass::NumSubObjTypes()
	{
		return 1;
	}

	static GenSubObjType subobj(1);

	ISubObjType *SkinModifierClass::GetSubObjType(int i)
	{
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			subobj.SetName(L"Vertices");
		}

		if (i != -1 || GetSubObjectLevel() <= 0)
		{
			return &subobj;
		}

		return GetSubObjType(GetSubObjectLevel() - 1);
	}

	ChannelMask SkinModifierClass::ChannelsUsed()
	{
		return SUBSEL_TYPE_CHANNEL | SELECT_CHANNEL | GEOM_CHANNEL;
	}

	ChannelMask SkinModifierClass::ChannelsChanged()
	{
		return SUBSEL_TYPE_CHANNEL | SELECT_CHANNEL | GEOM_CHANNEL;
	}

	void SkinModifierClass::NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc)
	{
	}

	void SkinModifierClass::ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node)
	{
		//not necessary, unimplemented
	}

	BOOL SkinModifierClass::DependOnTopology(ModContext &mc)
	{
		return TRUE;
	}

	Class_ID SkinModifierClass::InputType()
	{
		return Class_ID(TRIOBJ_CLASS_ID, 0);
	}

	IOResult SkinModifierClass::SaveLocalData(ISave *isave, LocalModData *ld)
	{
		return ((SkinDataClass *)ld)->Save(isave);
	}

	IOResult SkinModifierClass::LoadLocalData(ILoad *iload, LocalModData **pld)
	{
		if (!*pld)
		{
			*pld = new SkinDataClass();
		}
		return ((SkinDataClass *)*pld)->Load(iload);
	}
};
