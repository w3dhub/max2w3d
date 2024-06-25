#include "general.h"
#include <maxheapdirect.h>
#include <iInstanceMgr.h>
#include <notify.h>
#include "w3dutilities.h"

namespace
{
	struct NodeCloneInfo //See notify.h in the max SDK. NOTIFY_POST_NODES_CLONED
	{
		INodeTab* OriginalNodes = nullptr;
		INodeTab* NewNodes = nullptr;
		CloneType CloneType;
	};
}

namespace W3D::MaxTools
{
	W3DUtilities::W3DUtilities()
		: m_SelectedNodes()
		, m_ExportSettings(*this)
		, m_MiscUtilities(*this)
	{ }

	void W3DUtilities::BeginEditParams(Interface * ip, IUtil * iu)
	{
		m_ExportSettings.Initialise(ip);
		m_MiscUtilities.Initialise(ip);
		SelectionSetChanged(ip, iu);
	}

	void W3DUtilities::EndEditParams(Interface * ip, IUtil * iu)
	{
		m_ExportSettings.Close(ip);
		m_MiscUtilities.Close(ip);
	}

	void W3DUtilities::SelectionSetChanged(Interface *ip, IUtil *iu)
	{
		m_SelectedNodes.clear();
		m_SelectedNodes.reserve(ip->GetSelNodeCount());

		for (int i = 0; i < ip->GetSelNodeCount(); ++i)
			m_SelectedNodes.push_back(ip->GetSelNode(i));

		m_ExportSettings.RefreshUI();
	}

	void W3DUtilities::DeleteThis()
	{
		delete this;
	}

	W3DAppDataChunk&  W3DUtilities::GetOrCreateW3DAppDataChunk(INode& node)
	{
		return *static_cast<W3DAppDataChunk*>(GetOrCreateExportSettingsChunk(node).data);
	}

	void W3DUtilities::SetDazzleTypeInAppData(INode* node, const CStr& text)
	{
		using namespace W3D::MaxTools;

		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			AppDataChunk& chunk = W3DUtilities::GetOrCreateChunk(*nodes[i], W3DUtilityAppDataSubID::DazzleName);

			if (chunk.data)
			{
				MAX_free(chunk.data);
			}

			void* alloc = MAX_malloc(sizeof(DazzleAppData));
			DazzleAppData* d = new(alloc) DazzleAppData;
			chunk.length = sizeof(DazzleAppData);
			chunk.data = d;
			memcpy_s(d->name, text.ByteCount(), text.data(), text.ByteCount());
		}
	}

	const char *W3DUtilities::GetDazzleTypeFromAppData(INode* node)
	{
		using namespace W3D::MaxTools;

		AppDataChunk& chunk = W3DUtilities::GetOrCreateChunk(*node, W3DUtilityAppDataSubID::DazzleName);
		DazzleAppData* d = (DazzleAppData*)chunk.data;
		if (d)
		{
			return d->name;
		}
		else
		{
			return nullptr;
		}
	}

	AppDataChunk & W3DUtilities::GetOrCreateChunk(INode& node, W3D::MaxTools::W3DUtilityAppDataSubID subID)
	{
		const int chunkSID = enum_to_value(subID);

		AppDataChunk* chunk = node.GetAppDataChunk(W3DUtilitiesClassDesc::Instance()->ClassID(), W3DUtilitiesClassDesc::Instance()->SuperClassID(), chunkSID);

		if (!chunk)
		{
			node.AddAppDataChunk(W3DUtilitiesClassDesc::Instance()->ClassID(), W3DUtilitiesClassDesc::Instance()->SuperClassID(), chunkSID, 0, nullptr);
			chunk = node.GetAppDataChunk(W3DUtilitiesClassDesc::Instance()->ClassID(), W3DUtilitiesClassDesc::Instance()->SuperClassID(), chunkSID);
		}

		return *chunk;
	}

	AppDataChunk & W3DUtilities::GetOrCreateExportSettingsChunk(INode& node)
	{
		AppDataChunk& chunk = GetOrCreateChunk(node, W3DUtilityAppDataSubID::ExportSettings);
		if (!chunk.data)
		{
			chunk.length = sizeof(W3DAppDataChunk);
			void* alloc = MAX_malloc(chunk.length);
			chunk.data = new(alloc) W3DAppDataChunk;
		}

		return chunk;
	}

	ClassDesc2 * W3DUtilitiesClassDesc::Instance()
	{
		static W3DUtilitiesClassDesc s_instance;
		return &s_instance;
	}

	W3DUtilitiesClassDesc::W3DUtilitiesClassDesc()
	{
		RegisterNotification(NotifyPreNodesCloned, this, NOTIFY_PRE_NODES_CLONED);
		RegisterNotification(NotifyPostNodesCloned, this, NOTIFY_POST_NODES_CLONED);
		RegisterNotification(NotifyPreSave, this, NOTIFY_FILE_PRE_SAVE);
	}

	W3DUtilitiesClassDesc::~W3DUtilitiesClassDesc()
	{
		UnRegisterNotification(NotifyPreNodesCloned, this, NOTIFY_PRE_NODES_CLONED);
		UnRegisterNotification(NotifyPostNodesCloned, this, NOTIFY_POST_NODES_CLONED);
		UnRegisterNotification(NotifyPreSave, this, NOTIFY_FILE_PRE_SAVE);
	}

	void W3DUtilitiesClassDesc::NotifyPreSave(void* param, NotifyInfo* info)
	{
		PROPSPEC ps;
		WStr str(L"license");
		ps.lpwstr = (LPOLESTR)str.data();
		if (GetCOREInterface()->FindProperty(4, &ps) != -1)
		{
			GetCOREInterface()->DeleteProperty(4, &ps);
		}
	}

	void W3DUtilitiesClassDesc::NotifyPreNodesCloned(void* param, NotifyInfo* info)
	{
		INodeTab* OriginalNodes = (INodeTab*)info->callParam;
		for (int i = 0; i < OriginalNodes->Count(); i++)
		{
			// Save the name and pointer of the original object in user text before cloning.
			// The cloned objects will have these user props in the post-clone callback as well.
			// We need this because (at least in Max 2017) the cloned objects are either in alphabetical or selection order
			//  depending on the clone method you use (Shift+Drag vs Ctrl+V), while the originals are always in selection order.
			// Name can be the same between nodes, so I save the pointer as well.
			INode* node = (*OriginalNodes)[i];
			MSTR str;
			str.printf(_M("%s %p"), node->GetName(), node);
			node->SetUserPropString(_M("ClonedFromObject"), str);
		}
	}
	void W3DUtilitiesClassDesc::NotifyPostNodesCloned(void * param, NotifyInfo * info)
	{
		NodeCloneInfo* cloneInfo = static_cast<NodeCloneInfo*>(info->callParam);

#ifdef DEBUG_CLONING
		auto printName = [](INode& node) {
			static wchar_t buf[256];
			wsprintf(buf, L"%s\n", node.GetName());
			OutputDebugStringW(buf);
		};

		OutputDebugStringA("\nOriginal order:\n");
		for (int i = 0; i < cloneInfo->OriginalNodes->Count(); ++i) {
			INode& orig = *(*cloneInfo->OriginalNodes)[i];
			printName(orig);
		}
		OutputDebugStringA("\nClone order:\n");
		for (int i = 0; i < cloneInfo->NewNodes->Count(); ++i) {
			INode& clone = *(*cloneInfo->NewNodes)[i];
			printName(clone);
		}
		OutputDebugStringA("\nNew Clone order:\n");
#endif
		// NOTE: In Max 2017, if you use Shift+Drag to copy, the originals and clone node arrays are *not* in the same order.
		// We match them by setting a unique ID on the originals in NotifyPreNodesCloned, which is then copied to the clones automatically so we can query it here.
		
		std::vector<INode*> clones_sorted(cloneInfo->OriginalNodes->Count(), nullptr);

		// match clones with their originals by id
		bool failed_to_match_clones = false;
		for (int orig_idx = 0; orig_idx < cloneInfo->OriginalNodes->Count(); orig_idx++)
		{
			INode* orig = (*cloneInfo->OriginalNodes)[orig_idx];
			MSTR orig_id;
			orig->GetUserPropString(_M("ClonedFromObject"), orig_id);

			for (int clone_idx = 0; clone_idx < cloneInfo->NewNodes->Count(); ++clone_idx)
			{
				INode* clone = (*cloneInfo->NewNodes)[clone_idx];
				MSTR clone_id;
				clone->GetUserPropString(_M("ClonedFromObject"), clone_id);
				if (orig_id == clone_id) {
					clones_sorted[orig_idx] = clone;
					break;
				}
			}

			if (!clones_sorted[orig_idx]) {
				// this should never happen!
				failed_to_match_clones = true;
				clones_sorted[orig_idx] = orig; // just put something in here so it doesn't completely break
			}
#ifdef DEBUG_CLONING
			printName(*clones_sorted[orig_idx]);
#endif
		}

		if (failed_to_match_clones)
		{
			MessageBox(nullptr,
				L"Warning: Could not match clones to originals. Please double-check all W3D export properties (collision flags, geometry flags, sort level) and dazzle name (if applicable) of all cloned objects.\n\n"
				L"Please report this to the W3D export plugin developers (with reproduction steps if possible).", L"Warning", MB_OK | MB_ICONEXCLAMATION);
		}

		for (int i = 0; i < cloneInfo->NewNodes->Count(); ++i)
		{
			INode& orig = *(*cloneInfo->OriginalNodes)[i];
			INode& clone = *clones_sorted[i];

			for (int subIdx = 0; subIdx < enum_to_value(W3DUtilityAppDataSubID::Num); ++subIdx)
			{
				AppDataChunk* origChunk = orig.GetAppDataChunk(Instance()->ClassID(), Instance()->SuperClassID(), subIdx);
				if (origChunk && origChunk->data)
				{
					clone.AddAppDataChunk(Instance()->ClassID(), Instance()->SuperClassID(), subIdx, origChunk->length, MAX_malloc(origChunk->length));
					AppDataChunk* newChunk = clone.GetAppDataChunk(Instance()->ClassID(), Instance()->SuperClassID(), subIdx);
					memcpy(newChunk->data, origChunk->data, newChunk->length);
				}
			}
		}
	}
}