#include "general.h"
#include <atomic>
#include <iskin.h>
#include <stdmat.h>
#include <meshnormalspec.h>
#include <maxheapdirect.h>
#include "w3dexport.h"
#include "EulerAngles.h"
#include "Dialog/w3dexportdlg.h"
#include "BufferedFileClass.h"
#include "w3d.h"
#include "matrix3d.h"
#ifndef W3X
#include "chunkclass.h"
#include "w3dutilities.h"
#else
#include "w3xutilities.h"
#include "ramfileclass.h"
#include "xmlwriter.h"
#include "pugixml.hpp"
#endif
#include <modstack.h>
#include "w3dmaterial.h"
#include "crc32.h"
#include "aabtreebuilderclass.h"
#include "resource.h"
#include "engine_string.h"
#include "vector.h"
#include <unordered_map>
#include <IDxMaterial.h>
#include <d3dx9.h>
#include <id3d9graphicswindow.h>
#include <pbbitmap.h>
#include <vector>
#include <deque>
#include <unordered_set>
#include "CriticalSectionClass.h"

#ifdef W3X
extern unsigned long crc_table[256];
#endif

namespace
{
	const std::array<const MCHAR*, 1>& valid_w3d_export_extensions()
	{
		static const std::array<const MCHAR*, 1> s_extensions
		{
#ifndef W3X
			_M("w3d")
#else
			_M("w3x")
#endif
		};

		return s_extensions;
	}
}

namespace W3D::MaxTools
{
	// forward declaration
	void ClearNameMap();

	class ErrorClass
	{
		const wchar_t* error;

	public:
		ErrorClass(_Printf_format_string_ const wchar_t* format, ...) : error(nullptr)
		{
			va_list va;
			wchar_t buf[1024];
			va_start(va, format);
			vswprintf(buf, 1024, format, va);
			error = newwcs(buf);
		}

		~ErrorClass()
		{
			if (error)
			{
				delete[] error;
			}
		}

		ErrorClass(ErrorClass const& that)
		{
			error = newwcs(that.error);
		}

		const wchar_t* GetError() const { return error; }
	};

	class INodeListFilter
	{
	public:
		virtual BOOL filter(INode* node, TimeValue time) = 0;
	};

	class AnyINodeFilter final : public INodeListFilter
	{
		virtual BOOL filter(INode* node, TimeValue time)
		{
			return TRUE;
		}
	};

	AnyINodeFilter anyfilter;

	class INodeListClass : public ITreeEnumProc
	{
		std::deque<INode*> list;
		INodeListFilter* FilterProc;
		TimeValue Time;
		int NodeCount;

	public:
		INodeListClass(IScene* scene, TimeValue time, INodeListFilter* filter) : NodeCount(0), Time(time), FilterProc(filter)
		{
			if (!filter)
			{
				FilterProc = &anyfilter;
			}

			scene->EnumTree(this);
		}

		INodeListClass(INode* node, TimeValue time, INodeListFilter* filter) : NodeCount(0), Time(time), FilterProc(filter)
		{
			if (!filter)
			{
				FilterProc = &anyfilter;
			}

			AddNodeRecursive(node);
		}

		INode* GetNode(int i)
		{
			return list[(size_t)i];
		}

		void AddNode(INode* node)
		{
			if (FilterProc->filter(node, Time))
			{
				// NOTE(Mara): Original code pushes to the front, so we do it too, though I haven't been able to see any difference in the output
				list.push_front(node);
				NodeCount++;
			}
		}

		void AddNodeRecursive(INode* node)
		{
			if (node)
			{
				AddNode(node);

				for (int i = 0; i < node->NumberOfChildren(); i++)
				{
					AddNodeRecursive(node->GetChildNode(i));
				}
			}
		}

		int callback(INode* node) override
		{
			AddNode(node);
			return TREE_CONTINUE;
		}

		auto begin() { return list.begin(); }
		auto end() { return list.end(); }
		auto begin() const { return list.begin(); }
		auto end() const { return list.end(); }
		int GetNodeCount() const { return NodeCount; }
	};

	void Set_Bit(unsigned char* array, int bit, int value);
	int Get_Bit(unsigned char const* array, int bit);
	int First_True_Bit(unsigned char const* array);
	int First_False_Bit(unsigned char const* array);

	class BooleanVectorClass
	{
	public:
		BooleanVectorClass(unsigned int size = 0, unsigned char* array = nullptr);
		BooleanVectorClass(BooleanVectorClass const& vector);
		BooleanVectorClass& operator =(BooleanVectorClass const& vector);
		bool operator == (BooleanVectorClass const& vector) const;

		void Init(unsigned int size, unsigned char* array);
		void Init(unsigned int size);
		int Length() { return BitCount; };
		void Reset();
		void Set();
		void Clear();
		int Resize(unsigned int size);

		bool const& operator[](int index) const
		{
			if (LastIndex != index)
			{
				Fixup(index);
			}

			return Copy;
		};

		bool& operator[](int index)
		{
			if (LastIndex != index)
			{
				Fixup(index);
			}

			return Copy;
		};

		bool Is_True(int index) const
		{
			if (index == LastIndex)
			{
				return Copy;
			}

			return Get_Bit(BitArray.begin(), index);
		};

		int First_False() const
		{
			if (LastIndex != -1)
			{
				Fixup(-1);
			}

			int retval = First_False_Bit(BitArray.begin());

			if (retval < BitCount)
			{
				return retval;
			}

			return -1;
		}

		int First_True() const
		{
			if (LastIndex != -1)
			{
				Fixup(-1);
			}

			int retval = First_True_Bit(BitArray.begin());

			if (retval < BitCount)
			{
				return retval;
			}

			return -1;
		}

		const VectorClass<unsigned char>& Get_Bit_Array() { return BitArray; }

	protected:
		void Fixup(int index = -1) const;
		int BitCount;
		bool Copy;
		int LastIndex;
		VectorClass<unsigned char> BitArray;
	};

	void Set_Bit(unsigned char* array, int bit, int value)
	{
		int bit2 = bit;

		if (bit < 0)
		{
			bit2 = bit + 7;
		}

		int bit3 = 1 << (bit - (bit2 & 0xF8));

		if (value)
		{
			array[bit / 8] |= bit3;
		}
		else
		{
			array[bit / 8] &= ~(unsigned char)bit3;
		}
	}

	int Get_Bit(unsigned char const* array, int bit)
	{
		int bit2 = bit;

		if (bit < 0)
		{
			bit2 = bit + 7;
		}

		return array[bit / 8] & (unsigned __int8)(1 << (bit - (bit2 & 0xF8)));
	}

	int First_True_Bit(unsigned char const* array)
	{
		int i;

		for (i = 0; !*array; array++)
		{
			i++;
		}

		int count = 0;

		do
		{
			if (Get_Bit(array, count))
			{
				break;
			}

			count++;
		} while (count <= 7);

		return count + 8 * i;
	}

	int First_False_Bit(unsigned char const* array)
	{
		int i;

		for (i = 0; *array == 0xFF; array++)
		{
			i++;
		}

		int count = 0;

		do
		{
			if (!Get_Bit(array, count))
			{
				break;
			}

			count++;
		} while (count <= 7);

		return count + 8 * i;
	}

	BooleanVectorClass::BooleanVectorClass(unsigned int size, unsigned char* array) : BitCount(size), Copy(false), LastIndex(-1), BitArray(0, nullptr)
	{
		BitArray.Resize(((size + (8 - 1)) / 8), array);
	}

	BooleanVectorClass::BooleanVectorClass(BooleanVectorClass const& vector)
	{
		LastIndex = -1;
		*this = vector;
	}

	BooleanVectorClass& BooleanVectorClass::operator =(BooleanVectorClass const& vector)
	{
		Fixup();
		Copy = vector.Copy;
		LastIndex = vector.LastIndex;
		BitArray = vector.BitArray;
		BitCount = vector.BitCount;
		return *this;
	}


	bool BooleanVectorClass::operator == (const BooleanVectorClass& vector) const
	{
		Fixup(LastIndex);
		return BitCount == vector.BitCount && BitArray == vector.BitArray;
	}

	int BooleanVectorClass::Resize(unsigned int size)
	{
		Fixup();

		if (size > 0)
		{
			unsigned int oldsize = BitCount;
			int success = BitArray.Resize(((size + (8 - 1)) / 8));
			BitCount = size;

			if (success && oldsize < size)
			{
				for (unsigned int index = oldsize; index < size; index++)
				{
					(*this)[index] = false;
				}
			}

			return success;
		}

		Clear();
		return true;
	}

	void BooleanVectorClass::Clear()
	{
		Fixup();
		BitCount = 0;
		BitArray.Clear();
	}

	void BooleanVectorClass::Reset()
	{
		LastIndex = -1;

		if (BitArray.Length() > 0)
		{
			memset(BitArray.begin(), '\0', BitArray.Length());
		}
	}

	void BooleanVectorClass::Set()
	{
		LastIndex = -1;

		if (BitArray.Length() > 0)
		{
			memset(BitArray.begin(), '\xFF', BitArray.Length());
		}
	}

	void BooleanVectorClass::Fixup(int index) const
	{
		if ((unsigned int)index >= (unsigned int)BitCount)
		{
			index = -1;
		}

		if (index != LastIndex)
		{
			if (LastIndex != -1)
			{
				assert((unsigned int)LastIndex < (unsigned int)BitCount);
				Set_Bit((unsigned char*)BitArray.begin(), LastIndex, Copy);
			}

			if (index != -1)
			{
				assert((unsigned int)index < (unsigned int)BitCount);
				((unsigned char&)Copy) = (unsigned char)Get_Bit(BitArray.begin(), index);
			}

			((BooleanVectorClass*)this)->LastIndex = index;
		}
	}

	void BooleanVectorClass::Init(unsigned int size, unsigned char* array)
	{
		Copy = false;
		LastIndex = -1;
		BitCount = size;
		BitArray.Resize(((size + (8 - 1)) / 8), array);
	}

	void BooleanVectorClass::Init(unsigned int size)
	{
		Copy = false;
		LastIndex = -1;
		BitCount = size;
		BitArray.Resize(((size + (8 - 1)) / 8));
	}

	class LogDataDialogClass
	{
		enum DialogState : uint32_t {
			UNINITIALIZED,
			INITIALIZED,
			REVIEW_LOG,
			PRESSED_OK,
			DESTROY,
		};

		HWND DlgWindow;
		HWND ParentWindow;
		HANDLE ThreadHandle;
		std::atomic<DialogState> State;
		std::vector<wchar_t> StringBuilder; // ghetto string builder
		FastCriticalSectionClass Mutex;

		static constexpr UINT_PTR LogDataDialogTimer = 12591025;
		static LogDataDialogClass* Instance;
		static int TotalVertexCount;
	public:
		static void AddToTotalVertexCount(int count) { TotalVertexCount += count; }

		template <typename... Args> static void WriteLogWindow(const wchar_t* format, Args... argList)
		{
			if (Instance)
			{
				if constexpr (sizeof...(argList) == 0)
				{
					Instance->WriteWindow(format);
				}

				else
				{
					Instance->PrintWindow(format, argList...);
				}
			}
		}

		static void CreateLogDialog(HWND parent)
		{
			Instance = new LogDataDialogClass(parent);
			TotalVertexCount = 0;
		}

		static void PrintTotalVertexCount()
		{
			WriteLogWindow(L"\n");
			WriteLogWindow(L"-------------------------------------------------------------\n");
			WriteLogWindow(L"\n");
			WriteLogWindow(L"Vertex Count: %d\n", TotalVertexCount);
			WriteLogWindow(L"\n");
		}

		static void DestroyLogDialog(bool review)
		{
			if (review)
			{
				LogDataDialogClass::Instance->OnReviewLog();
			}

			delete LogDataDialogClass::Instance;
			LogDataDialogClass::Instance = nullptr;
		}

		LogDataDialogClass(HWND parent) : DlgWindow(nullptr), ParentWindow(parent), State(UNINITIALIZED)
		{
			TT_PROFILER_SCOPE("Initialize Log Dialog");
			ThreadHandle = (HANDLE)_beginthreadex(nullptr, 0, ThreadProc, this, 0, nullptr);

			if (ThreadHandle)
			{
				StringBuilder.reserve(1'000'000);

				while (!State)
				{
					Sleep(4);
				}
			}
		}

		~LogDataDialogClass()
		{
			DestroyWindow();
			WaitForSingleObject(ThreadHandle, 1000);
			CloseHandle(ThreadHandle);
		}

		void DestroyWindow()
		{
			State = DESTROY;

			if (IsWindow(DlgWindow))
			{
				SendMessage(DlgWindow, WM_CLOSE, 0, 0);
			}

			FastCriticalSectionClass::LockClass lock(Mutex);
			StringBuilder.clear();
		}

		void __declspec(noinline) WriteWindow(WideStringView string)
		{
			FastCriticalSectionClass::LockClass lock(Mutex);
			StringBuilder.insert(StringBuilder.end(), string.begin(), string.end());
		}

		//+V576, namespace:W3D::MaxTools, class:LogDataDialogClass, function:PrintWindow, format_arg:1, ellipsis_arg:2
		void __declspec(noinline) PrintWindow(_Printf_format_string_ const wchar_t* format, ...)
		{
			static WideStringClass str;
			va_list va;
			va_start(va, format);
			str.Format_Args(format, va);
			va_end(va);
			FastCriticalSectionClass::LockClass lock(Mutex);
			StringBuilder.insert(StringBuilder.end(), str.Peek_Buffer(), str.Peek_Buffer() + str.Get_Length());
		}

		void OnReviewLog()
		{
			State = REVIEW_LOG;
			EnableWindow(GetDlgItem(DlgWindow, IDOK), TRUE);
			SetForegroundWindow(DlgWindow);
			PostMessage(DlgWindow, WM_TIMER, LogDataDialogTimer, 0); // manually trigger timer
			KillTimer(DlgWindow, LogDataDialogTimer); // kill actual timer

			while (State < PRESSED_OK)
			{
				Sleep(8);
			}
		}

		void OnInitDialog()
		{
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			RECT r;
			RECT r2;
			GetWindowRect(GetDesktopWindow(), &r);
			GetWindowRect(DlgWindow, &r2);
			SetWindowPos(DlgWindow, nullptr, r.left + (r2.left + r.right - r.left - r2.right) / 2, r.top + (r2.top + r.bottom - r.top - r2.bottom) / 2, 0, 0, SWP_NOSIZE);
			EnableWindow(GetDlgItem(DlgWindow, IDOK), FALSE);
			SetTimer(DlgWindow, LogDataDialogTimer, 16, nullptr);
			State = INITIALIZED;
		}

		bool RealDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_CLOSE:
				if (State >= PRESSED_OK)
				{
					KillTimer(DlgWindow, LogDataDialogTimer);
					EndDialog(DlgWindow, 1);
					DlgWindow = nullptr;
				}

				return true;
			case WM_INITDIALOG:
				OnInitDialog();
				return true;
			case WM_COMMAND:
				if (LOWORD(wParam) == IDOK)
				{
					State = PRESSED_OK;
					KillTimer(DlgWindow, LogDataDialogTimer);
					EndDialog(DlgWindow, 1);
					DlgWindow = nullptr;
					return true;
				}

				return false;
			case WM_TIMER:
				TT_PROFILER_SCOPE("Log To Dialog");

				if (State >= REVIEW_LOG)
				{
					KillTimer(DlgWindow, LogDataDialogTimer);
				}

				// NOTE(Mara): This is designed to do as few Edit_ReplaceSel calls as possible, as that is *by far* the slowest part.
				WideStringClass str;

				{
					FastCriticalSectionClass::LockClass lock(Mutex);
					str = WideStringView(StringBuilder.data(), StringBuilder.size());
					StringBuilder.clear();
				}

				if (!str.Is_Empty() && State < PRESSED_OK)
				{
					TT_PROFILER_TAG("Length", str.Get_Length());
					HWND log = GetDlgItem(DlgWindow, IDC_LOG);
					Edit_SetSel(log, 0xFFFFFFFF, -1);
					Edit_ReplaceSel(log, str.Peek_Buffer());
					Edit_ScrollCaret(log);
				}

				return true;
			}

			return false;
		}

		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			if (message == WM_INITDIALOG)
			{
				TT_PROFILER_THREAD_START("LogDataDialog");
				TT_PROFILER_SCOPE_START("LogDataDialog");
				((LogDataDialogClass*)lParam)->DlgWindow = hWnd;
				SetProp(hWnd, L"LogDataDialogClass", (HANDLE)lParam);
			}

			LogDataDialogClass* dlg = (LogDataDialogClass*)GetProp(hWnd, L"LogDataDialogClass");

			if (message == WM_DESTROY)
			{
				TT_PROFILER_SCOPE_STOP();
				TT_PROFILER_THREAD_STOP();
				RemoveProp(hWnd, L"LogDataDialogClass");
			}

			if (dlg)
			{
				return dlg->RealDlgProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}

		static unsigned int __stdcall ThreadProc(LPVOID lpThreadParameter)
		{
			DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_W3D_EXPORT_LOG), ((LogDataDialogClass*)lpThreadParameter)->ParentWindow, LogDataDialogClass::DlgProc, (LPARAM)lpThreadParameter);
			return 0;
		}
	};

	LogDataDialogClass* LogDataDialogClass::Instance = nullptr;
	int LogDataDialogClass::TotalVertexCount = 0;

#ifndef W3X
	std::unordered_map<Object*, StringClass> ObjectMap;
	bool MeshDeduplication = false;
#endif

	class HierarchySave
	{
		struct HierarchyNodeStruct : public NoEqualsClass<HierarchyNodeStruct>
		{

			INode* Node;
			W3dPivotStruct pivot;
			W3dPivotFixupStruct pivotfixup;
		};

		TimeValue Time;
		W3dHierarchyStruct header;
		DynamicVectorClass<HierarchyNodeStruct> Bones;
		int BoneCount;
		int Unk;
		Matrix3 NodeTMInverse;
		HierarchySave* BasePose;
		std::unordered_map<StringClass, int, hash_string, equals_string> BoneMap;

	public:
		const char* GetHierarchyName();
		Matrix3 FixupMatrix(Matrix3& m);
		Matrix3 GetTransform(int bone);
		INode* GetNode(int bone);
		const char* GetName(int bone);
		int FindBone(const char* name);
		int FindBone(const StringClass& name);
		int GetBoneIndexForNode(INode* node);
		Matrix3 GetPivotFixup(int bone);
#ifndef W3X
		bool SaveHierarchy(ChunkSaveClass& csave);
		bool SaveHierarchyHeader(ChunkSaveClass& csave);
		bool SavePivots(ChunkSaveClass& csave);
		bool SavePivotFixups(ChunkSaveClass& csave);
		bool LoadHierarchy(ChunkLoadClass& cload);
		bool LoadHierarchyHeader(ChunkLoadClass& cload);
		bool LoadPivots(ChunkLoadClass& cload);
		bool LoadPivotFixups(ChunkLoadClass& cload);
#else
		bool SaveHierarchy(XMLWriter& csave);
		bool SavePivots(XMLWriter& csave);
		bool LoadHierarchy(const char* filename);
#endif
		HierarchySave();
		~HierarchySave();
		void GetFinalTransform(Matrix3& m, int bone);
		void FindBoneForNode(INode* node, int* boneindex, INode** bonenode, Matrix3* bonepivot);
		int AddNode(INode* node, int parent);
		void AddNodeRecursive(INode* node, int parent);
		HierarchySave(INode* node, TimeValue time, const char* name, int unk, HierarchySave* basepose);
		HierarchySave(INodeListClass* nodes, TimeValue time, char* name, int unk, HierarchySave* basepose, Matrix3* tm);

		int GetBoneCount() { return BoneCount; }
	};

	class AnimationSave
	{
		IScene* Scene;
		INode* Node;
		INodeListClass* Tree;
		HierarchySave* Hierarchy;
		int StartFrame;
		int EndFrame;
		int NumFrames;
		int FrameRate;
#ifdef W3X
		W3DExportSettings* ExportStr;
#endif
		char Name[W3D_NAME_LEN];
		Matrix3 Matrix;
		Matrix3** Transforms;
		Point3** Angles;
		BooleanVectorClass* BitChannels;
		VectorClass<float>* VisibilityChannels;
		BooleanVectorClass* BinaryMove;
		BooleanVectorClass HasData;

	public:
		AnimationSave(IScene* scene, INode* node, HierarchySave* hierarchyobject, W3DExportSettings* exportstr, int framerate, const char* name, Matrix3& mat);
		void CaptureBones();
		void CaptureFrame(int frame);
		void CopyTransform(int bone, int frame, Matrix3& transform);
		void SetAngles(int bone, int frame, float xrot, float yrot, float zrot);
		void CopyVisibility(int bone, int frame, bool vis, float floatvis);
		Matrix3 GetTransform(int node, int frame);
#ifndef W3X
		bool WriteAnimation(ChunkSaveClass& csave);
		bool WriteAnimationHeader(ChunkSaveClass& csave);
		bool WriteAnimationChannels(ChunkSaveClass& csave);
#else
		bool WriteAnimation(XMLWriter& csave);
		bool WriteAnimationHeader(XMLWriter& csave);
		bool WriteAnimationChannels(XMLWriter& csave);
#endif
		~AnimationSave();
	};

	W3DExport::W3DExport() : ExpInt(nullptr), Int(nullptr), Time(0), FrameRate(0), OriginNodeList(nullptr), HierarchyStruct(nullptr)
	{
		ClearNameMap();
	}

	int W3DExport::ExtCount()
	{
		return static_cast<int>(valid_w3d_export_extensions().size());
	}

	const MCHAR* W3DExport::Ext(int n)
	{
		return valid_w3d_export_extensions()[n];
	}

	const MCHAR* W3DExport::LongDesc()
	{
#ifndef W3X
		return _M("W3D Assets");
#else
		return _M("W3D XML Assets");
#endif
	}

	const MCHAR* W3DExport::ShortDesc()
	{
#ifndef W3X
		return _M("W3D Asset");
#else
		return _M("W3D XML Asset");
#endif
	}

	const MCHAR* W3DExport::AuthorName()
	{
		return _M("Tiberian Technologies");
	}

	const MCHAR* W3DExport::CopyrightMessage()
	{
		return _M("(c) Tiberian Technologies 2023");
	}

	const MCHAR* W3DExport::OtherMessage1()
	{
		return nullptr;
	}

	const MCHAR* W3DExport::OtherMessage2()
	{
		return nullptr;
	}

	unsigned int W3DExport::Version()
	{
		return 100; //100 per major version increment
	}

	void W3DExport::ShowAbout(HWND hWnd)
	{
	}

	int W3DExport::DoExport(const MCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options)
	{
		timeBeginPeriod(1);
		ExpInt = ei;
		Int = i;
		OriginNodeList = nullptr;
		HierarchyStruct = nullptr;

		try {
			Time = i->GetTime();
			FrameRate = GetFrameRate();
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			StringClass n = name;
			_splitpath(n, drive, dir, fname, nullptr);

			if (strlen(fname) >= W3D_NAME_LEN)
			{
				WideStringClass str;
				str.Format(L"Warning: W3D filenames should be %d characters or less!", W3D_NAME_LEN);
				MessageBox(nullptr, str.Peek_Buffer(), L"Warning", 0);
			}

			StringClass str = i->GetCurFilePath();
			_splitpath(str, drive, dir, nullptr, nullptr);
			sprintf(Path, "%s%s", drive, dir);
			ReferenceTarget* node = Int->GetScenePointer();
			AppDataChunk* chunk = node->GetAppDataChunk(W3DExportClassDesc::Instance()->ClassID(), W3DExportClassDesc::Instance()->SuperClassID(), 0);

			if (!chunk)
			{
				node->AddAppDataChunk(W3DExportClassDesc::Instance()->ClassID(), W3DExportClassDesc::Instance()->SuperClassID(), 0, 0, nullptr);
				chunk = node->GetAppDataChunk(W3DExportClassDesc::Instance()->ClassID(), W3DExportClassDesc::Instance()->SuperClassID(), 0);
			}

			if (!chunk->data)
			{
				chunk->length = sizeof(W3DExportSettings);
				void* alloc = MAX_malloc(chunk->length);
				chunk->data = new(alloc) W3DExportSettings;
				((W3DExportSettings*)chunk->data)->AnimFramesStart = Int->GetAnimRange().Start() / GetTicksPerFrame();
				((W3DExportSettings*)chunk->data)->AnimFramesEnd = Int->GetAnimRange().End() / GetTicksPerFrame();
			}

			if (chunk->length == sizeof(OldW3DExportSettings))
			{
				OldW3DExportSettings* data = (OldW3DExportSettings*)chunk->data;
				chunk->length = sizeof(W3DExportSettings);
				void* alloc = MAX_malloc(chunk->length);
				chunk->data = new(alloc) W3DExportSettings;
				memcpy(chunk->data, data, sizeof(OldW3DExportSettings));
				MAX_free(data);
			}

			W3DExportSettings* settings = (W3DExportSettings*)chunk->data;
			W3DExportDlg dlg(*settings);

			if (dlg.ShowDialog() == IDOK)
			{
				TT_PROFILER_THREAD_START("Export Thread");
				TT_PROFILER_START_CAPTURE(Optick::Mode::Type(Optick::Mode::AUTOSAMPLING | Optick::Mode::INSTRUMENTATION | Optick::Mode::SWITCH_CONTEXT | Optick::Mode::TAGS), 8192);
				TT_PROFILER_SCOPE("W3DExport::DoExport");

				memcpy(&m_Settings, settings, sizeof(W3DExportSettings));
				WideStringClass str1 = m_Settings.ExistingSkeletonFileName;
				StringClass str2 = str1;
				strcpy(SkeletonPath, str2);

				switch (m_Settings.ExportType)
				{
				case W3DExportType::HierarchicalModel:
					m_Settings.ExportSkeleton = !m_Settings.UseExistingSkeleton;
					m_Settings.ExportAnimation = false;
					m_Settings.ExportGeometry = true;
					m_Settings.ExportAsTerrain = false;
					break;
				case W3DExportType::HierarchicalAnimatedModel:
					m_Settings.ExportSkeleton = !m_Settings.UseExistingSkeleton;
					m_Settings.ExportAnimation = true;
					m_Settings.ExportGeometry = true;
					m_Settings.ExportAsTerrain = false;
					break;
				case W3DExportType::PureAnimation:
					m_Settings.ExportSkeleton = !m_Settings.UseExistingSkeleton;
					m_Settings.ExportAnimation = true;
					m_Settings.ExportGeometry = false;
					m_Settings.ExportAsTerrain = false;
					m_Settings.OptimiseCollisions = true;
					m_Settings.SmoothVertexNormals = false;
#ifndef W3X
					m_Settings.MeshDeduplication = false;
					m_Settings.NewAABTree = false;
#endif
					break;
				case W3DExportType::Skeleton:
					m_Settings.ExportSkeleton = true;
					m_Settings.UseExistingSkeleton = false;
					m_Settings.ExportAnimation = false;
					m_Settings.ExportGeometry = false;
					m_Settings.ExportAsTerrain = false;
					m_Settings.OptimiseCollisions = true;
					m_Settings.SmoothVertexNormals = false;
					m_Settings.ExportAsTerrain = false;
#ifndef W3X
					m_Settings.MeshDeduplication = false;
					m_Settings.NewAABTree = false;
#endif
					break;
#ifndef W3X
				case W3DExportType::Terrain:
					m_Settings.ExportSkeleton = true;
					m_Settings.UseExistingSkeleton = false;
					m_Settings.ExportAnimation = false;
					m_Settings.ExportGeometry = true;
					m_Settings.ExportAsTerrain = true;
					m_Settings.OptimiseCollisions = true;
					m_Settings.MeshDeduplication = false;
					break;
#endif
				case W3DExportType::SimpleMesh:
					m_Settings.ExportSkeleton = false;
					m_Settings.UseExistingSkeleton = false;
					m_Settings.ExportAnimation = false;
					m_Settings.ExportGeometry = true;
					m_Settings.ExportAsTerrain = false;
					m_Settings.SmoothVertexNormals = false;
					m_Settings.ExportAsTerrain = false;
#ifndef W3X
					m_Settings.MeshDeduplication = false;
#endif
					break;
				}

				if (m_Settings.ExportSkeleton || m_Settings.ExportAnimation || m_Settings.ExportGeometry)
				{
#ifndef W3X
					MeshDeduplication = m_Settings.MeshDeduplication;
					ObjectMap.clear();
#endif
					LogDataDialogClass::CreateLogDialog(nullptr);
					StringClass fn = name;
					BufferedFileClass file(fn);

					if (file.Open(2))
					{
						TT_PROFILER_SCOPE("Export");
#ifndef W3X
						ChunkSaveClass csave(&file);
#else
						XMLWriter csave(&file, true);
#endif

						if (CreateOriginNodeList())
						{
							ExportData(fname, csave);
						}

						file.Close();
						SAFE_DELETE(HierarchyStruct);
						SAFE_DELETE(OriginNodeList);

						{
							TT_PROFILER_SCOPE("Redraw Views");
							Int->RedrawViews(Int->GetTime());
						}
					}
					else
					{
						MessageBox(nullptr, L"Unable to open file.", L"Error", MB_SETFOREGROUND);
					}

					LogDataDialogClass::PrintTotalVertexCount();
					LogDataDialogClass::DestroyLogDialog(m_Settings.ReviewLog);
				}
			}
		}
		catch (ErrorClass& e)
		{
			MessageBox(nullptr, e.GetError(), L"Error", MB_SETFOREGROUND);
		}

		TT_PROFILER_STOP_CAPTURE();
		TT_PROFILER_SAVE_CAPTURE("W3DExport");
		TT_PROFILER_THREAD_STOP();

		timeEndPeriod(1);
		return 1;
	}

	bool ExportAsTerrain = false;

	const char* HierarchySave::GetHierarchyName()
	{
		return header.Name;
	}

	Matrix3 DoMatrixFixup(Matrix3& m)
	{
		Matrix3 tm = m;

		for (int i = 0; i < 3; i++)
		{
			Point3 p1 = tm.GetRow(i);

			if (fabs(p1.x) < 0.00001f)
			{
				p1.x = 0.0;
			}

			if (fabs(p1.y) < 0.00001f)
			{
				p1.y = 0.0;
			}

			if (fabs(p1.z) < 0.00001f)
			{
				p1.z = 0.0;
			}

			tm.SetRow(i, Normalize(p1));
		}

		return tm;
	}

	void MakeMatrix3D(const Matrix3& m1, Matrix3D& m2)
	{
		m2[2][3] = m1[3][2];
		m2[0][3] = m1[3][0];
		m2[1][3] = m1[3][1];
		m2[0][0] = m1[0][0];
		m2[1][0] = m1[0][1];
		m2[2][0] = m1[0][2];
		m2[0][1] = m1[1][0];
		m2[1][1] = m1[1][1];
		m2[2][1] = m1[1][2];
		m2[0][2] = m1[2][0];
		m2[1][2] = m1[2][1];
		m2[2][2] = m1[2][2];
	}

	bool IsOriginNode(INode* node)
	{
		if (!node)
		{
			return false;
		}

		if (node->IsRootNode())
		{
			return true;
		}

		if (node->IsHidden())
		{
			return false;
		}

		INode* n = node->GetParentNode();

		if (!n || !n->IsRootNode())
		{
			return false;
		}

		const wchar_t* str = node->GetName();
		return _wcsnicmp(str, L"origin.", 7) == 0;
	}

	bool IsHierarchyRootNode(INode* node)
	{
		if (!node)
		{
			return false;
		}

		if (node->IsRootNode())
		{
			return true;
		}

		if (!IsOriginNode(node))
		{
			return false;
		}

		const wchar_t* str = node->GetName();
		int i;

		if (!_wcsicmp(str, L"origin.") || ((wcslen(str) > 7) && (swscanf(str + 7, L"%d", &i) == 1) && (i == 0)))
		{
			return true;
		}

		return false;
	}

	Modifier* FindSkinModifier(INode* nodePtr)
	{
		// Get object from node. Abort if no object.
		Object* ObjectPtr = nodePtr->GetObjectRef();

		if (!ObjectPtr)
		{
			return nullptr;
		}

		// Is derived object ?
		while (ObjectPtr && ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		{
			// Yes -> Cast.
			IDerivedObject* DerivedObjectPtr = (IDerivedObject*)(ObjectPtr);

			// Iterate over all entries of the modifier stack.
			int ModStackIndex = 0;

			while (ModStackIndex < DerivedObjectPtr->NumModifiers())
			{
				// Get current modifier.
				Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

				// Is this Physique ?
				if (ModifierPtr->ClassID() == SKIN_CLASSID)
				{
					// Yes -> Exit.
					return ModifierPtr;
				}

				// Next modifier stack entry.
				ModStackIndex++;
			}

			ObjectPtr = DerivedObjectPtr->GetObjRef();
		}

		// Not found.
		return nullptr;
	}

	bool HasSkin(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		if (W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType != W3DGeometryType::Normal)
		{
			return false;
		}

		return FindSkinModifier(node) != nullptr;
	}

	bool IsNormalGeometry(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry) && !HasSkin(node) && !wcsrchr(node->GetName(), '~') && W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::Normal;
	}

	bool IsCameraAligned(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::CamParal;
	}

	bool IsCameraOriented(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::CamOrient;
	}

#ifndef W3X
	bool IsCameraZOriented(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::CamZOrient;
	}

	bool IsNullGeometry(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::NullLOD;
	}
#endif

	bool IsAABox(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::AABox;
	}

	bool IsOBBox(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::OBBox;
	}

#ifndef W3X
	bool IsAggregate(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::Aggregate;
	}

	bool IsDazzle(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		if (!(enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry)))
		{
			return false;
		}

		return W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryType == W3DGeometryType::Dazzle;
	}
#endif

	bool IsExportBone(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		return !HasSkin(node) && !IsOriginNode(node) && enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportTransform);
	}

#ifndef W3X
	bool IsCollidePhysical(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).CollisionFlags, W3DCollisionFlags::Physical);
	}

	bool IsCollideProjectile(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).CollisionFlags, W3DCollisionFlags::Projectile);
	}

	bool IsCollideVis(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).CollisionFlags, W3DCollisionFlags::Vis);
	}

	bool IsCollideCamera(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).CollisionFlags, W3DCollisionFlags::Camera);
	}

	bool IsCollideVehicle(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).CollisionFlags, W3DCollisionFlags::Vehicle);
	}
#endif

	void CopyW3DName(char* newname, const char* oldname)
	{
		memset(newname, 0, W3D_NAME_LEN);
		strncpy(newname, oldname, W3D_NAME_LEN - 1);
		char* s = strrchr(newname, '.');
		int i;

		// TODO(Mara): Test perf difference!
		//if (s && (!s[1] || (std::from_chars(s + 1, newname + W3D_NAME_LEN - 1, i).ec == std::errc{})))
		if (s && (!s[1] || (sscanf(s + 1, "%d", &i) == 1)))
		{
			*s = 0;
		}

		_strupr(newname);
	}

	// TODO(Mara): We should cache more node-related data! E.g. the export "chunks", properties like "is origin", transforms, etc.
	struct NodePtrHash {
		size_t operator()(INode* ptr) const noexcept { return PointerHashFunc((void*)ptr); }
	};

	static std::unordered_map <INode*, StringClass, NodePtrHash> W3DNameMap;

	// literally just clears the name map. That's all it does, in function form.
	void ClearNameMap()
	{
		W3DNameMap.clear();
	}

	const StringClass& GetW3DNameFromNode(INode* node)
	{
		auto it = W3DNameMap.try_emplace(node, W3D_NAME_LEN);
		StringClass& w3dname = it.first->second;
		bool not_found = it.second;

		if (not_found)
		{
			StringClass temp(node->GetName(), true); // convert to ASCII
			CopyW3DName(w3dname.Peek_Buffer(), temp.Peek_Buffer()); // convert to W3D name
			w3dname.Get_Length(); // update length of string
		}

		return w3dname;
	}

	void CopyW3DNameFromNode(char* dest, INode* node)
	{
		const StringClass& name = GetW3DNameFromNode(node);
		size_t len = (size_t)name.Get_Length();
		memcpy(dest, name.Peek_Buffer(), len);
		size_t rest = max(W3D_NAME_LEN - len, 0);
		memset(dest + len, '\0', rest);
	}

#ifdef W3X
	unsigned int Do_CRC(const char* name)
	{
		if (name)
		{
			unsigned char n = (unsigned char)*name;

			if (*name)
			{
				unsigned int i1 = 0xFFFFFFFF;

				do
				{
					name++;

					if ((unsigned __int8)(n - 'a') <= 25)
					{
						n &= 223;
					}

					int i2 = n ^ (unsigned __int8)i1;
					n = *name;
					i1 = crc_table[i2] ^ (i1 >> 8);
				} while (*name);

				return ~i1;
			}
		}

		return 0;
	}

	bool CheckW3DName(const char* name)
	{
		for (char i = *name; ; i = *name)
		{
			if (!i)
			{
				return true;
			}

			if ((i < '0' || i > '9') && (i < 'a' || i > 'z') && (i < 'A' || i > 'Z') && i != '_' && i != '.' && i != '-')
			{
				break;
			}

			name++;
		}

		wchar_t buf[360];
		swprintf(buf, 360, L"Invalid character in name '%S'. Allowed are: Letters (a-z), numbers, _ . and -\nPlease fix re-export!", name);
		LogDataDialogClass::WriteLogWindow(buf);
		MessageBox(nullptr, buf, L"Error", MB_SETFOREGROUND);
		return false;
	}
#endif

	Matrix3 HierarchySave::FixupMatrix(Matrix3& m)
	{
		Matrix3 tm(m);
		Matrix3 tm2;
		int u = Unk;
		Quat q;
		Point3 p;
		Point3 s;

		if (!u)
		{
			Matrix3& tm3 = tm;
			tm2 = tm3;
		}
		else if (u == 1)
		{
			tm2.SetTrans(tm.GetTrans());
			Matrix3 tm3 = DoMatrixFixup(tm2);
			tm2 = tm3;
		}
		else if (u == 2)
		{
			DecomposeMatrix(tm, p, q, s);
			q.MakeMatrix(tm2);
			tm2.SetTrans(p);
			Matrix3 tm3 = DoMatrixFixup(tm2);
			tm2 = tm3;
		}

		return tm2;
	}

#ifndef W3X
	bool HierarchySave::SaveHierarchyHeader(ChunkSaveClass& csave)
	{
		return csave.Begin_Chunk(W3DChunkType::HIERARCHY_HEADER) && csave.Write(&header, sizeof(W3dHierarchyStruct)) == sizeof(W3dHierarchyStruct) && csave.End_Chunk();
	}
#endif

	Matrix3 HierarchySave::GetTransform(int bone)
	{
		Quat q;
		Point3 p;
		Matrix3 tm;
		Matrix3 tm2;
		p.x = Bones[bone].pivot.Translation.X;
		p.y = Bones[bone].pivot.Translation.Y;
		p.z = Bones[bone].pivot.Translation.Z;
		q.x = -Bones[bone].pivot.Rotation.Q[0];
		q.y = -Bones[bone].pivot.Rotation.Q[1];
		q.z = -Bones[bone].pivot.Rotation.Q[2];
		q.w = Bones[bone].pivot.Rotation.Q[3];
		tm.Translate(p);
		q.MakeMatrix(tm2);
		return tm2 * tm;
	}

	INode* HierarchySave::GetNode(int bone)
	{
		return Bones[bone].Node;
	}

	const char* HierarchySave::GetName(int bone)
	{
		return Bones[bone].pivot.Name;
	}

	int HierarchySave::FindBone(const StringClass& name)
	{
		auto it = BoneMap.find(name);

		if (it != BoneMap.end())
		{
			return it->second;
		}

		return -1;
	}

	int HierarchySave::FindBone(const char* name)
	{
		// TODO(Mara): std::unordered_map doesn't support heterogeneous lookup until C++20, so we're constructing a StringClass here
#if (_MSVC_LANG < 202002L)
		StringClass tmp(name, true);
		auto it = BoneMap.find(tmp);
#else
		auto it = BoneMap.find(name);
#endif

		if (it != BoneMap.end())
		{
			return it->second;
		}

		return -1;
	}

	int HierarchySave::GetBoneIndexForNode(INode* node)
	{
		const StringClass& name = GetW3DNameFromNode(node);
		int bone = FindBone(name);

		if (bone == -1)
		{
			bone = 0;
		}

		return bone;
	}

	Matrix3 HierarchySave::GetPivotFixup(int bone)
	{
		Matrix3 tm;
		Point3 p;

		for (int i = 0; i < 4; i++)
		{
			p.x = Bones[bone].pivotfixup.TM[i][0];
			p.y = Bones[bone].pivotfixup.TM[i][1];
			p.z = Bones[bone].pivotfixup.TM[i][2];
			tm.SetRow(i, p);
		}

		return tm;
	}

#ifndef W3X
	bool HierarchySave::SavePivots(ChunkSaveClass& csave)
	{
		if (!csave.Begin_Chunk(W3DChunkType::PIVOTS))
		{
			return false;
		}

		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			if (!(csave.Write(&Bones[i].pivot, sizeof(W3dPivotStruct)) == sizeof(W3dPivotStruct)))
			{
				return false;
			}
		}

		return csave.End_Chunk();
	}
#else
	bool HierarchySave::SavePivots(XMLWriter& csave)
	{
		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			if (csave.StartTag("Pivot", 1) && csave.SetStringAttribute("Name", Bones[i].pivot.Name) && csave.SetIntAttribute("Parent", Bones[i].pivot.ParentIdx) && csave.EndTag() && csave.WriteVector("Translation", Bones[i].pivot.Translation))
			{
				if (csave.WriteQuaternion("Rotation", Bones[i].pivot.Rotation))
				{
					if (csave.StartTag("FixupMatrix", 0))
					{
						for (int k = 0; k < 3; k++)
						{
							for (int j = 0; j < 4; j++)
							{
								char buf[16];
								sprintf(buf, "M%d%d", j, k);

								if (!csave.SetFloatAttribute(buf, Bones[i].pivotfixup.TM[j][k]))
								{
									return false;
								}
							}
						}

						if (!csave.EndTag() || !csave.WriteClosingTag())
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	}
#endif

#ifndef W3X
	bool HierarchySave::SavePivotFixups(ChunkSaveClass& csave)
	{
		if (!csave.Begin_Chunk(W3DChunkType::PIVOT_FIXUPS))
		{
			return false;
		}

		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			if (!(csave.Write(&Bones[i].pivotfixup, sizeof(W3dPivotFixupStruct)) == sizeof(W3dPivotFixupStruct)))
			{
				return false;
			}
		}

		return csave.End_Chunk();
	}
#endif

#ifndef W3X
	bool HierarchySave::LoadHierarchyHeader(ChunkLoadClass& cload)
	{
		if (cload.Read(&header, sizeof(W3dHierarchyStruct)) != sizeof(W3dHierarchyStruct))
		{
			return false;
		}

		Bones.Resize(header.NumPivots);
		Bones.Set_Active(header.NumPivots);
		BoneMap.reserve(header.NumPivots);
		BoneCount = 0;

		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			memset(&Bones[i], 0, sizeof(Bones[i]));
			const Matrix3 tm;
			Bones[i].pivotfixup.TM[0][0] = tm[0][0];
			Bones[i].pivotfixup.TM[0][1] = tm[0][1];
			Bones[i].pivotfixup.TM[0][2] = tm[0][2];
			Bones[i].pivotfixup.TM[1][0] = tm[1][0];
			Bones[i].pivotfixup.TM[1][1] = tm[1][1];
			Bones[i].pivotfixup.TM[1][2] = tm[1][2];
			Bones[i].pivotfixup.TM[2][0] = tm[2][0];
			Bones[i].pivotfixup.TM[2][1] = tm[2][1];
			Bones[i].pivotfixup.TM[2][2] = tm[2][2];
			Bones[i].pivotfixup.TM[3][0] = tm[3][0];
			Bones[i].pivotfixup.TM[3][1] = tm[3][1];
			Bones[i].pivotfixup.TM[3][2] = tm[3][2];
		}

		return true;
	}

	bool HierarchySave::LoadPivots(ChunkLoadClass& cload)
	{
		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			Bones[i].Node = nullptr;

			if (cload.Read(&Bones[i].pivot, sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct))
			{
				return false;
			}

			BoneMap.emplace(Bones[i].pivot.Name, i);
		}

		return true;
	}

	bool HierarchySave::LoadPivotFixups(ChunkLoadClass& cload)
	{
		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			if (cload.Read(&Bones[i].pivotfixup, sizeof(W3dPivotFixupStruct)) != sizeof(W3dPivotFixupStruct))
			{
				return false;
			}
		}

		return true;
	}
#endif

	HierarchySave::HierarchySave() : Time(0), BoneCount(0), Unk(0), BasePose(nullptr)
	{
	}

	HierarchySave::~HierarchySave()
	{
	}

	void HierarchySave::GetFinalTransform(Matrix3& m, int bone)
	{
		Matrix3 tm;

		for (int i = bone; i != -1; i = Bones[i].pivot.ParentIdx)
		{
			tm = tm * GetTransform(i);
		}

		m = tm;
	}

	void HierarchySave::FindBoneForNode(INode* node, int* boneindex, INode** bonenode, Matrix3* bonepivot)
	{
		INode* n = node;
		int index;

		for (;;)
		{
			const StringClass& name = GetW3DNameFromNode(n);
			index = FindBone(name);

			if (index != -1)
			{
				break;
			}

			if (IsOriginNode(n))
			{
				index = 0;
				break;
			}

			n = n->GetParentNode();
		}

		if (boneindex)
		{
			*boneindex = index;
		}

		if (bonenode)
		{
			*bonenode = n;
		}

		if (bonepivot)
		{
			Matrix3 tm = n->GetNodeTM(Time);
			Matrix3 tm2 = GetPivotFixup(index);
			*bonepivot = tm2 * tm;
		}
	}

#ifndef W3X
	bool HierarchySave::SaveHierarchy(ChunkSaveClass& csave)
	{
		LogDataDialogClass::LogDataDialogClass::WriteLogWindow(L"\nSaving Hierarchy Tree %S.\n", header.Name);
		LogDataDialogClass::LogDataDialogClass::WriteLogWindow(L"Node Count: %d\n", BoneCount);
		LogDataDialogClass::LogDataDialogClass::WriteLogWindow(L"Nodes: \n");

		for (int i = 0; i < BoneCount; i++)
		{
			char* name = Bones[i].pivot.Name;
			LogDataDialogClass::LogDataDialogClass::WriteLogWindow(L"  %hs\n", name); // NOTE(Mara): This is too expensive to be in the inner loop.
		}

		return csave.Begin_Chunk(W3DChunkType::HIERARCHY) && SaveHierarchyHeader(csave) && SavePivots(csave) && SavePivotFixups(csave) && csave.End_Chunk();
	}

	bool HierarchySave::LoadHierarchy(ChunkLoadClass& cload)
	{
		Bones.Clear();
		BoneMap.clear();
		while (cload.Open_Chunk())
		{
			bool b = true;

			switch ((W3DChunkType)cload.Cur_Chunk_ID())
			{
			case W3DChunkType::HIERARCHY_HEADER:
				b = LoadHierarchyHeader(cload);
				break;
			case W3DChunkType::PIVOTS:
				b = LoadPivots(cload);
				break;
			case W3DChunkType::PIVOT_FIXUPS:
				b = LoadPivotFixups(cload);
				break;
			}

			if (!cload.Close_Chunk() || !b)
			{
				return false;
			}
		}

		BoneCount = header.NumPivots;
		return true;
	}
#else

	bool HierarchySave::LoadHierarchy(const char* filename)
	{
		memset(&header, 0, sizeof(W3dHierarchyStruct));
		Bones.Clear();
		BoneMap.clear();
		header.Version = 0x40001;
		pugi::xml_document doc;
		WideStringClass str = filename;
		doc.load_file(str);
		const char* buf = doc.select_node("/AssetDeclaration/W3DHierarchy").node().attribute("id").value();
		const char* c = buf;

		if (strchr(buf, ':'))
		{
			c = strchr(buf, ':') + 1;
		}

		strncpy(header.Name, buf, W3D_NAME_LEN);
		pugi::xpath_node_set nodes = doc.select_nodes("/AssetDeclaration/W3DHierarchy/Pivot");
		header.NumPivots = (uint32)nodes.size();
		Bones.Resize((int)nodes.size());
		Bones.Set_Active((int)nodes.size());
		BoneCount = (int)nodes.size();
		BoneMap.reserve(nodes.size());

		for (unsigned int i = 0; i < header.NumPivots; i++)
		{
			memset(&Bones[i], 0, sizeof(Bones[i]));
			const Matrix3 tm;
			Bones[i].pivotfixup.TM[0][0] = tm[0][0];
			Bones[i].pivotfixup.TM[0][1] = tm[0][1];
			Bones[i].pivotfixup.TM[0][2] = tm[0][2];
			Bones[i].pivotfixup.TM[1][0] = tm[1][0];
			Bones[i].pivotfixup.TM[1][1] = tm[1][1];
			Bones[i].pivotfixup.TM[1][2] = tm[1][2];
			Bones[i].pivotfixup.TM[2][0] = tm[2][0];
			Bones[i].pivotfixup.TM[2][1] = tm[2][1];
			Bones[i].pivotfixup.TM[2][2] = tm[2][2];
			Bones[i].pivotfixup.TM[3][0] = tm[3][0];
			Bones[i].pivotfixup.TM[3][1] = tm[3][1];
			Bones[i].pivotfixup.TM[3][2] = tm[3][2];
			pugi::xml_node node = nodes[i].node();
			strncpy(Bones[i].pivot.Name, node.attribute("Name").value(), W3D_NAME_LEN);
			Bones[i].pivot.Name[W3D_NAME_LEN - 1] = 0;
			Bones[i].pivot.ParentIdx = node.attribute("Parent").as_int();
			pugi::xml_node translation = node.select_node("Translation").node();
			Bones[i].pivot.Translation.X = translation.attribute("X").as_float();
			Bones[i].pivot.Translation.Y = translation.attribute("Y").as_float();
			Bones[i].pivot.Translation.Z = translation.attribute("Z").as_float();
			pugi::xml_node rotation = node.select_node("Rotation").node();
			Bones[i].pivot.Rotation.Q[0] = rotation.attribute("X").as_float();
			Bones[i].pivot.Rotation.Q[1] = rotation.attribute("Y").as_float();
			Bones[i].pivot.Rotation.Q[2] = rotation.attribute("Z").as_float();
			Bones[i].pivot.Rotation.Q[3] = rotation.attribute("W").as_float();
			pugi::xml_node fixupmatrix = node.select_node("./FixupMatrix").node();

			for (int k = 0; k < 3; k++)
			{
				for (int j = 0; j < 4; j++)
				{
					char buf[16];
					sprintf(buf, "M%d%d", j, k);
					Bones[i].pivotfixup.TM[j][k] = fixupmatrix.attribute(buf).as_float();
				}
			}

			BoneMap.emplace(Bones[i].pivot.Name, i);
		}

		return true;
	}

	bool HierarchySave::SaveHierarchy(XMLWriter& csave)
	{
		LogDataDialogClass::WriteLogWindow(L"\nSaving Hierarchy Tree %S.\n", header.Name);
		LogDataDialogClass::WriteLogWindow(L"Node Count: %d\n", BoneCount);
		LogDataDialogClass::WriteLogWindow(L"Nodes: \n");

		for (int i = 0; i < BoneCount; i++)
		{
			char* name = Bones[i].pivot.Name;
			LogDataDialogClass::WriteLogWindow(L"  %hs\n", name); // NOTE(Mara): This is too expensive to be in the inner loop.
		}

		return csave.StartTag("W3DHierarchy", 1) && csave.SetStringAttribute("id", header.Name) && csave.EndTag() && SavePivots(csave) && csave.WriteClosingTag();
	}
#endif

	int HierarchySave::AddNode(INode* node, int parent)
	{
		if (BoneCount >= Bones.Length())
		{
			Bones.Grow();
		}

		Bones[BoneCount].Node = node;
		Bones[BoneCount].pivot.ParentIdx = parent;

		if (node)
		{
			CopyW3DNameFromNode(Bones[BoneCount].pivot.Name, node);
		}
		else
		{
			CopyW3DName(Bones[BoneCount].pivot.Name, "ROOTTRANSFORM"); // TODO(Mara): this is stupid
		}

		if (FindBone(Bones[BoneCount].pivot.Name) != -1)
		{
			throw ErrorClass(L"Bones with duplicate names found!\nThis could be due to names exceeding %i characters.\n\nDuplicated Name: %S\n", W3D_NAME_LEN, Bones[BoneCount].pivot.Name);
		}

		Matrix3 tm;
		Point3 p;
		Matrix3 tm4;
		Quat q(tm4);
		Point3 s;

		if (node)
		{
			tm = node->GetNodeTM(Time) * NodeTMInverse;
		}
		else
		{
			tm;
		}

		HierarchySave* pose = BasePose;

		if (pose)
		{
			const char* bone = Bones[BoneCount].pivot.Name;
			int pivot = pose->FindBone(bone);

			if (pivot == -1)
			{
				throw ErrorClass(L"Incompatible Base Pose!\nMissing Bone: %S\n", bone);
			}

			tm = pose->GetPivotFixup(pivot) * tm;
		}

		Matrix3 tm2 = FixupMatrix(tm);
		Matrix3 tm3 = tm2 * Inverse(tm);

		if (parent != -1)
		{
			GetFinalTransform(tm4, parent);
			tm2 = tm2 * Inverse(tm4);
		}

		DecomposeMatrix(tm2, p, q, s);

		Bones[BoneCount].pivotfixup.TM[0][0] = tm3.GetRow(0)[0];
		Bones[BoneCount].pivotfixup.TM[0][1] = tm3.GetRow(0)[1];
		Bones[BoneCount].pivotfixup.TM[0][2] = tm3.GetRow(0)[2];
		Bones[BoneCount].pivotfixup.TM[1][0] = tm3.GetRow(1)[0];
		Bones[BoneCount].pivotfixup.TM[1][1] = tm3.GetRow(1)[1];
		Bones[BoneCount].pivotfixup.TM[1][2] = tm3.GetRow(1)[2];
		Bones[BoneCount].pivotfixup.TM[2][0] = tm3.GetRow(2)[0];
		Bones[BoneCount].pivotfixup.TM[2][1] = tm3.GetRow(2)[1];
		Bones[BoneCount].pivotfixup.TM[2][2] = tm3.GetRow(2)[2];
		Bones[BoneCount].pivotfixup.TM[3][0] = tm3.GetRow(3)[0];
		Bones[BoneCount].pivotfixup.TM[3][1] = tm3.GetRow(3)[1];
		Bones[BoneCount].pivotfixup.TM[3][2] = tm3.GetRow(3)[2];
		Bones[BoneCount].pivot.Translation.X = p.x;
		Bones[BoneCount].pivot.Translation.Y = p.y;
		Bones[BoneCount].pivot.Translation.Z = p.z;
		Bones[BoneCount].pivot.Rotation.Q[0] = -q.x;
		Bones[BoneCount].pivot.Rotation.Q[1] = -q.y;
		Bones[BoneCount].pivot.Rotation.Q[2] = -q.z;
		Bones[BoneCount].pivot.Rotation.Q[3] = q.w;

		Matrix3 tm5;
		q.MakeMatrix(tm5);

		Matrix3D m;
		MakeMatrix3D(tm5, m);

		EulerAngles e;
		e.FromMatrix3D(m);

		Bones[BoneCount].pivot.EulerAngles.X = (float)e[0];
		Bones[BoneCount].pivot.EulerAngles.Y = (float)e[1];
		Bones[BoneCount].pivot.EulerAngles.Z = (float)e[2];

		BoneMap.emplace(Bones[BoneCount].pivot.Name, BoneCount);
		int count = BoneCount;
		BoneCount = count + 1;
		Bones.Set_Active(BoneCount);
		return count;
	}

	void HierarchySave::AddNodeRecursive(INode* node, int parent)
	{
#ifndef W3X
		if (!node->IsHidden() && (!ExportAsTerrain || !IsNormalGeometry(node) && !IsNullGeometry(node)) && IsExportBone(node))
#else
		if (!node->IsHidden() && (!ExportAsTerrain || !IsNormalGeometry(node)) && IsExportBone(node))
#endif
		{
			parent = AddNode(node, parent);
		}

		for (int i = 0; i < node->NumberOfChildren(); i++)
		{
			AddNodeRecursive(node->GetChildNode(i), parent);
		}
	}

	HierarchySave::HierarchySave(INode* node, TimeValue time, const char* name, int unk, HierarchySave* basepose) :
		Time(time), Bones(512), BoneCount(0), Unk(unk), BasePose(basepose)
	{
		Bones.Grow(node->NumberOfChildren());
		BoneMap.reserve(node->NumberOfChildren());
		NodeTMInverse = Inverse(node->GetNodeTM(time));
		int pivot = AddNode(nullptr, -1);
		AddNodeRecursive(node, pivot);
		header.Version = 0x40001;
		CopyW3DName(header.Name, name);
#ifndef W3X
		header.Center.X = 0;
		header.Center.Y = 0;
		header.Center.Z = 0;
#endif
		header.NumPivots = BoneCount;
	}

	HierarchySave::HierarchySave(INodeListClass* nodes, TimeValue time, char* name, int unk, HierarchySave* basepose, Matrix3* tm) :
		Time(time), Bones(200), BoneCount(0), Unk(unk), NodeTMInverse(*tm), BasePose(basepose)
	{
		Bones.Grow(nodes->GetNodeCount());
		BoneMap.reserve(nodes->GetNodeCount());
		int pivot = AddNode(nullptr, -1);

		for (int i = 0; i < nodes->GetNodeCount(); i++)
		{
			AddNodeRecursive(nodes->GetNode(i), pivot);
		}

		header.Version = 0x40001;
		CopyW3DName(header.Name, name);
		header.Center.X = 0;
		header.Center.Y = 0;
		header.Center.Z = 0;
		header.NumPivots = BoneCount;
	}

	class OriginFilterClass final : public INodeListFilter
	{
	public:
		BOOL filter(INode* node, TimeValue time)
		{
			return IsOriginNode(node);
		}
	};

	bool GetExportGeometry(INode* node)
	{
		if (node->IsGroupHead())
		{
			return false;
		}

		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).ExportFlags, W3DExportFlags::ExportGeometry);
	}

	class GeometryFilterClass final : public INodeListFilter
	{
	public:
		BOOL filter(INode* node, TimeValue time)
		{
			return node->EvalWorldState(time).obj && !IsOriginNode(node) && !node->IsHidden() && GetExportGeometry(node);
		}
	};

	class GeometryExportTaskClass;

	class WorldInfoClass
	{
	public:
		virtual ~WorldInfoClass() {};
		virtual Vector3 Get_Shared_Vertex_Normal(Vector3 v, int smoothing) = 0;
		virtual bool Are_Meshes_Smoothed() { return true; }
	};

	class MaxWorldInfoClass : public WorldInfoClass
	{
		DynamicVectorClass<GeometryExportTaskClass*>* Vector;
		GeometryExportTaskClass* CurrentGeometryTask;
		Matrix3 Transform;
		bool SmoothNormals;
	public:
		MaxWorldInfoClass(DynamicVectorClass<GeometryExportTaskClass*>* v) : Vector(v), CurrentGeometryTask(nullptr), SmoothNormals(true)
		{
		}

		virtual ~MaxWorldInfoClass() {};
		virtual Vector3 Get_Shared_Vertex_Normal(Vector3 v, int smoothing);
		virtual bool Are_Meshes_Smoothed() { return SmoothNormals; }
		virtual GeometryExportTaskClass* Get_Current_Geometry_Task() { return CurrentGeometryTask; }
		virtual void Set_Current_Geometry_Task(GeometryExportTaskClass* task) { CurrentGeometryTask = task; }
		virtual void Get_Transform(Matrix3& tm) { tm = Transform; }
		virtual void Set_Transform(Matrix3& tm) { Transform = tm; }
		virtual void Set_Are_Meshes_Smoothed(bool b) { SmoothNormals = b; }
	};

	class LodData
	{
	public:
		const char* Name;
#ifndef W3X
		ChunkSaveClass* ChunkSave;
#else
		XMLWriter* ChunkSave;
		std::vector<StringClass>* Includes;
#endif
		MaxWorldInfoClass* Info;
		W3DExportSettings* ExportData;
		TimeValue Time;
		HierarchySave* Hierarchy;
		INodeListClass* NodeList;
		INode* Node;
		Matrix3 Transform;

#ifndef W3X
		LodData(const char* name, ChunkSaveClass* csave, MaxWorldInfoClass* info, W3DExportSettings* exportdata, HierarchySave* hierarchy, INode* node, INodeListClass* nodelist, TimeValue time) : ChunkSave(csave), Info(info), ExportData(exportdata), Time(time), Hierarchy(hierarchy), NodeList(nodelist), Node(node)
#else
		LodData(const char* name, XMLWriter* csave, std::vector<StringClass>* includes, MaxWorldInfoClass* info, W3DExportSettings* exportdata, HierarchySave* hierarchy, INode* node, INodeListClass* nodelist, TimeValue time) : ChunkSave(csave), Includes(includes), Info(info), ExportData(exportdata), Time(time), Hierarchy(hierarchy), NodeList(nodelist), Node(node)
#endif
		{
			Name = newstr(name);
			Transform = node->GetNodeTM(Time);
		}
	};

	class MeshConnection
	{
		class ConnectionStruct : public NoEqualsClass<ConnectionStruct>
		{
		public:
			int BoneIndex;
			char Name[W3D_NAME_LEN * 2];
			INode* Node;
#ifdef W3X
			int Type;
#endif

#ifndef W3X
			ConnectionStruct() : BoneIndex(0), Node(nullptr)
#else
			ConnectionStruct() : BoneIndex(0), Node(nullptr), Type(-1)
#endif
			{
			}
		};

	public:
		TimeValue Time;
		INode* Node;
		char Name[W3D_NAME_LEN];
		DynamicVectorClass<ConnectionStruct> Meshes;
		DynamicVectorClass<ConnectionStruct> Aggregates;
		DynamicVectorClass<ConnectionStruct> Proxies;

#ifndef W3X
		bool GetMeshConnectionInfo(int index, const char** name, int* bone, INode** node)
#else
		bool GetMeshConnectionInfo(int index, const char** name, int* bone, INode** node, int* type)
#endif
		{
			if (index >= Meshes.Count())
			{
				return false;
			}

			if (name)
			{
				*name = Meshes[index].Name;
			}

			if (bone)
			{
				*bone = Meshes[index].BoneIndex;
			}

			if (node)
			{
				*node = Meshes[index].Node;
			}

#ifdef W3X
			if (type)
			{
				*type = Meshes[index].Type;
			}
#endif

			return true;
		}

#ifndef W3X
		bool GetAggregateConnectionInfo(int index, const char** name, int* bone, INode** node)
#else
		bool GetAggregateConnectionInfo(int index, const char** name, int* bone, INode** node, int* type)
#endif
		{
			if (index >= Aggregates.Count())
			{
				return false;
			}

			if (name)
			{
				*name = Aggregates[index].Name;
			}

			if (bone)
			{
				*bone = Aggregates[index].BoneIndex;
			}

			if (node)
			{
				*node = Aggregates[index].Node;
			}

#ifdef W3X
			if (type)
			{
				*type = Aggregates[index].Type;
			}
#endif

			return true;
		}

#ifndef W3X
		bool GetProxyConnectionInfo(int index, const char** name, int* bone, INode** node)
#else
		bool GetProxyConnectionInfo(int index, const char** name, int* bone, INode** node, int* type)
#endif
		{
			if (index >= Proxies.Count())
			{
				return false;
			}

			if (name)
			{
				*name = Proxies[index].Name;
			}

			if (bone)
			{
				*bone = Proxies[index].BoneIndex;
			}

			if (node)
			{
				*node = Proxies[index].Node;
			}

#ifdef W3X
			if (type)
			{
				*type = Proxies[index].Type;
			}
#endif

			return true;
		}

		MeshConnection(DynamicVectorClass<GeometryExportTaskClass*> vector, LodData& lod);
	};

	int GetLODLevelFromNode(INode* node)
	{
		if (node->IsRootNode())
		{
			return 0;
		}

		if (!IsOriginNode(node))
		{
			return -1;
		}

		return _wtol(wcsrchr(node->GetName(), '.') + 1);
	}

	INode* FindNodeForName(const char* name, INode* node)
	{
		if (!node)
		{
			return nullptr;
		}

		if (!name)
		{
			return nullptr;
		}

		const StringClass& w3dname = GetW3DNameFromNode(node);

		if (!strcmp(w3dname, name))
		{
			return node;
		}

		for (int i = 0; i < node->NumChildren(); i++)
		{
			INode* child = node->GetChildNode(i);
			const StringClass& childname = GetW3DNameFromNode(child);

			if (!strcmp(name, childname))
			{
				return child;
			}
		}

		for (int i = 0; i < node->NumChildren(); i++)
		{
			INode* child = FindNodeForName(name, node->GetChildNode(i));

			if (child)
			{
				return child;
			}
		}

		return nullptr;
	}

	bool UpdateLODNames(char* name, int level, INodeListClass* list)
	{
		if (!name || level < 0)
		{
			return false;
		}

		if (!list)
		{
			return false;
		}

		for (int i = 0; i < list->GetNodeCount(); i++)
		{
			if (GetLODLevelFromNode(list->GetNode(i)) != level)
			{
				if (FindNodeForName(name, list->GetNode(i)))
				{
					int len = (int)strlen(name);

					if (level < 10 && len < W3D_NAME_LEN - 1)
					{
						name[len] = level + '0';
						name[len + 1] = 0;
						return true;
					}

					if (level < 100 && len < W3D_NAME_LEN - 2)
					{
						char buf[4];
						sprintf(buf, "%d", level);
						strcat(name, buf);
						return true;
					}

					name[W3D_NAME_LEN - 2] = level + '0';
				}
			}
		}

		return true;
	}

	class GeometryExportTaskClass
	{
	public:
		char Name[W3D_NAME_LEN];
		char ContainerName[W3D_NAME_LEN];
		int BoneIndex;
		Matrix3 Transform;
		TimeValue Time;
		INode* Node;
		GeometryExportTaskClass(INode* node, LodData& lod) : BoneIndex(0), Time(lod.Time), Node(node)
		{
			//TT_PROFILER_SCOPE("GeometryExportTaskClass()");
			CopyW3DNameFromNode(Name, node);
#ifdef W3X
			CheckW3DName(Name);
#endif
			UpdateLODNames(Name, GetLODLevelFromNode(lod.Node), lod.NodeList);
			CopyW3DName(ContainerName, lod.Name);
#ifdef W3X
			CheckW3DName(ContainerName);
#endif

			if (lod.Hierarchy)
			{
				if (!HasSkin(node))
				{
					lod.Hierarchy->FindBoneForNode(Node, &BoneIndex, nullptr, &Transform);
				}
				else
				{
					BoneIndex = 0;
					Transform = lod.Transform;
				}
			}
		}

		virtual ~GeometryExportTaskClass() {};
		virtual void Save(LodData& lod) = 0;

		virtual Point3 Build_Vertex_Normal_For_Point(Point3& point, int smoothing)
		{
			return Point3(0, 0, 0);
		}

		virtual bool IsAggregate() { return false; }
		virtual bool IsProxy() { return false; }
		virtual int GetType() = 0;

		void GetSubObjectName(char* subobjname, int size)
		{
			char name[128];
			memset(name, 0, 128);

			if (ContainerName[0])
			{
				strcat(name, ContainerName);
				strcat(name, ".");
			}

			strcat(name, Name);
			strncpy(subobjname, name, size);
		}
	};

	class ShaderParam
	{
	public:
#ifdef W3X
		unsigned int ParameterHash;
#endif
		StringClass ParameterName;
		int Type;
		StringClass TextureParam;
		float FloatParam[4];
		int IntParam;
		bool BoolParam;

		ShaderParam()
		{
			Reset();
		}

		void SetType(int type)
		{
			Type = type;
		}

		void Reset()
		{
#ifdef W3X
			ParameterHash = 0;
#endif
			ParameterName.Free_String();
			Type = 0;
			TextureParam.Free_String();
			FloatParam[0] = 0;
			FloatParam[1] = 0;
			FloatParam[2] = 0;
			FloatParam[3] = 0;
			IntParam = 0;
			BoolParam = false;
		}

		void SetName(const wchar_t* name)
		{
			ParameterName = name;
#ifdef W3X
			ParameterHash = Do_CRC(ParameterName);
#endif
		}

		void SetTexture(const char* path)
		{
			TextureParam.Free_String();

			if (path)
			{
				StringClass s = path;
				char Filename[_MAX_FNAME];
				char Ext[_MAX_EXT];
				char fname[_MAX_PATH];
				_splitpath(s, nullptr, nullptr, Filename, Ext);
				_makepath(fname, NULL, NULL, Filename, Ext);
				TextureParam = fname;
			}
		}

		ShaderParam& operator=(const ShaderParam& that)
		{
			ParameterName = that.ParameterName;
			Type = that.Type;
			StringClass s = that.TextureParam;
			SetTexture(s);
			FloatParam[0] = that.FloatParam[0];
			FloatParam[1] = that.FloatParam[1];
			FloatParam[2] = that.FloatParam[2];
			FloatParam[3] = that.FloatParam[3];
			IntParam = that.IntParam;
			BoolParam = that.BoolParam;
			return *this;
		}
	};

	class W3dMaterialDescClass
	{
		class MaterialRemapClass
		{
		public:
			int MaterialIndices[4];
			int ShaderIndices[4];
			int TextureIndices[4][2];
			int UVSources[4][2];
			int FXShaderIndices[4];

			MaterialRemapClass()
			{
				for (int i = 0; i < 4; i++)
				{
					MaterialIndices[i] = -1;
					ShaderIndices[i] = -1;
					TextureIndices[i][0] = -1;
					TextureIndices[i][1] = -1;
					UVSources[i][0] = -1;
					UVSources[i][1] = -1;
					FXShaderIndices[i] = -1;
				}
			}

			bool operator == (MaterialRemapClass const& mat) const
			{
				for (int i = 0; i < 4; i++)
				{
					if (MaterialIndices[i] != mat.MaterialIndices[i] || ShaderIndices[i] != mat.ShaderIndices[i] || TextureIndices[i][0] != mat.TextureIndices[i][0] || TextureIndices[i][1] != mat.TextureIndices[i][1] || UVSources[i][0] != mat.UVSources[i][0] || UVSources[i][1] != mat.UVSources[i][1] || FXShaderIndices[i] != mat.FXShaderIndices[i])
					{
						return false;
					}
				}

				return true;
			}

			bool operator != (MaterialRemapClass const& mat) const
			{
				return !(*this == mat);
			}
		};

		class ShadeClass : public NoEqualsClass<ShadeClass>
		{
		public:
			W3dShaderStruct Shader;
			int Hash;

			ShadeClass() : Hash(0)
			{
			}
		};

		class VertMatClass : public NoEqualsClass<VertMatClass>
		{
		public:
			W3dVertexMaterialStruct Material;
			char* MapperArgs[2];
			int Pass;
			int Hash;
			char* MaterialName;

			VertMatClass() : Pass(-1), Hash(0), MaterialName(nullptr)
			{
				MapperArgs[0] = nullptr;
				MapperArgs[1] = nullptr;
			}

			~VertMatClass()
			{
				if (MaterialName)
				{
					delete[] MaterialName;
				}

				if (MapperArgs[0])
				{
					delete[] MapperArgs[0];
				}

				if (MapperArgs[1])
				{
					delete[] MapperArgs[1];
				}
			}

			VertMatClass& operator =(VertMatClass const& mat)
			{
				memcpy(&Material, &mat.Material, sizeof(Material));
				Pass = mat.Pass;
				Hash = mat.Hash;

				if (MaterialName)
				{
					delete[] MaterialName;
				}

				if (mat.MaterialName)
				{
					MaterialName = newstr(mat.MaterialName);
				}
				else
				{
					MaterialName = nullptr;
				}

				for (int i = 0; i < 2; i++)
				{
					SetMapperArgs(mat.MapperArgs[i], i);
				}

				return *this;
			}

			void SetMapperArgs(const char* args, int stage)
			{
				if (MapperArgs[stage])
				{
					delete[] MapperArgs[stage];
					MapperArgs[stage] = nullptr;
				}

				if (args)
				{
					MapperArgs[stage] = newstr(args);
				}
			}
		};

		class TexClass : public NoEqualsClass<TexClass>
		{
		public:
			char* TextureName;
			W3dTextureInfoStruct* TextureInfo;
			int Hash;

			TexClass() : TextureName(nullptr), TextureInfo(nullptr), Hash(0)
			{
			}

			~TexClass()
			{
				if (TextureName)
				{
					delete[] TextureName;
				}

				if (TextureInfo)
				{
					delete TextureInfo;
				}
			}

			TexClass& operator =(TexClass const& tex)
			{
				SetTextureFilename(tex.TextureName);
				SetTextureInfo(tex.TextureInfo);
				Hash = tex.Hash;
				return *this;
			}

			void SetTextureFilename(const char* fullpath)
			{
				if (TextureName)
				{
					delete[] TextureName;
				}

				if (fullpath)
				{
					char filename[_MAX_FNAME];
					char ext[_MAX_EXT];
					char fname[_MAX_FNAME];
					_splitpath(fullpath, nullptr, nullptr, filename, ext);
					_makepath(fname, nullptr, nullptr, filename, ext);
					TextureName = newstr(fname);
				}
				else
				{
					TextureName = nullptr;
				}
			}

			void SetTextureInfo(W3dTextureInfoStruct* info)
			{
				if (info)
				{
					if (!TextureInfo)
					{
						TextureInfo = new W3dTextureInfoStruct;
					}

					*TextureInfo = *info;
				}
				else if (TextureInfo)
				{
					delete TextureInfo;
					TextureInfo = nullptr;
				}
			}
		};

		class FXShaderClass : public NoEqualsClass<FXShaderClass>
		{
		public:
			W3dFXShaderStruct ShaderHeader;
			ShaderParam ShaderParams[64];
			int UVSource[8];
			int Hash;

			FXShaderClass() : Hash(0)
			{
			}

			FXShaderClass& operator =(FXShaderClass const& fx)
			{
				memcpy(&ShaderHeader, &fx.ShaderHeader, sizeof(ShaderHeader));

				for (int i = 0; i < 64; i++)
				{
					if (!fx.ShaderParams[i].Type)
					{
						break;
					}

					ShaderParams[i] = fx.ShaderParams[i];
				}

				memcpy(UVSource, fx.UVSource, sizeof(UVSource));
				return *this;
			}
		};

	public:
		class W3dMat
		{
			int SurfaceType;
			bool Tangents;
			int SortLevel;
			int PassCount;
			W3dShaderStruct Shaders[4];
			W3dVertexMaterialStruct* Materials[4];
			const char* MapperArgs[4][2];
			TexClass* Textures[4][2];
			W3dFXShaderStruct ShaderHeader;
			ShaderParam ShaderParams[64];
			int UVSources[4][2];

		public:
			W3dMat() : SurfaceType(0), SortLevel(0), PassCount(0), Tangents(false)
			{
				for (int i = 0; i < 4; i++)
				{
					Materials[i] = nullptr;
					W3d_Shader_Reset(&Shaders[i]);

					for (int j = 0; j < 2; j++)
					{
						MapperArgs[i][j] = nullptr;
						Textures[i][j] = nullptr;
						UVSources[i][j] = 1;
					}
				}

				memset(&ShaderHeader, 0, sizeof(ShaderHeader));

				for (int i = 0; i < 64; i++)
				{
					ShaderParams[i].Reset();
				}

				UVSources[0][0] = -10000;
				UVSources[0][1] = -10000;
				UVSources[1][0] = -10000;
				UVSources[1][1] = -10000;
				UVSources[2][0] = -10000;
				UVSources[2][1] = -10000;
				UVSources[3][0] = -10000;
				UVSources[3][1] = -10000;
			}

			~W3dMat()
			{
				Free();
			}

			void SetSurfaceType(int type)
			{
				SurfaceType = type;
			}

			void SetSortLevel(int level)
			{
				SortLevel = level;
			}

			void SetPassCount(int pass)
			{
				PassCount = pass;
			}

			void SetVertexMaterial(W3dVertexMaterialStruct* material, int pass)
			{
				if (!Materials[pass])
				{
					Materials[pass] = new W3dVertexMaterialStruct;
				}

				*Materials[pass] = *material;
			}

			void SetMapperArgs(const char* newargs, int pass, signed int stage)
			{
				const char** args = &MapperArgs[pass][stage];

				if (*args)
				{
					delete[] * args;
					*args = nullptr;
				}

				if (args)
				{
					*args = newstr(newargs);
				}
			}

			void SetShader(W3dShaderStruct* shader, int pass)
			{
				Shaders[pass] = *shader;
			}

			void SetUVSource(int pass, int stage, int source)
			{
				UVSources[pass][stage] = source;
			}

			int GetSurfaceType() { return SurfaceType; }
			int GetSortLevel() { return SortLevel; }
			int GetPassCount() { return PassCount; }
			bool NeedsTangents() { return Tangents; }
			bool HasEffectFile() { return ShaderHeader.shadername[0] != 0; }

			W3dFXShaderStruct* GetShaderHeader()
			{
				return &ShaderHeader;
			}

			ShaderParam* GetShaderParams()
			{
				return ShaderParams;
			}

			int* GetUVSources()
			{
				return (int*)UVSources;
			}

			W3dVertexMaterialStruct* GetVertexMaterial(int pass)
			{
				return Materials[pass];
			}

			const char* GetMapperArgs(int pass, signed int stage)
			{
				return MapperArgs[pass][stage];
			}

			W3dShaderStruct GetShader(int pass)
			{
				return Shaders[pass];
			}

			TexClass* GetTexture(int pass, signed int stage)
			{
				return Textures[pass][stage];
			}

			int GetUVSource(int pass, signed int stage)
			{
				return UVSources[pass][stage];
			}

			void Free()
			{
				for (int i = 0; i < 4; i++)
				{
					delete Materials[i];
					Materials[i] = nullptr;

					for (int j = 0; j < 2; j++)
					{
						delete Textures[i][j];
						Textures[i][j] = nullptr;
						delete[] MapperArgs[i][j];
						MapperArgs[i][j] = nullptr;
					}
				}
			}

			void Reset()
			{
				Free();
				SortLevel = 0;
				Tangents = false;

				for (int i = 0; i < 4; i++)
				{
					UVSources[i][0] = 1;
					UVSources[i][1] = 1;
					W3d_Shader_Reset(&Shaders[i]);
				}

				memset(&ShaderHeader, 0, sizeof(ShaderHeader));

				for (int i = 0; i < 64; i++)
				{
					ShaderParams[i].Reset();
				}

				UVSources[0][0] = -10000;
				UVSources[0][1] = -10000;
				UVSources[1][0] = -10000;
				UVSources[1][1] = -10000;
				UVSources[2][0] = -10000;
				UVSources[2][1] = -10000;
				UVSources[3][0] = -10000;
				UVSources[3][1] = -10000;
			}

			void SetTexture(TexClass* tex, int pass, int stage)
			{
				if (!Textures[pass][stage])
				{
					Textures[pass][stage] = new TexClass;
				}

				*Textures[pass][stage] = *tex;
			}

			void InitFromW3DMaterial(W3DMaterial* mtl)
			{
				Reset();
				PassCount = mtl->NumActivePasses();
				SurfaceType = mtl->GetSurfaceType();
				SortLevel = mtl->GetSortLevel();

				for (int i = 0; i < mtl->NumActivePasses(); i++)
				{
					W3DMaterialPass& pass = mtl->GetMaterialPass(i);
					W3dShaderStruct shader;
					shader.DepthCompare = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DepthCmp));
					shader.DepthMask = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::BlendWriteZBuffer));
					shader.AlphaTest = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::AlphaTest));
					shader.DestBlend = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomDestMode));
					shader.PriGradient = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PriGradient));
					shader.SecGradient = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SecGradient));
					shader.SrcBlend = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomSrcMode));
					shader.DetailColorFunc = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailColour));
					shader.DetailAlphaFunc = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailAlpha));
					shader.Texturing = W3DSHADER_TEXTURING_DISABLE;
					shader.PostDetailColorFunc = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailColour));
					shader.PostDetailAlphaFunc = pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailAlpha));
					shader.ColorMask = 0;
					shader.FogFunc = 0;
					shader.ShaderPreset = 0;
					shader.pad[0] = 0;
					W3dVertexMaterialStruct mat;
					mat.Attributes = 0;

					if (pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SpecularToDiffuse)))
					{
						mat.Attributes = W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE;
					}

					switch ((W3DMaterialMappingType)pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0Mapping)))
					{
					case W3DMaterialMappingType::Environment:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::ClassicEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::Screen:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SCREEN;
						break;
					case W3DMaterialMappingType::LinearOffset:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::Silhouette:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE;
						break;
					case W3DMaterialMappingType::Scale:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SCALE;
						break;
					case W3DMaterialMappingType::Grid:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID;
						break;
					case W3DMaterialMappingType::Rotate:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ROTATE;
						break;
					case W3DMaterialMappingType::Sine:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::Step:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::ZigZag:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::WSClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::WSEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::GridClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::GridEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::Random:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_RANDOM;
						break;
					case W3DMaterialMappingType::Edge:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_EDGE;
						break;
					case W3DMaterialMappingType::BumpEnv:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_BUMPENV;
						break;
					case W3DMaterialMappingType::GridWSClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::GridWSEnv:
						mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT;
						break;
					default:
						break;
					}

					switch ((W3DMaterialMappingType)pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1Mapping)))
					{
					case W3DMaterialMappingType::Environment:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::ClassicEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::Screen:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SCREEN;
						break;
					case W3DMaterialMappingType::LinearOffset:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::Silhouette:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE;
						break;
					case W3DMaterialMappingType::Scale:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SCALE;
						break;
					case W3DMaterialMappingType::Grid:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID;
						break;
					case W3DMaterialMappingType::Rotate:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ROTATE;
						break;
					case W3DMaterialMappingType::Sine:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::Step:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::ZigZag:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET;
						break;
					case W3DMaterialMappingType::WSClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::WSEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::GridClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::GridEnvironment:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT;
						break;
					case W3DMaterialMappingType::Random:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_RANDOM;
						break;
					case W3DMaterialMappingType::Edge:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_EDGE;
						break;
					case W3DMaterialMappingType::BumpEnv:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_BUMPENV;
						break;
					case W3DMaterialMappingType::GridWSClassicEnv:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV;
						break;
					case W3DMaterialMappingType::GridWSEnv:
						mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT;
						break;
					default:
						break;
					}

					Color ambient = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::AmbientColour));
					mat.Ambient.R = (uint8)(ambient.r * 255.0f);
					mat.Ambient.G = (uint8)(ambient.g * 255.0f);
					mat.Ambient.B = (uint8)(ambient.b * 255.0f);

					Color diffuse = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::DiffuseColour));
					mat.Diffuse.R = (uint8)(diffuse.r * 255.0f);
					mat.Diffuse.G = (uint8)(diffuse.g * 255.0f);
					mat.Diffuse.B = (uint8)(diffuse.b * 255.0f);

					Color specular = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour));
					mat.Specular.R = (uint8)(specular.r * 255.0f);
					mat.Specular.G = (uint8)(specular.g * 255.0f);
					mat.Specular.B = (uint8)(specular.b * 255.0f);

					Color emissive = pass.ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::EmissiveColour));
					mat.Emissive.R = (uint8)(emissive.r * 255.0f);
					mat.Emissive.G = (uint8)(emissive.g * 255.0f);
					mat.Emissive.B = (uint8)(emissive.b * 255.0f);

					mat.Shininess = pass.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Shininess));
					mat.Opacity = pass.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Opacity));
					mat.Translucency = pass.ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Translucency));

					for (int j = 0; j < 2; j++)
					{
						TexClass tex;

						if (!pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1TextureEnabled : W3DMaterialParamID::Stage0TextureEnabled)) || !mtl->GetSubTexmap(2 * i + j))
						{
							break;
						}

						StringClass str = ((BitmapTex*)mtl->GetSubTexmap(2 * i + j))->GetMapName();
						tex.SetTextureFilename(str);
						W3dTextureInfoStruct texinfo;
						texinfo.AnimType = pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1AnimMode : W3DMaterialParamID::Stage0AnimMode));

						if (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1Publish : W3DMaterialParamID::Stage0Publish)))
						{
							texinfo.Attributes |= W3DTEXTURE_PUBLISH;
						}

						if (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1NoLOD : W3DMaterialParamID::Stage0NoLOD)))
						{
							texinfo.Attributes |= W3DTEXTURE_NO_LOD;
						}

						if (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1ClampU : W3DMaterialParamID::Stage0ClampU)))
						{
							texinfo.Attributes |= W3DTEXTURE_CLAMP_U;
						}

						if (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1ClampV : W3DMaterialParamID::Stage0ClampV)))
						{
							texinfo.Attributes |= W3DTEXTURE_CLAMP_V;
						}

						if (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1AlphaBitmap : W3DMaterialParamID::Stage0AlphaBitmap)))
						{
							texinfo.Attributes |= W3DTEXTURE_ALPHA_BITMAP;
						}

						texinfo.Attributes |= (pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1PassHint : W3DMaterialParamID::Stage0PassHint)) << W3DTEXTURE_HINT_SHIFT);

						if (!j && pass.ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PriGradient)) == 3)
						{
							texinfo.Attributes |= W3DTEXTURE_TYPE_BUMPMAP;
						}

						texinfo.FrameCount = pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1Frames : W3DMaterialParamID::Stage0Frames));
						texinfo.FrameRate = pass.ParamBlock->GetFloat(enum_to_value(j ? W3DMaterialParamID::Stage1FPS : W3DMaterialParamID::Stage0FPS));

						if (texinfo.FrameCount > 1 || texinfo.Attributes)
						{
							tex.SetTextureInfo(&texinfo);
						}

						SetTexture(&tex, i, j);
						shader.Texturing = W3DSHADER_TEXTURING_ENABLE;
						UVSources[i][j] = pass.ParamBlock->GetInt(enum_to_value(j ? W3DMaterialParamID::Stage1MappingUVChannel : W3DMaterialParamID::Stage0MappingUVChannel));
						StringClass str2 = pass.ParamBlock->GetStr(enum_to_value(j ? W3DMaterialParamID::Stage1MappingArgs : W3DMaterialParamID::Stage0MappingArgs));

						if (str2.Get_Length())
						{
							SetMapperArgs(str2, i, j);
						}
					}

					SetShader(&shader, i);
					SetVertexMaterial(&mat, i);
				}
			}

			void GetFVFFromShader(ID3DXEffect* effect, D3DXHANDLE Technique, unsigned int* fvf, bool* NeedsTangents)
			{
				*fvf = 0;
				*NeedsTangents = false;
				int count = 0;

				if (!effect)
				{
					return;
				}

				int p = 0;
				for (D3DXHANDLE pass = effect->GetPass(Technique, 0); pass; pass = effect->GetPass(Technique, p))
				{
					D3DXPASS_DESC pass_desc;

					if (effect->GetPassDesc(pass, &pass_desc) >= 0)
					{
						D3DXSEMANTIC pSemantics[64];
						unsigned int pcount;

						if (D3DXGetShaderInputSemantics(pass_desc.pVertexShaderFunction, pSemantics, &pcount) >= 0)
						{
							for (unsigned int j = 0; j < pcount; j++)
							{
								UINT usage = pSemantics[j].Usage;

								if (usage)
								{
									if (usage != D3DDECLUSAGE_COLOR)
									{
										goto l1;
									}

									int index = pSemantics[j].UsageIndex;

									if (index)
									{
										if (index != 1)
										{
										l1:
											switch (usage)
											{
											case D3DDECLUSAGE_NORMAL:
												*fvf |= D3DFVF_NORMAL;
												break;
											case D3DDECLUSAGE_TANGENT:
											case D3DDECLUSAGE_BINORMAL:
												*NeedsTangents = true;
												break;
											case D3DDECLUSAGE_TEXCOORD:
											{
												int ccount = pSemantics[j].UsageIndex + 1;

												if (count < ccount)
												{
													count = ccount;
												}

												break;
											}
											}

											continue;
										}

										*fvf |= D3DFVF_SPECULAR;
									}
									else
									{
										*fvf |= D3DFVF_DIFFUSE;
									}
								}
								else
								{
									*fvf |= D3DFVF_XYZ;
								}
							}
						}

					}
					p++;

				}

				int c = 8;

				if (count <= 8)
				{
					c = count;
				}

				*fvf |= (c & 0xF) << 8;
			}

			void InitFromFXShaderMaterial(Mtl* mtl, IDxMaterial2* dxmtl)
			{
				WStr path;
#ifdef W3X
				WStr fname;
#endif

				for (int i = 1; i < mtl->NumParamBlocks(); i++)
				{
					IParamBlock2* block = mtl->GetParamBlock(i);

					if (block)
					{
						for (int j = 0; j < block->NumParams(); j++)
						{
							ParamID id = block->IndextoID(j);
							ParamType2 type = block->GetParameterType(id);
							WStr str = block->GetLocalName(id);

							if (type == TYPE_FILENAME && str == L"EffectFile")
							{
								path = block->GetStr(id);
#ifndef W3X
								WStr fname(path);
#else
								fname = path;
#endif
								int i1 = fname.last('/');
								int i2 = fname.last('\\');

								if (i1 <= i2)
								{
									i1 = i2;
								}

								if (i1 != -1)
								{
									fname = fname.Substr(i1 + 1, fname.Length() - i1 - 1);
								}

								StringClass s = fname;
								strncpy(ShaderHeader.shadername, s, W3D_NAME_LEN);
								ShaderHeader.shadername[W3D_NAME_LEN - 1] = 0;
							}
							else if (type == TYPE_INT && str == L"Technique")
							{
								ShaderHeader.technique = block->GetInt(id);
							}
						}
					}
				}

				LPD3DXEFFECT ppEffect = nullptr;
				D3DXMACRO pDefines[3];
				pDefines[0].Name = "_WW3D_";
				pDefines[0].Definition = "";
				pDefines[1].Name = "_WW3D_VERSION_";
				pDefines[1].Definition = "1";
				pDefines[2].Name = nullptr;
				pDefines[2].Definition = nullptr;
				FILE* f = _wfopen(path, L"r");

				if (!f)
				{
#ifndef W3X
					LogDataDialogClass::WriteLogWindow(L"Cannot find shader file! Make sure it is in the correct path for your machine: %s\n", path.ToBSTR());
					throw ErrorClass(L"Cannot find shader file! Make sure it is in the correct path for your machine: %s", path.ToBSTR());
#else
					int i;
					WStr str;

					for (i = 0; i < TheManager->GetMapDirCount(); i++)
					{
						str = TheManager->GetMapDir(i);

						if (str[str.Length() - 1] != L'\\')
						{
							str += L"\\";
						}

						str += fname;
						f = _wfopen(str, L"r");

						if (f)
						{
							break;
						}
					}

					if (i >= TheManager->GetMapDirCount())
					{
						LogDataDialogClass::WriteLogWindow(L"Cannot find shader file! Make sure it is in the correct path for your machine: %s\n", str.ToBSTR());
						throw ErrorClass(L"Cannot find shader file! Make sure it is in the correct path for your machine: %s", str.ToBSTR());
					}

					LogDataDialogClass::WriteLogWindow(L"WARNING: Shader file was only found by searching Max paths, as the original location was invalid. This might lead to using an outdated shader during export! Make sure this is the correct shader on your machine: %s\n", str.ToBSTR());
#endif
				}

				fclose(f);
				LPD3DXBUFFER ppCompilationErrors = nullptr;
				LPDIRECT3DDEVICE9 device = nullptr;
				ViewExp& view = GetCOREInterface()->GetActiveViewExp();

				if (view.IsAlive())
				{
					GraphicsWindow* gw = view.getGW();

					if (gw)
					{
						ID3D9GraphicsWindow* d3dw = (ID3D9GraphicsWindow*)gw->GetInterface(D3D9_GRAPHICS_WINDOW_INTERFACE_ID);

						if (d3dw)
						{
							device = d3dw->GetDevice();
						}
					}
				}

				HRESULT res = D3DXCreateEffectFromFile(device, path, pDefines, nullptr, 0, nullptr, &ppEffect, &ppCompilationErrors);

				if (res < 0 || !ppEffect)
				{
					LogDataDialogClass::WriteLogWindow(L"Could not compile FX shader material \"%s\".\n", path.ToBSTR());

					if (ppCompilationErrors)
					{
						LogDataDialogClass::WriteLogWindow(L"Shader errors:\n%S\n", (char*)(ppCompilationErrors->GetBufferPointer()));
					}
					else
					{
						LogDataDialogClass::WriteLogWindow(L"Unspecified shader error (%ux)\n", res);
					}

					throw ErrorClass(L"Could not compile FX shader material \"%s\". Review the export log for error message", path.ToBSTR());
				}

				if (ppCompilationErrors)
				{
					ppCompilationErrors->Release();
				}

#ifdef W3X
				D3DXEFFECT_DESC desc;
				int newtechnique = 0;
				int technique = 0;

				if (ppEffect->GetDesc(&desc) >= 0)
				{
					for (unsigned int i = 0; i < desc.Techniques; i++)
					{
						D3DXTECHNIQUE_DESC tdesc;
						D3DXHANDLE th = ppEffect->GetTechnique(i);

						if (th && ppEffect->GetTechniqueDesc(th, &tdesc) >= 0 && tdesc.Name && *tdesc.Name != '_')
						{
							if (!technique)
							{
								newtechnique = i;
							}

							technique++;
						}
					}
				}

				if (ShaderHeader.technique >= technique)
				{
					LogDataDialogClass::WriteLogWindow(L"\nWARNING: The selected shader technique index is out of range (%d, max allowed %d). Resetting it to first technique. To correct it, please reselect the technique.\n\n", ShaderHeader.technique, technique - 1);
					ShaderHeader.technique = newtechnique;
				}
#endif
				unsigned int flags;

				if (ppEffect)
				{
					GetFVFFromShader(ppEffect, ppEffect->GetTechnique(ShaderHeader.technique), &flags, &Tangents);
				}

				int uv = (flags >> 8) & 0xF;

				for (int i = 0; i < uv; i++)
				{
					UVSources[i / 2][i % 2] = i + 1;
				}

				PassCount = 1;
				ShaderParam param;
				IParamBlock2* block = mtl->GetParamBlock(0);
				ShaderParam* params = ShaderParams;
				int* uvs = (int*)UVSources;

				for (int index = 0; index < block->NumParams(); index++)
				{
					ParamID id = block->IndextoID(index);
					ParamDef& def = block->GetParamDef(id);
					ParamType2 type = block->GetParameterType(id);
					WStr name = block->GetLocalName(id);
					const wchar_t* w = def.int_name;

					if (!w)
					{
						w = L"";
					}

					WStr int_name = w;
					D3DXPARAMETER_DESC desc;
					memset(&desc, 0, sizeof(desc));
					const char* semantic = nullptr;
					D3DXHANDLE parameter = nullptr;

					if (ppEffect)
					{
						StringClass str = name;
						parameter = ppEffect->GetParameterByName(nullptr, str);

						if (parameter)
						{
							ppEffect->GetParameterDesc(parameter, &desc);
							semantic = desc.Semantic;
#ifdef W3X
							D3DXHANDLE annotation = ppEffect->GetAnnotationByName(parameter, "ExportValue");

							if (annotation)
							{
								BOOL exportvalue = TRUE;

								if (ppEffect->GetBool(annotation, &exportvalue) >= 0 && !exportvalue)
								{
									continue;
								}
							}
#endif
						}
					}

					if (type == TYPE_BITMAP)
					{
						PBBitmap* bitmap = block->GetBitmap(id);

#ifdef W3X
						if (!_wcsicmp(bitmap->bi.Name(), L"none"))
						{
							LogDataDialogClass::WriteLogWindow(L"Invalid texture name \"none\". Please check the shader material.");
							throw ErrorClass(L"Invalid texture name \"none\". Please check the shader material.");
						}
#endif

						param.Reset();
						param.SetName(int_name);
						param.SetType(CONSTANT_TYPE_TEXTURE);
#ifndef W3X
						StringClass str = bitmap->bi.Name();
						param.SetTexture(str);
#else
						char buf[_MAX_FNAME];
						StringClass str = bitmap->bi.Name();
						_splitpath(str, nullptr, nullptr, buf, nullptr);

						if (!CheckW3DName(buf))
						{
							throw ErrorClass(L"Invalid texture name. Please check the shader material.");
						}

						param.SetTexture(buf);
#endif
						* params++ = param;
					}
					else
					{
						if (type == TYPE_INT)
						{
							if (int_name.Length() > 10)
							{
								if (int_name.Substr(int_name.Length() - 10, 10) == L"mapChannel")
								{
									*uvs = block->GetInt(id);
									uvs++;
									continue;
								}
							}
						}

						switch (type)
						{
						case TYPE_INT:
							if (def.ctrl_type == TYPE_INTLISTBOX || semantic && !_stricmp(semantic, "DIRECTION"))
							{
								continue;
							}

							param.Reset();
							param.SetName(name);
							param.SetType(CONSTANT_TYPE_INT);
							param.IntParam = block->GetInt(id);
							*params = param;
							params++;
							continue;
						case TYPE_BOOL:
							param.Reset();
							param.SetName(name);
							param.SetType(CONSTANT_TYPE_BOOL);
							param.BoolParam = block->GetInt(id) != 0;
							*params = param;
							params++;
							continue;
						case TYPE_FLOAT:
							param.Reset();

							if (name.Substr(name.Length() - 3, 3) == L"__X")
							{
								param.SetName(name.Substr(0, name.Length() - 3));
								int cindex = 0;

								for (int i = index; i < block->NumParams() && cindex < 4; i++)
								{
									int id = block->IndextoID(i);

									if (block->GetParameterType(id) != TYPE_FLOAT)
									{
										break;
									}

									static const wchar_t* coords[4] = { L"__X", L"__Y", L"__Z", L"__W" };
									WStr coord = coords[cindex];
									WideStringClass s = param.ParameterName;
									WStr str(s);
									str = str + coord;

									if (!(block->GetLocalName(id) == str))
									{
										break;
									}

									param.FloatParam[cindex] = block->GetFloat(id);
									cindex++;
								}

								param.SetType(cindex + 1);
								*params = param;
								params++;
								index = index + cindex - 1;
							}
							else
							{
								param.SetName(name);
								param.SetType(CONSTANT_TYPE_FLOAT1);
								param.FloatParam[0] = block->GetFloat(id);
								*params = param;
								params++;
							}

							continue;
						case TYPE_POINT4:
						{
							int count = 4;

							if (parameter && desc.Columns > 1 && desc.Columns <= 4)
							{
								count = desc.Columns;
							}

							param.Reset();
							param.SetName(name);
							param.SetType(count + 1);
							Point4 p = block->GetPoint4(id);
							param.FloatParam[0] = p.x;
							param.FloatParam[1] = p.y;
							param.FloatParam[2] = p.z;
							param.FloatParam[3] = p.w;
							*params = param;
							params++;
							continue;
						}
						}

						if (type == TYPE_FRGBA && (!semantic || !_stricmp(semantic, "LIGHTCOLOR")))
						{
							param.Reset();
							param.SetName(name);
							param.SetType(CONSTANT_TYPE_FLOAT4);
							AColor c = block->GetAColor(id);
							param.FloatParam[0] = c.r;
							param.FloatParam[1] = c.g;
							param.FloatParam[2] = c.b;
							param.FloatParam[3] = c.a;
							*params = param;
							params++;
							continue;
						}
					}
				}

				if (ppEffect)
				{
					ppEffect->Release();
					ppEffect = nullptr;
				}
			}

			void InitFromMaxMaterial(Mtl* mtl)
			{
				Reset();

				if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
				{
					InitFromW3DMaterial((W3DMaterial*)mtl);
				}
				else
				{
					IDxMaterial2* dxmtl = (IDxMaterial2*)mtl->GetInterface(IDXMATERIAL2_INTERFACE);
					if (dxmtl)
					{
						InitFromFXShaderMaterial(mtl, dxmtl);
					}
					else
					{
						PassCount = 1;
						Texmap* map = mtl->GetSubTexmap(9);

						if (map)
						{
							if (map->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
							{
								PassCount++;
							}
						}

						TexClass tex;
						W3dShaderStruct shader;
						W3d_Shader_Reset(&shader);
						W3dVertexMaterialStruct material;

						material.Attributes = 0;
						material.Emissive.B = 0;
						material.Emissive.G = 0;
						material.Emissive.R = 0;

						Color diffuse = mtl->GetDiffuse();
						material.Diffuse.R = (unsigned int)(diffuse.r * 255.0f);
						material.Diffuse.G = (unsigned int)(diffuse.g * 255.0f);
						material.Diffuse.B = (unsigned int)(diffuse.b * 255.0f);
						material.Ambient = material.Diffuse;

						Color specular = mtl->GetSpecular(0, 0);
						material.Specular.R = (unsigned int)(specular.r * 255.0f);
						material.Specular.G = (unsigned int)(specular.g * 255.0f);
						material.Specular.B = (unsigned int)(specular.b * 255.0f);

						material.Shininess = mtl->GetShininess();
						material.Opacity = 1.0f - mtl->GetXParency();
						material.Translucency = 0.0;
						map = mtl->GetSubTexmap(1);

						if (map)
						{
							if (map->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
							{
								StringClass str = ((BitmapTex*)map)->GetMapName();
								tex.SetTextureFilename(str);
								material.Diffuse.R = 0xFF;
								material.Diffuse.G = 0xFF;
								material.Diffuse.B = 0xFF;
								shader.Texturing = W3DSHADER_TEXTURING_ENABLE;
								SetTexture(&tex, 0, 0);
							}
						}

						if (material.Opacity != 1.0f)
						{
							shader.DestBlend = W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA;
							shader.SrcBlend = W3DSHADER_SRCBLENDFUNC_SRC_ALPHA;
						}

						SetVertexMaterial(&material, 0);
						SetShader(&shader, 0);

						if (PassCount == 2)
						{
							W3d_Shader_Reset(&shader);
							shader.PriGradient = W3DSHADER_PRIGRADIENT_MODULATE;
							shader.SecGradient = W3DSHADER_SECGRADIENT_DISABLE;
							shader.DepthMask = W3DSHADER_DEPTHMASK_WRITE_DISABLE;
							shader.DestBlend = W3DSHADER_DESTBLENDFUNC_ONE;
							shader.SrcBlend = W3DSHADER_SRCBLENDFUNC_ONE;
							shader.Texturing = W3DSHADER_TEXTURING_ENABLE;
							shader.ColorMask = 0;
							shader.FogFunc = 1;

							W3d_Vertex_Material_Reset(&material);
							material.Diffuse.B = 0x80;
							material.Diffuse.G = 0x80;
							material.Diffuse.R = 0x80;
							material.Attributes = material.Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK | W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT;
							map = mtl->GetSubTexmap(9);

							if (map)
							{
								if (map->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
								{
									StringClass str = ((BitmapTex*)map)->GetMapName();
									tex.SetTextureFilename(str);
									SetTexture(&tex, 1, 0);
								}
							}

							SetVertexMaterial(&material, 1);
							SetShader(&shader, 1);
						}
					}
				}
			}
		};

	private:
		int PassCount;
		int SortLevel;
		bool NeedsTangents;
		DynamicVectorClass<MaterialRemapClass> MaterialRemaps;
		DynamicVectorClass<ShadeClass> Shaders;
		DynamicVectorClass<VertMatClass> VertexMaterials;
		DynamicVectorClass<TexClass> Textures;
		DynamicVectorClass<FXShaderClass> FXShaders;

		int GetTextureHash(TexClass* tex)
		{
			int hash = 0;

			if (tex->TextureInfo)
			{
				hash = CRC_Memory((const unsigned char*)&tex->TextureInfo->FrameRate, 4, CRC_Memory((const unsigned char*)&tex->TextureInfo->FrameCount, 4, CRC_Memory((const unsigned char*)&tex->TextureInfo->AnimType, 2, CRC_Memory((const unsigned char*)&tex->TextureInfo->Attributes, 0))));
			}

			return CRC_Stringi(tex->TextureName, hash);
		}

		int GetStringHash(const char* str, int hash)
		{
			if (!str)
			{
				return hash;
			}

			char* s = new char[strlen(str) + 1];
			const char* c;

			for (c = str; ; c++)
			{
				if (*c != ' ' && *c != '\t' && *c != '\r' && *c != '\n')
				{
					break;
				}
			}

			int i = 0;
			char* s2 = s;

			if (*c)
			{
				do
				{
					while (*c == ' ' || *c == '\t')
					{
						c++;
					}

					if (!*c)
					{
						break;
					}

					*s2 = *c;
					s2++;
					i++;
					c++;
				} while (*c);
			}

			*s2 = 0;

			if (i)
			{
				for (;;)
				{
					char s3 = *(s2-- - 1);

					if (s3 != '\r' && s3 != '\n')
					{
						break;
					}

					*s2 = 0;
					i--;
				}
			}

			int h = CRC_Memory((const unsigned char*)s, i, hash);
			delete[] s;
			return h;
		}

		int GetMaterialHash(W3dVertexMaterialStruct* material, const char* mapperargs1, const char* mapperargs2)
		{
			return GetStringHash(mapperargs2, GetStringHash(mapperargs1, CRC_Memory((const unsigned char*)&material->Translucency, 4, CRC_Memory((const unsigned char*)&material->Opacity, 4, CRC_Memory((const unsigned char*)&material->Shininess, 4, CRC_Memory((const unsigned char*)&material->Emissive, 4, CRC_Memory((const unsigned char*)&material->Specular, 4, CRC_Memory((const unsigned char*)&material->Diffuse, 4, CRC_Memory((const unsigned char*)&material->Ambient, 4, CRC_Memory((const unsigned char*)&material->Attributes, 4, 0))))))))));
		}

		int GetParamHash(ShaderParam* param, int hash)
		{
#ifndef W3X
			int newhash = GetStringHash(param->ParameterName, hash);
#else
			int newhash = CRC_Memory((const unsigned char*)&param->ParameterHash, 4, hash);
#endif
			newhash = CRC_Memory((const unsigned char*)&param->Type, 4, newhash);

			if (param->Type == CONSTANT_TYPE_TEXTURE)
			{
				newhash = GetStringHash(param->TextureParam, hash);
			}

			newhash = CRC_Memory((const unsigned char*)param->FloatParam, 16, newhash);
			newhash = CRC_Memory((const unsigned char*)&param->IntParam, 4, newhash);
			return CRC_Memory((const unsigned char*)&param->BoolParam, 1, newhash);
		}

		int GetFXShaderHash(W3dFXShaderStruct* header, ShaderParam* params, int* uvsources)
		{
			int newhash = GetStringHash(header->shadername, 0);
			newhash = CRC_Memory((const unsigned char*)&header->technique, 1, newhash);
			ShaderParam* p = params;

			while (p->Type)
			{
				newhash = GetParamHash(p, newhash);
				p++;
			}

			return CRC_Memory((const unsigned char*)uvsources, 32, newhash);
		}
	public:

		W3dMaterialDescClass()
		{
			Reset();
		}

		~W3dMaterialDescClass()
		{
		}

		void Reset()
		{
			PassCount = -1;
			SortLevel = -1;
			NeedsTangents = false;
			MaterialRemaps.Clear();
			Shaders.Clear();
			VertexMaterials.Clear();
			Textures.Clear();
		}

		int GetPassCount() { return PassCount; }
		int GetSortLevel() { return SortLevel; }
		int GetMaterialRemapCount() { return MaterialRemaps.Count(); }
		int GetShaderCount() { return Shaders.Count(); }
		int GetMaterialCount() { return VertexMaterials.Count(); }
		int GetTextureCount() { return Textures.Count(); }
		int GetFXShaderCount() { return FXShaders.Count(); }
		const char* GetVertexMaterialName(int index) { return VertexMaterials[index].MaterialName; }
		W3dVertexMaterialStruct* GetVertexMaterialInfo(int index) { return &VertexMaterials[index].Material; }
		const char* GetVertexMaterialMapperArgs(int index, int stage) { return VertexMaterials[index].MapperArgs[stage]; }
		W3dShaderStruct* GetShader(int index) { return &Shaders[index].Shader; }
		TexClass* GetTexture(int index) { return &Textures[index]; }
		W3dFXShaderStruct* GetFXShaderHeader(int index) { return &FXShaders[index].ShaderHeader; }
		ShaderParam* GetFXShaderParams(int index) { return FXShaders[index].ShaderParams; }
		int GetFXShaderUVSourceForStage(int index, int stage) { return FXShaders[index].UVSource[stage]; }
		int* GetFXShaderUVSource(int index) { return FXShaders[index].UVSource; }
		int GetMaterialIndex(int index, int pass) { return MaterialRemaps[index].MaterialIndices[pass]; }
		int GetShaderIndex(int index, int pass) { return MaterialRemaps[index].ShaderIndices[pass]; }
		int GetTextureIndex(int index, int pass, int stage) { return MaterialRemaps[index].TextureIndices[pass][stage]; }
		int GetUVSource(int index, int pass, int stage) { return MaterialRemaps[index].UVSources[pass][stage]; }
		int GetFXShaderIndex(int index, int pass) { return MaterialRemaps[index].FXShaderIndices[pass]; }
		bool HasFXShader(int index) { return MaterialRemaps[index].FXShaderIndices[0] != -1; }

		bool IsAlpha(int pass)
		{
			for (int i = 0; i < MaterialRemaps.Count(); i++)
			{
				int index = GetShaderIndex(i, pass);
				W3dShaderStruct* s;

				if (index == -1)
				{
					s = nullptr;
				}
				else
				{
					s = GetShader(index);
				}

				if (s && (s->DestBlend == W3DSHADER_DESTBLENDFUNC_SRC_ALPHA || s->DestBlend == W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA || s->SrcBlend == W3DSHADER_SRCBLENDFUNC_SRC_ALPHA || s->SrcBlend == W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA || s->AlphaTest))
				{
					return true;
				}
			}

			return false;
		}

		bool IsPassAlpha(int pass)
		{
			int p = -1;

			for (int i = 0; i < PassCount; i++)
			{
				if (IsAlpha(i))
				{
					p = i;
				}
			}

			return p == pass;
		}

		int AddVertexMaterial(W3dVertexMaterialStruct* mat, const char* mapperargs1, const char* mapperargs2, int pass, const char* name)
		{
			if (!mat)
			{
				return -1;
			}

			int hash = GetMaterialHash(mat, mapperargs1, mapperargs2);
			int i;

			for (i = 0; i < VertexMaterials.Count(); i++)
			{
				if (hash == VertexMaterials[i].Hash && pass == VertexMaterials[i].Pass)
				{
					break;
				}
			}

			if (i == VertexMaterials.Count())
			{
				VertMatClass s;
				s.Material = *mat;
				s.Hash = hash;
				s.Pass = pass;

				if (name)
				{
					s.MaterialName = newstr(name);
				}
				else
				{
					s.MaterialName = nullptr;
				}

				s.SetMapperArgs(mapperargs1, 0);
				s.SetMapperArgs(mapperargs2, 1);
				VertexMaterials.Add(s);
			}

			return i;
		}

		int AddShader(W3dShaderStruct* shader, int pass)
		{
			int hash = CRC_Memory((const unsigned char*)shader, sizeof(W3dShaderStruct), 0);
			int i;

			for (i = 0; i < Shaders.Count(); i++)
			{
				if (hash == Shaders[i].Hash)
				{
					break;
				}
			}

			if (i == Shaders.Count())
			{
				ShadeClass s;
				s.Shader = *shader;
				s.Hash = hash;
				Shaders.Add(s);
			}

			return i;
		}

		int AddTexture(TexClass* tex, int pass, int stage)
		{
			if (!tex || !tex->TextureName)
			{
				return -1;
			}

			int hash = GetTextureHash(tex);
			int i;

			for (i = 0; i < Textures.Count(); i++)
			{
				if (hash == Textures[i].Hash)
				{
					break;
				}
			}

			if (i == Textures.Count())
			{
				TexClass& s = *tex;
				s.Hash = hash;
				Textures.Add(s);
			}

			return i;
		}

		int AddMaterial(W3dMat* Material, const char* name)
		{
			if (PassCount == -1)
			{
				PassCount = Material->GetPassCount();
			}

			if (SortLevel == -1)
			{
				SortLevel = Material->GetSortLevel();
			}

			NeedsTangents = NeedsTangents || Material->NeedsTangents();

			if (Material->GetPassCount() != PassCount)
			{
				return 1;
			}

			if (Material->GetSortLevel() != SortLevel)
			{
				return 3;
			}

			MaterialRemapClass remapper;

			for (int i = 0; i < PassCount; i++)
			{
				if (Material->HasEffectFile())
				{
					remapper.FXShaderIndices[i] = AddFXShader(Material->GetShaderHeader(), Material->GetShaderParams(), Material->GetUVSources(), i);
				}
				else
				{
					remapper.MaterialIndices[i] = AddVertexMaterial(Material->GetVertexMaterial(i), Material->GetMapperArgs(i, 0), Material->GetMapperArgs(i, 1), i, name);
					W3dShaderStruct& s = Material->GetShader(i);
					remapper.ShaderIndices[i] = W3dMaterialDescClass::AddShader(&s, i);

					for (int j = 0; j < 2; j++)
					{
						remapper.TextureIndices[i][j] = AddTexture(Material->GetTexture(i, j), i, j);
						remapper.UVSources[i][j] = Material->GetUVSource(i, j);
					}
				}
			}

			MaterialRemaps.Add(remapper);
			return 0;
		}

		int AddFXShader(W3dFXShaderStruct* header, ShaderParam* params, int* uvsources, int pass)
		{
			int hash = GetFXShaderHash(header, params, uvsources);
			int i;

			for (i = 0; i < FXShaders.Count(); i++)
			{
				if (hash == FXShaders[i].Hash)
				{
					break;
				}
			}

			if (i == FXShaders.Count())
			{
				FXShaderClass effect;
				memcpy(&effect.ShaderHeader, header, sizeof(effect.ShaderHeader));

				for (int j = 0; j < 64; j++)
				{
					if (!params[j].Type)
					{
						break;
					}
					effect.ShaderParams[j] = params[j];
				}

				memcpy(&effect.UVSource, uvsources, sizeof(effect.UVSource));
				effect.Hash = hash;
				FXShaders.Add(effect);
			}

			return i;
		}

		bool GetNeedsTangents()
		{
			return NeedsTangents;
		}
	};

	class MeshBuilderClass
	{
	public:
		struct MeshStatsStruct
		{
			bool HasTexture[4][2];
			bool HasShader[4];
			bool HasVertexMaterial[4];
			bool HasFXShader[4];
			bool HasPerPolyTexture[4][2];
			bool HasPerPolyShader[4];
			bool HasPerVertexMaterial[4];
			bool HasPerPolyFXShader[4];
			bool HasDiffuseColor[4];
			bool HasSpecularColor[4];
			bool HasDiffuseIllumination[4];
			bool HasTexCoords[16][2];
			int UVSplitCount;
			int StripCount;
			int MaxStripLength;
			float AvgStripLength;

			MeshStatsStruct() : UVSplitCount(0), StripCount(0), MaxStripLength(0), AvgStripLength(0)
			{
			}

			void Reset()
			{
				for (int i = 0; i < 4; i++)
				{
					HasPerPolyShader[i] = false;
					HasPerVertexMaterial[i] = false;
					HasPerPolyFXShader[i] = false;
					HasDiffuseColor[i] = false;
					HasVertexMaterial[i] = false;
					HasShader[i] = false;
					HasFXShader[i] = false;

					for (int j = 0; j < 2; j++)
					{
						HasPerPolyTexture[i][j] = false;
						HasTexture[i][j] = false;
						HasTexCoords[i][j] = false;
						HasTexCoords[i + 4][j] = false;
						HasTexCoords[i + 8][j] = false;
						HasTexCoords[i + 12][j] = false;
					}
				}

				UVSplitCount = 0;
				StripCount = 0;
				MaxStripLength = 0;
				AvgStripLength = 0;
			}
		};

		struct WingedEdgeStruct
		{
			int MaterialIdx;
			WingedEdgeStruct* Next;
			int Vertex[2];
			int Poly[2];
		};

		struct WingedEdgePolyStruct
		{
			WingedEdgeStruct* Edge[3];
		};

		class VertClass // TODO(Mara): Split this up into hot/cold parts?
		{
		public:
			Vector3 Vertexes[2];
			Vector3 Normals[2];
			int SmGroup;
			int Id;
			int BoneIndexes[2];
			int BoneWeights[2];
			int MaterialRemapIndex;
			int MaxVertColIndex;
			Vector2 TexCoord[16][2];
			Vector3 DiffuseColor[4];
			Vector3 SpecularColor[4];
			Vector3 DiffuseIllumination[4];
			float Alpha[4];
			int VertexMaterialIndex[4];
			Vector3 Tangent;
			Vector3 Binormal;
			Vector3 CrossProduct;
			int Attribute0;
			int Attribute1;
			int SharedSmGroup;
			int UniqueIndex;
			int ShadeIndex;
			VertClass* NextHash;

			VertClass() : SmGroup(0), Id(0), MaterialRemapIndex(0), MaxVertColIndex(0), Attribute0(0), Attribute1(0), SharedSmGroup(0), UniqueIndex(0), ShadeIndex(0), NextHash(nullptr)
			{
				Reset();
			}

			void Reset()
			{
				Vertexes[0] = Vector3(0, 0, 0);
				Normals[0] = Vector3(0, 0, 0);
				Vertexes[1] = Vector3(0, 0, 0);
				Normals[1] = Vector3(0, 0, 0);
				SmGroup = 0;
				Id = 0;
				MaxVertColIndex = 0;
				MaterialRemapIndex = 0;

				for (int i = 0; i < 4; i++)
				{
					DiffuseColor[i] = Vector3(1, 1, 1);
					SpecularColor[i] = Vector3(1, 1, 1);
					DiffuseIllumination[i] = Vector3(1, 1, 1);
					Alpha[i] = 1;
					VertexMaterialIndex[i] = -1;
					TexCoord[i][0] = Vector2(0, 0);
					TexCoord[i][1] = Vector2(0, 0);
					TexCoord[i + 4][0] = Vector2(0, 0);
					TexCoord[i + 4][1] = Vector2(0, 0);
					TexCoord[i + 8][0] = Vector2(0, 0);
					TexCoord[i + 8][1] = Vector2(0, 0);
					TexCoord[i + 12][0] = Vector2(0, 0);
					TexCoord[i + 12][1] = Vector2(0, 0);
				}

				BoneIndexes[0] = 0;
				BoneIndexes[1] = 0;
				BoneWeights[0] = 100;
				BoneWeights[1] = 0;
				Attribute0 = 0;
				Attribute1 = 0;
				UniqueIndex = 0;
				ShadeIndex = 0;
				NextHash = nullptr;
			}
		};

		class FaceClass // TODO(Mara): Split this up into hot/cold parts? Figure out which members are not needed.
		{
		public:
			VertClass Verts[3];
			int SmGroup;
			int Index;
			int Attributes;
			int TextureIndex[4][2];
			int ShaderIndex[4];
			int FXShaderIndex[4];
			unsigned long SurfaceType;
			int AddIndex;
			int VertIdx[3];
			Vector3 Normal;
			float Dist;

			FaceClass() : SmGroup(0), Index(0), Attributes(0), SurfaceType(0), AddIndex(0), Dist(0)
			{
				Reset();
			}

			void Reset()
			{
				for (int i = 0; i < 3; i++)
				{
					Verts[i].Reset();
					VertIdx[i] = 0;
				}

				SmGroup = 0;
				Index = 0;
				Attributes = 0;
				SurfaceType = 0;

				for (int i = 0; i < 4; i++)
				{
					TextureIndex[i][0] = -1;
					TextureIndex[i][1] = -1;
					ShaderIndex[i] = -1;
					FXShaderIndex[i] = -1;
				}

				AddIndex = 0;
				Normal = Vector3(0, 0, 0);
				Dist = 0;
			}

			bool Is_Degenerate()
			{
				for (int i = 0; i < 3; ++i)
				{
					for (int j = i + 1; j < 3; ++j)
					{
						if (VertIdx[i] == VertIdx[j] || Verts[i].Vertexes[0] == Verts[j].Vertexes[0])
						{
							return true;
						}
					}
				}

				return false;
			}

			void Compute_Plane()
			{
				Vector3 a, b;
				const Vector3& p0 = Verts[0].Vertexes[0];
				Vector3::Subtract(Verts[1].Vertexes[0], p0, &a);
				Vector3::Subtract(Verts[2].Vertexes[0], p0, &b);
				Vector3::Cross_Product(a, b, &Normal);
				Normal.Normalize();
				Dist = Vector3::Dot_Product(Normal, p0);
			}
		};

	private:
		int State;
		int PassCount;
		int FaceCount;
		FaceClass* Faces;
		int InputVertCount;
		int VertCount;
		VertClass* Vertexes;
		int CurFace;
		WorldInfoClass* WorldInfo;
		MeshStatsStruct Stats;
		int PolyOrderPass;
		int PolyOrderStage;
		int AllocFaceCount;
		int AllocFaceGrowth;

	public:
		enum
		{
			STATE_ACCEPTING_INPUT = 0x0,
			STATE_MESH_PROCESSED = 0x1,
			MAX_PASSES = 0x4,
			MAX_STAGES = 0x2,
		};

		MeshBuilderClass(int passcount, int allocfacecount, int allocfacegrowth);
		~MeshBuilderClass();
		void Compute_Mesh_Stats();
		void Compute_Bounding_Box(Vector3* min, Vector3* max, int index);
		void Compute_Bounding_Sphere(Vector3* center, float* radius, int index);
		void Free();
		void Reset(int passcount, int allocfacecount, int allocfacegrowth);
		void Compute_Face_Normals();
		bool Verify_Face_Normals();
		void Compute_Vertex_Normals();
		void Compute_Tangents_Binormals();
		void Strip_Optimize_Mesh();
		void Grow_Face_Array();
		void Sort_Vertices();
		void Add_Face(FaceClass* face);
		void Remove_Degenerate_Faces();
		void Optimize_Mesh(bool keepnormals);
		void Build_Mesh(bool keepnormals);
		void Set_World_Info(WorldInfoClass* info) { WorldInfo = info; }
		int Get_Pass_Count() { return PassCount; }
		int Get_Vertex_Count() { return VertCount; }
		int Get_Face_Count() { return FaceCount; }
		VertClass& Get_Vertex(int i) { return Vertexes[i]; }
		FaceClass& Get_Face(int i) { return Faces[i]; }
		MeshStatsStruct& Get_Mesh_Stats() { return Stats; }
	};

	MeshBuilderClass::MeshBuilderClass(int passcount, int allocfacecount, int allocfacegrowth) : State(STATE_ACCEPTING_INPUT), PassCount(passcount), FaceCount(0), Faces(nullptr), InputVertCount(0), VertCount(0), Vertexes(nullptr), CurFace(0), WorldInfo(nullptr), PolyOrderPass(0), PolyOrderStage(0), AllocFaceCount(0), AllocFaceGrowth(0)
	{
		Reset(passcount, allocfacecount, allocfacegrowth);
	}

	MeshBuilderClass::~MeshBuilderClass()
	{
		Free();
		WorldInfo = nullptr;
	}

	void MeshBuilderClass::Compute_Mesh_Stats()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Compute_Mesh_Stats");
		Stats.Reset();
		int VertexMaterialIndex[4];
		int ShaderIndex[4];
		int FXShaderIndex[4];
		int TextureIndex[4][2];

		for (int i = 0; i < 4; i++)
		{
			VertexMaterialIndex[i] = Vertexes[0].VertexMaterialIndex[i];
			ShaderIndex[i] = Faces[0].ShaderIndex[i];
			FXShaderIndex[i] = Faces[0].FXShaderIndex[i];
			TextureIndex[i][0] = Faces[0].TextureIndex[i][0];
			TextureIndex[i][1] = Faces[0].TextureIndex[i][1];
		}

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < FaceCount; k++)
				{
					if (TextureIndex[i][j] != Faces[k].TextureIndex[i][j])
					{
						Stats.HasPerPolyTexture[i][j] = true;
						break;
					}
				}
			}

			for (int j = 0; j < 8; j++)
			{
				for (int k = 0; k < VertCount; k++)
				{
					Vector2& v = Vertexes[k].TexCoord[i][j];
					if (v.X != 0.0f || v.Y != 0.0f)
					{
						Stats.HasTexCoords[i][j] = true;
						break;
					}
				}
			}

			for (int j = 0; j < FaceCount; j++)
			{
				if (ShaderIndex[i] != Faces[j].ShaderIndex[i])
				{
					Stats.HasPerPolyShader[i] = true;
					break;
				}

				if (FXShaderIndex[i] != Faces[j].FXShaderIndex[i])
				{
					Stats.HasPerPolyFXShader[i] = true;
					break;
				}
			}

			for (int j = 0; j < VertCount; j++)
			{
				if (VertexMaterialIndex[i] != Vertexes[j].VertexMaterialIndex[i])
				{
					Stats.HasPerVertexMaterial[i] = true;
					break;
				}
			}

			for (int j = 0; j < VertCount; j++)
			{
				Vector3& v = Vertexes[j].DiffuseColor[i];
				float f = Vertexes[j].Alpha[i];
				if (v.X != 1.0f || v.Y != 1.0f || v.Z != 1.0f || f != 1.0f)
				{
					Stats.HasDiffuseColor[i] = true;
					break;
				}
			}

			for (int j = 0; j < VertCount; j++)
			{
				Vector3& v = Vertexes[j].SpecularColor[i];
				if (v.X != 1.0f || v.Y != 1.0f || v.Z != 1.0f)
				{
					Stats.HasSpecularColor[i] = true;
					break;
				}
			}

			for (int j = 0; j < VertCount; j++)
			{
				Vector3& v = Vertexes[j].DiffuseIllumination[i];
				if (v.X != 1.0f || v.Y != 1.0f || v.Z != 1.0f)
				{
					Stats.HasDiffuseIllumination[i] = true;
					break;
				}
			}

			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < FaceCount; k++)
				{
					if (Faces[k].TextureIndex[i][j] != -1)
					{
						Stats.HasTexture[i][j] = true;
						break;
					}
				}
			}

			for (int j = 0; j < FaceCount; j++)
			{
				if (Faces[j].ShaderIndex[i] != -1)
				{
					Stats.HasShader[i] = true;
					break;
				}
				if (Faces[j].FXShaderIndex[i] != -1)
				{
					Stats.HasFXShader[i] = true;
					break;
				}
			}

			for (int j = 0; j < VertCount; j++)
			{
				if (Vertexes[j].VertexMaterialIndex[i] != -1)
				{
					Stats.HasVertexMaterial[i] = true;
				}
			}
		}
	}

	void MeshBuilderClass::Compute_Bounding_Box(Vector3* min, Vector3* max, int index)
	{
		int start = 0;
		int i;

		if (index != -1)
		{
			for (i = 0; i < VertCount; i++)
			{
				if (Vertexes[i].MaterialRemapIndex == index)
				{
					start = i;
					break;
				}
			}

			if (i == VertCount)
			{
				*min = Vector3(0, 0, 0);
				*max = Vector3(0, 0, 0);
				return;
			}
		}

		*min = *max = Vertexes[start].Vertexes[0];

		for (i = start; i < VertCount; i++)
		{
			if (index == -1 || index == Vertexes[i].MaterialRemapIndex)
			{
				min->Update_Min(Vertexes[i].Vertexes[0]);
				max->Update_Max(Vertexes[i].Vertexes[0]);
			}
		}
	}

	void MeshBuilderClass::Compute_Bounding_Sphere(Vector3* center, float* radius, int index)
	{
		int start = 0;
		int i;

		if (index != -1)
		{
			for (i = 0; i < VertCount; i++)
			{
				if (Vertexes[i].MaterialRemapIndex == index)
				{
					start = i;
					break;
				}
			}

			if (i == VertCount)
			{
				*center = Vector3(0, 0, 0);
				*radius = 0;
				return;
			}
		}

		Vector3 xmin = Vertexes[start].Vertexes[0];
		Vector3 xmax = Vertexes[start].Vertexes[0];
		Vector3 ymin = Vertexes[start].Vertexes[0];
		Vector3 ymax = Vertexes[start].Vertexes[0];
		Vector3 zmin = Vertexes[start].Vertexes[0];
		Vector3 zmax = Vertexes[start].Vertexes[0];

		if (start < VertCount)
		{
			for (i = start; i < VertCount; i++)
			{
				if (index == -1 || index == Vertexes[i].MaterialRemapIndex)
				{
					if (xmin.X > Vertexes[i].Vertexes[0].X)
					{
						xmin = Vertexes[i].Vertexes[0];
					}

					if (xmax.X < Vertexes[i].Vertexes[0].X)
					{
						xmax = Vertexes[i].Vertexes[0];
					}

					if (ymin.Y > Vertexes[i].Vertexes[0].Y)
					{
						ymin = Vertexes[i].Vertexes[0];
					}

					if (ymax.Y < Vertexes[i].Vertexes[0].Y)
					{
						ymax = Vertexes[i].Vertexes[0];
					}

					if (zmin.Z > Vertexes[i].Vertexes[0].Z)
					{
						zmin = Vertexes[i].Vertexes[0];
					}

					if (zmax.Z < Vertexes[i].Vertexes[0].Z)
					{
						zmax = Vertexes[i].Vertexes[0];
					}
				}
			}
		}

		float xlen = (xmax - xmin).Length2();
		float ylen = (ymax - ymin).Length2();
		float zlen = (zmax - zmin).Length2();
		Vector3 min = xmin;
		Vector3 max = xmax;

		if (xlen < ylen)
		{
			min = ymin;
			max = ymax;
			xlen = ylen;
		}

		if (zlen > xlen)
		{
			min = zmin;
			max = zmax;
		}

		Vector3 c = (max + min) * 0.5f;
		float radsq = (max - c).Length2();
		float rad = sqrt(radsq);

		for (i = start; i < VertCount; i++)
		{
			if (index == -1 || index == Vertexes[i].MaterialRemapIndex)
			{
				float newradsq = (Vertexes[i].Vertexes[0] - c).Length2();

				if (radsq < newradsq)
				{
					float newrad = sqrt(newradsq);
					rad = (rad + newrad) * 0.5f;
					radsq = rad * rad;
					c = (Vertexes[i].Vertexes[0] * (newrad - rad) + c * rad) * (1.0f / newrad);
				}
			}
		}

		*center = c;
		*radius = rad;
	}

	void MeshBuilderClass::Free()
	{
		if (Faces)
		{
			delete[] Faces;
			Faces = nullptr;
		}

		if (Vertexes)
		{
			delete[] Vertexes;
			Vertexes = nullptr;
		}

		FaceCount = 0;
		VertCount = 0;
		AllocFaceCount = 0;
		AllocFaceGrowth = 0;
	}

	void MeshBuilderClass::Reset(int passcount, int allocfacecount, int allocfacegrowth)
	{
		Free();
		PassCount = passcount;
		AllocFaceCount = allocfacecount;
		AllocFaceGrowth = allocfacegrowth;
		Faces = new FaceClass[allocfacecount];
		CurFace = 0;
		Stats.Reset();
	}

	void MeshBuilderClass::Compute_Face_Normals()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Compute_Face_Normals");

		for (int i = 0; i < FaceCount; i++)
		{
			Faces[i].Compute_Plane();
		}
	}

	bool MeshBuilderClass::Verify_Face_Normals()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Verify_Face_Normals");
		bool b = true;

		for (int i = 0; i < FaceCount; i++)
		{
			FaceClass* f = &Faces[i];
			Vector3& v1 = Vertexes[f->VertIdx[0]].Vertexes[0];
			Vector3& v2 = Vertexes[f->VertIdx[2]].Vertexes[0] - v1;
			Vector3& v3 = Vertexes[f->VertIdx[1]].Vertexes[0] - v1;
			Vector3 v4;
			Vector3::Cross_Product(v3, v2, &v4);
			v4.Normalize();

			if ((Faces[i].Normal - v4).Length() > 0.0000001f)
			{
				b = false;
			}
		}

		return b;
	}

	void MeshBuilderClass::Compute_Vertex_Normals()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Compute_Vertex_Normals");

		for (int i = 0; i < VertCount; i++)
		{
			Vertexes[i].Normals[0] = Vector3(0, 0, 0);
		}

		for (int i = 0; i < FaceCount; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				Vertexes[Vertexes[Faces[i].VertIdx[j]].ShadeIndex].Normals[0] += Faces[i].Normal;
			}
		}

		if (WorldInfo)
		{
			if (WorldInfo->Are_Meshes_Smoothed())
			{
				for (int i = 0; i < VertCount; i++)
				{
					VertClass* v = &Vertexes[i];

					if (v->ShadeIndex == i)
					{
						Vertexes[i].Normals[0] += WorldInfo->Get_Shared_Vertex_Normal(v->Vertexes[0], v->SharedSmGroup);
					}
				}
			}
		}

		for (int i = 0; i < VertCount; i++)
		{
			Vertexes[i].Normals[0] = Vertexes[Vertexes[i].ShadeIndex].Normals[0];
			Vertexes[i].Normals[0].Normalize();
		}
	}

	void MeshBuilderClass::Compute_Tangents_Binormals()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Compute_Tangents_Binormals");

		for (int i = 0; i < VertCount; i++)
		{
			Vertexes[i].Tangent = Vector3(0, 0, 0);
			Vertexes[i].Binormal = Vector3(0, 0, 0);
		}

		for (int i = 0; i < FaceCount; i++)
		{
			VertClass& v0 = Vertexes[Faces[i].VertIdx[0]];
			VertClass& v1 = Vertexes[Faces[i].VertIdx[1]];
			VertClass& v2 = Vertexes[Faces[i].VertIdx[2]];
			Vector3 a1;
			Vector3 a2;
			a1.X = v1.Vertexes[0].X - v0.Vertexes[0].X;
			a1.Y = v1.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a1.Z = v1.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			a2.X = v2.Vertexes[0].X - v0.Vertexes[0].X;
			a2.Y = v2.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a2.Z = v2.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			Vector3 a3;
			Vector3::Cross_Product(a1, a2, &a3);

			if (fabs(a3.X) > 1.0e-12)
			{
				float f10 = 1.0f / a3.X;
				float f11 = a3.Z * f10;
				v0.Tangent.X = v0.Tangent.X - f11;
				float f12 = a3.Y * f10;
				v0.Binormal.X = v0.Binormal.X - f12;
				v1.Tangent.X = v1.Tangent.X - f11;
				v1.Binormal.X = v1.Binormal.X - f12;
				v2.Tangent.X = v2.Tangent.X - f11;
				v2.Binormal.X = v2.Binormal.X - f12;
			}

			a1.X = v1.Vertexes[0].Y - v0.Vertexes[0].Y;
			a1.Y = v1.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a1.Z = v1.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			a2.X = v2.Vertexes[0].Y - v0.Vertexes[0].Y;
			a2.Y = v2.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a2.Z = v2.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			Vector3::Cross_Product(a1, a2, &a3);

			if (fabs(a3.X) > 1.0e-12)
			{
				float f10 = 1.0f / a3.X;
				float f11 = f10 * a3.Z;
				v0.Tangent.Y = v0.Tangent.Y - f11;
				float f12 = f10 * a3.Y;
				v0.Binormal.Y = v0.Binormal.Y - f12;
				v1.Tangent.Y = v1.Tangent.Y - f11;
				v1.Binormal.Y = v1.Binormal.Y - f12;
				v2.Tangent.Y = v2.Tangent.Y - f11;
				v2.Binormal.Y = v2.Binormal.Y - f12;
			}

			a1.X = v1.Vertexes[0].Z - v0.Vertexes[0].Z;
			a1.Y = v1.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a1.Z = v1.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			a2.X = v2.Vertexes[0].Z - v0.Vertexes[0].Z;
			a2.Y = v2.TexCoord[0][0].X - v0.TexCoord[0][0].X;
			a2.Z = v2.TexCoord[0][0].Y - v0.TexCoord[0][0].Y;
			Vector3::Cross_Product(a1, a2, &a3);

			if (fabs(a3.X) > 1.0e-12)
			{
				float f10 = 1.0f / a3.X;
				float f11 = f10 * a3.Z;
				v0.Tangent.Z = v0.Tangent.Z - f11;
				float f12 = f10 * a3.Y;
				v0.Binormal.Z = v0.Binormal.Z - f12;
				v1.Tangent.Z = v1.Tangent.Z - f11;
				v1.Binormal.Z = v1.Binormal.Z - f12;
				v2.Tangent.Z = v2.Tangent.Z - f11;
				v2.Binormal.Z = v2.Binormal.Z - f12;
			}
		}

		for (int i = 0; i < VertCount; i++)
		{
			Vertexes[i].Tangent.Normalize();
			Vertexes[i].Binormal.Normalize();
			Vertexes[i].Tangent = -Vertexes[i].Tangent;
			Vector3::Cross_Product(Vertexes[i].Tangent, Vertexes[i].Binormal, &Vertexes[i].CrossProduct);
			Vertexes[i].CrossProduct.Normalize();

			if (Vertexes[i].CrossProduct * Vertexes[i].Normals[0] < 0.0f)
			{
				Vertexes[i].CrossProduct = -Vertexes[i].CrossProduct;
			}
		}
	}

	void MeshBuilderClass::Strip_Optimize_Mesh()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Strip_Optimize_Mesh");
		WingedEdgeStruct* pEdgeInfos = new WingedEdgeStruct[FaceCount * 3];
		WingedEdgeStruct* edgeHashList[512];

		memset(edgeHashList, 0, sizeof(edgeHashList));
		memset(pEdgeInfos, 0, FaceCount * 3 * sizeof(WingedEdgeStruct));

		WingedEdgePolyStruct* pEdgeFaces = new WingedEdgePolyStruct[FaceCount];
		int* pVertexIndexRemap = new int[VertCount];
		int* pIndices = new int[FaceCount];
		int* pOutputFaceTextureIndices = new int[FaceCount];
		FaceClass* pOutputFaces = new FaceClass[FaceCount];

		int i, j, k;
		int edgeInfoCount = 0;

		for (i = 0; i < FaceCount; i++)
		{
			pIndices[i] = -1;
		}

		for (i = 0; i < VertCount; i++)
		{
			pVertexIndexRemap[i] = -1;
		}

		for (i = 0; i < FaceCount; i++)
		{
			FaceClass& face = Faces[i];
			WingedEdgePolyStruct& edgeFace = pEdgeFaces[i];
			int textureIndex = face.TextureIndex[PolyOrderPass][PolyOrderStage];
			WingedEdgeStruct* pCurrentEdgeInfo = &pEdgeInfos[edgeInfoCount];

			for (j = 0; j < 3; j++)
			{
				int vertIndexA = face.VertIdx[j];
				int vertIndexB = face.VertIdx[(j + 1) % 3];

				if (vertIndexA > vertIndexB)
				{
					int temp = vertIndexA;
					vertIndexA = vertIndexB;
					vertIndexB = temp;
				}

				int hashIndex = ((vertIndexB * 119) + vertIndexA) % 512;
				WingedEdgeStruct* pEdge = edgeHashList[hashIndex];

				for (; pEdge != nullptr; pEdge = pEdge->Next)
				{
					if (pEdge->Vertex[0] == vertIndexA &&
						pEdge->Vertex[1] == vertIndexB &&
						pEdge->MaterialIdx == textureIndex)
					{
						pEdge->Poly[1] = i;
						break;
					}
				}

				if (pEdge == nullptr)
				{
					pCurrentEdgeInfo->Vertex[0] = vertIndexA;
					pCurrentEdgeInfo->Vertex[1] = vertIndexB;
					pCurrentEdgeInfo->Poly[0] = i;
					pCurrentEdgeInfo->MaterialIdx = textureIndex;
					pCurrentEdgeInfo->Poly[1] = -1;
					pCurrentEdgeInfo->Next = edgeHashList[hashIndex];
					edgeHashList[hashIndex] = pCurrentEdgeInfo;
					pEdge = pCurrentEdgeInfo;
					edgeInfoCount++;
					pCurrentEdgeInfo++;
				}

				edgeFace.Edge[j] = pEdge;
			}
		}

		int previousTextureIndex = 0;
		int newVertexIndex = 0;

		for (int outputFaceIndex = 0; outputFaceIndex < FaceCount;)
		{
			int minIndex = 0x20000000;
			int sourceFaceIndex = -1;
			int index = -1;

			//For texture indices
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < FaceCount; k++)
				{
					FaceClass& face = Faces[k];
					WingedEdgePolyStruct& edgeFace = pEdgeFaces[k];

					if (pIndices[k] != -1)
					{
						continue;
					}

					if (j == 0 && face.TextureIndex[PolyOrderPass][PolyOrderStage] != previousTextureIndex)
					{
						continue;
					}

					int count = 0;

					if (edgeFace.Edge[0]->Poly[1] >= 0)
					{
						count += newVertexIndex + 1;
					}

					if (edgeFace.Edge[1]->Poly[1] >= 0)
					{
						count += newVertexIndex + 1;
					}

					if (edgeFace.Edge[2]->Poly[1] >= 0)
					{
						count += newVertexIndex + 1;
					}

					int n = (newVertexIndex * 3) - pVertexIndexRemap[face.VertIdx[0]] - pVertexIndexRemap[face.VertIdx[1]] - pVertexIndexRemap[face.VertIdx[2]] + count;

					if (n >= minIndex)
					{
						sourceFaceIndex = index;
					}
					else
					{
						minIndex = n;
						sourceFaceIndex = k;
						index = k;
					}
				}

				if (sourceFaceIndex != -1)
				{
					break;
				}
			}

			FaceClass& sourceFace = Faces[sourceFaceIndex];
			Stats.StripCount++;
			pOutputFaceTextureIndices[outputFaceIndex] = sourceFace.TextureIndex[PolyOrderPass][PolyOrderStage];
			previousTextureIndex = sourceFace.TextureIndex[PolyOrderPass][PolyOrderStage];
			FaceClass& outputFace = pOutputFaces[outputFaceIndex];
			outputFace = sourceFace;
			bool adjacentFaceFound = false;

			for (j = 0; j < 3; j++)
			{
				if (adjacentFaceFound)
				{
					break;
				}

				WingedEdgeStruct* pEdge = pEdgeFaces[sourceFaceIndex].Edge[j];

				for (k = 0; k < 2; k++)
				{
					int faceIndex = pEdge->Poly[k];

					if (faceIndex != -1 && faceIndex != sourceFaceIndex && pIndices[faceIndex] == -1)
					{
						int firstVertexIndex = -1;

						for (i = 0; i < 3; i++)
						{
							if (outputFace.VertIdx[i] != pEdge->Vertex[0] && outputFace.VertIdx[i] != pEdge->Vertex[1])
							{
								firstVertexIndex = outputFace.VertIdx[i];
								break;
							}
						}

						for (; outputFace.VertIdx[0] != firstVertexIndex;)
						{
							int tempA = outputFace.VertIdx[0];
							outputFace.VertIdx[0] = outputFace.VertIdx[1];
							outputFace.VertIdx[1] = outputFace.VertIdx[2];
							outputFace.VertIdx[2] = tempA;

						}

						adjacentFaceFound = true;
						break;
					}
				}
			}

			//This seems useless
			if (!adjacentFaceFound)
			{
				outputFace = sourceFace;
			}

			pIndices[sourceFaceIndex] = outputFaceIndex++;

			if (pVertexIndexRemap[sourceFace.VertIdx[0]] == -1)
			{
				pVertexIndexRemap[sourceFace.VertIdx[0]] = newVertexIndex++;
			}

			if (pVertexIndexRemap[sourceFace.VertIdx[1]] == -1)
			{
				pVertexIndexRemap[sourceFace.VertIdx[1]] = newVertexIndex++;
			}

			if (pVertexIndexRemap[sourceFace.VertIdx[2]] == -1)
			{
				pVertexIndexRemap[sourceFace.VertIdx[2]] = newVertexIndex++;
			}

			WingedEdgePolyStruct& edgeFace = pEdgeFaces[sourceFaceIndex];

			if (edgeFace.Edge[0]->Poly[1] == -1 && edgeFace.Edge[1]->Poly[1] == -1 && edgeFace.Edge[2]->Poly[1] == -1)
			{
				continue;
			}

			int edgeVertIndexA = outputFace.VertIdx[1];
			int edgeVertIndexB = outputFace.VertIdx[2];
			int stripLength = 0;

			for (int faceIndex = sourceFaceIndex; faceIndex != -1;)
			{
				for (i = 0; i < 3; i++)
				{
					WingedEdgeStruct* pEdgeInfo = pEdgeFaces[faceIndex].Edge[i];

					if ((pEdgeInfo->Vertex[0] != edgeVertIndexA || pEdgeInfo->Vertex[1] != edgeVertIndexB) && (pEdgeInfo->Vertex[0] != edgeVertIndexB || pEdgeInfo->Vertex[1] != edgeVertIndexA))
					{
						continue;
					}

					bool doBreak = false;

					for (j = 0; j < 2; j++)
					{
						if (pEdgeInfo->Poly[j] > -1 && pIndices[pEdgeInfo->Poly[j]] == -1)
						{
							doBreak = true;
							break;
						}
					}

					if (doBreak)
					{
						break;
					}
				}

				if (i >= 3)
				{
					break;
				}

				faceIndex = pEdgeFaces[faceIndex].Edge[i]->Poly[j];
				FaceClass& face = Faces[faceIndex];
				int vertIndex = -1;

				for (j = 0; j < 3; j++)
				{
					if (face.VertIdx[j] != edgeVertIndexA && face.VertIdx[j] != edgeVertIndexB)
					{
						vertIndex = j;
						break;
					}
				}

				vertIndex = face.VertIdx[vertIndex];
				pOutputFaceTextureIndices[outputFaceIndex] = Faces[faceIndex].TextureIndex[PolyOrderPass][PolyOrderStage];
				FaceClass& destFace = pOutputFaces[outputFaceIndex];

				if (!(stripLength & 1))
				{
					destFace.VertIdx[0] = edgeVertIndexB;
					destFace.VertIdx[1] = edgeVertIndexA;
				}
				else
				{
					destFace.VertIdx[0] = edgeVertIndexA;
					destFace.VertIdx[1] = edgeVertIndexB;
				}

				destFace.VertIdx[2] = vertIndex;
				edgeVertIndexA = edgeVertIndexB;
				edgeVertIndexB = vertIndex;

				if (pVertexIndexRemap[vertIndex] == -1)
				{
					pVertexIndexRemap[vertIndex] = newVertexIndex++;
				}

				pIndices[faceIndex] = outputFaceIndex++;
				stripLength++;
			}

			Stats.AvgStripLength += float(stripLength + 1);

			if (stripLength + 1 > Stats.MaxStripLength)
			{
				Stats.MaxStripLength = stripLength + 1;
			}
		}

		for (i = 0; i < FaceCount; i++)
		{
			FaceClass& sourceFace = Faces[i];
			FaceClass& destFace = pOutputFaces[pIndices[i]];

			for (j = 0; j < 4; j++)
			{
				destFace.TextureIndex[j][0] = sourceFace.TextureIndex[j][0];
				destFace.TextureIndex[j][1] = sourceFace.TextureIndex[j][1];
				destFace.ShaderIndex[j] = sourceFace.ShaderIndex[j];
				destFace.FXShaderIndex[j] = sourceFace.FXShaderIndex[j];
			}

			destFace.SmGroup = sourceFace.SmGroup;
			destFace.Index = sourceFace.Index;
			destFace.Attributes = sourceFace.Attributes;
			destFace.AddIndex = sourceFace.AddIndex;
			destFace.Normal = sourceFace.Normal; //Original code copies only X, which is probably a bug
			destFace.Dist = sourceFace.Dist;
			destFace.SurfaceType = sourceFace.SurfaceType;
		}

		delete[] Faces;
		Faces = pOutputFaces;
		AllocFaceCount = FaceCount;
		delete[] pEdgeInfos;
		delete[] pEdgeFaces;
		delete[] pIndices;
		delete[] pOutputFaceTextureIndices;
		delete[] pVertexIndexRemap;
		Stats.AvgStripLength /= float(Stats.StripCount);
	}

	void MeshBuilderClass::Grow_Face_Array()
	{
		int count = AllocFaceCount;
		AllocFaceCount += AllocFaceGrowth;
		FaceClass* f = Faces;
		Faces = new FaceClass[AllocFaceCount];

		for (int i = 0; i < count; i++)
		{
			Faces[i] = f[i];
		}

		delete[] f;
	}

	int VertexSortFunc(const void* a, const void* b)
	{
		MeshBuilderClass::VertClass* v1 = (MeshBuilderClass::VertClass*)a;
		MeshBuilderClass::VertClass* v2 = (MeshBuilderClass::VertClass*)b;

		if (v1->BoneIndexes[0] < v2->BoneIndexes[0])
		{
			return -1;
		}

		if (v1->BoneIndexes[0] > v2->BoneIndexes[0])
		{
			return 1;
		}

		if (v1->BoneIndexes[1] < v2->BoneIndexes[1])
		{
			return -1;
		}

		if (v1->BoneIndexes[1] > v2->BoneIndexes[1])
		{
			return 1;
		}

		if (v1->BoneWeights[0] < v2->BoneWeights[0])
		{
			return -1;
		}

		if (v1->BoneWeights[0] > v2->BoneWeights[0])
		{
			return 1;
		}

		if (v1->BoneWeights[1] < v2->BoneWeights[1])
		{
			return -1;
		}

		if (v1->BoneWeights[1] > v2->BoneWeights[1])
		{
			return 1;
		}

		if (v1->MaterialRemapIndex < v2->MaterialRemapIndex)
		{
			return -1;
		}

		return v1->MaterialRemapIndex > v2->MaterialRemapIndex;
	}

	void MeshBuilderClass::Sort_Vertices()
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Sort_Vertices");
		qsort(Vertexes, VertCount, sizeof(VertClass), VertexSortFunc);
		int* indexes = new int[VertCount];

		for (int i = 0; i < VertCount; i++)
		{
			indexes[Vertexes[i].UniqueIndex] = i;
		}

		for (int i = 0; i < FaceCount; i++)
		{
			Faces[i].VertIdx[0] = indexes[Faces[i].VertIdx[0]];
			Faces[i].VertIdx[1] = indexes[Faces[i].VertIdx[1]];
			Faces[i].VertIdx[2] = indexes[Faces[i].VertIdx[2]];
		}

		delete[] indexes;
	}

	void MeshBuilderClass::Add_Face(FaceClass* face)
	{
		if (CurFace == AllocFaceCount)
		{
			Grow_Face_Array();
		}

		Faces[CurFace] = *face;
		Faces[CurFace].Compute_Plane();
		Faces[CurFace].AddIndex = CurFace;
		Faces[CurFace].Verts[0].SmGroup = Faces[CurFace].SmGroup;
		Faces[CurFace].Verts[1].SmGroup = Faces[CurFace].SmGroup;
		Faces[CurFace].Verts[2].SmGroup = Faces[CurFace].SmGroup;
		CurFace++;
	}

	class HasherClass
	{
	public:
		virtual bool CompareItems(void* a, void* b) = 0;
		virtual void AddItem(void* item) = 0;
		virtual int GetSize() = 0;
		virtual int GetCount() = 0;
		virtual int GetHash(int index) = 0;
	};

	template <class T> class UniqueArrayClass
	{
	public:
		class HashItem : public NoEqualsClass<HashItem>
		{
		public:
			T Item;
			int Index;
			HashItem() : Index(0)
			{
			}
		};

		DynamicVectorClass<typename UniqueArrayClass<T>::HashItem> Vector;
		int Size;
		int* Indexes;
		HasherClass* Hasher;

		UniqueArrayClass(int size, int growth, HasherClass* hasher) : Vector(size, nullptr), Hasher(hasher)
		{
			Vector.Set_Growth_Step(growth);
			Size = 1 << hasher->GetSize();
			Indexes = new int[Size];
			for (int i = 0; i < Size; i++)
			{
				Indexes[i] = -1;
			}
		}

		~UniqueArrayClass()
		{
			if (Indexes)
			{
				delete[] Indexes;
			}
		}

		int Add(T* item)
		{
			Hasher->AddItem(item);
			int count = Hasher->GetCount();
			int curhash = -1;

			for (int i = 0; i < count; i++)
			{
				int hash = Hasher->GetHash(i);

				if (hash != curhash)
				{
					for (int j = Indexes[hash]; j != -1; j = Vector[j].Index)
					{
						if (Hasher->CompareItems(&Vector[j].Item, item))
						{
							return j;
						}
					}
				}
				curhash = hash;
			}

			int index = Vector.Count();
			int hash = Hasher->GetHash(0);
			HashItem h;
			h.Item = *item;
			h.Index = Indexes[hash];
			Indexes[hash] = index;
			Vector.Add(h);
			return index;
		}
	};

	class FaceHasherClass : public HasherClass
	{
	public:
		int hash;

		virtual bool CompareItems(void* a, void* b)
		{
			MeshBuilderClass::FaceClass* f1 = (MeshBuilderClass::FaceClass*)a;
			MeshBuilderClass::FaceClass* f2 = (MeshBuilderClass::FaceClass*)b;
			return f1->VertIdx[0] == f2->VertIdx[0] && f1->VertIdx[1] == f2->VertIdx[1] && f1->VertIdx[2] == f2->VertIdx[2];
		}

		virtual void AddItem(void* item)
		{
			MeshBuilderClass::FaceClass* f = (MeshBuilderClass::FaceClass*)item;
			hash = (unsigned int)((float)f->VertIdx[2] * 27561.301f + (float)f->VertIdx[1] * 1714.3849f + (float)f->VertIdx[0] * 12345.6f) & 0x3FF;
		}

		virtual int GetSize()
		{
			return 10;
		}

		virtual int GetCount()
		{
			return 1;
		}

		virtual int GetHash(int index)
		{
			return hash;
		}
	};

	void MeshBuilderClass::Remove_Degenerate_Faces()
	{
		// TODO(Mara): This is weirdly slow, try a set or improve the hash function, make it non virtual, make it store pointers?
		TT_PROFILER_SCOPE("MeshBuilderClass::Remove_Degenerate_Faces");
		FaceHasherClass hasher;
		UniqueArrayClass<MeshBuilderClass::FaceClass> faces(FaceCount, FaceCount / 4, &hasher);

		for (int i = 0; i < FaceCount; i++)
		{
			if (!Faces[i].Is_Degenerate())
			{
				faces.Add(&Faces[i]);
			}
		}

		FaceCount = faces.Vector.Count();
		AllocFaceCount = faces.Vector.Count();
		CurFace = faces.Vector.Count();
		delete[] Faces;
		Faces = new FaceClass[AllocFaceCount];

		for (int i = 0; i < FaceCount; i++)
		{
			Faces[i] = faces.Vector[i].Item;
		}
	}

	int GetVertexID(float f1, float f2)
	{
		return (((int)f2 & 0x3F) << 6) | (int)f1 & 0x3F; // TODO(Mara): magic constants, bad hash function
	}

	class MeshOptimizerClass
	{
		int VertexCount;
		int UVSplitCount;
		MeshBuilderClass::VertClass* Vertexes;
		MeshBuilderClass::VertClass** VertexPointers;
		int Unk;
		Vector3 center;
		Vector3 extent;

	public:
		MeshOptimizerClass(int vertexcount, bool b) : VertexCount(0), UVSplitCount(0), Vertexes(nullptr), VertexPointers(nullptr), Unk(b)
		{
			Vertexes = new MeshBuilderClass::VertClass[vertexcount];
			VertexPointers = new MeshBuilderClass::VertClass * [4096]; // TODO(Mara): magic constants
			memset(VertexPointers, 0, 32768);
			center = Vector3(0, 0, 0);
			extent = Vector3(1, 1, 1);
		}

		void SetPoints(Vector3& point1, Vector3& point2)
		{
			extent = (point2 - point1) * 0.5f;
			center = (point2 + point1) * 0.5f;
		}

		void UpdateSmoothingGroup()
		{
			TT_PROFILER_SCOPE("MeshOptimizerClass::UpdateSmoothingGroup");

			for (int i = 0; i < VertexCount; i++)
			{
				if (Vertexes[i].ShadeIndex != i)
				{
					Vertexes[i].SharedSmGroup = Vertexes[Vertexes[i].ShadeIndex].SharedSmGroup;
				}
			}
		}

		MeshBuilderClass::VertClass* GetVertex(int i)
		{
			return &Vertexes[i];
		}

		int CompareVertexes(MeshBuilderClass::VertClass* v1, MeshBuilderClass::VertClass* v2)
		{
			if (v1->Id != v2->Id)
			{
				return 0;
			}

			if ((v1->Vertexes[0] - v2->Vertexes[0]).Length() > 0.0001f) // TODO(Mara): save the sqrt!
			{
				return 0;
			}

			if (Unk)
			{
				if ((v1->Normals[0] - v2->Normals[0]).Length() > 0.0001f)
				{
					return 0;
				}
			}
			else
			{
				int s1 = v1->SmGroup;
				int s2 = v2->SmGroup;

				if (!(s2 & s1) && s1 != s2)
				{
					return 0;
				}
			}

			if (v1->MaterialRemapIndex != v2->MaterialRemapIndex)
			{
				return 0;
			}

			for (int i = 0; i < 4; i++)
			{
				if (v1->DiffuseColor[i].X != v2->DiffuseColor[i].X || v1->DiffuseColor[i].Y != v2->DiffuseColor[i].Y || v1->DiffuseColor[i].Z != v2->DiffuseColor[i].Z || v1->SpecularColor[i].X != v2->SpecularColor[i].X || v1->SpecularColor[i].Y != v2->SpecularColor[i].Y || v1->SpecularColor[i].Z != v2->SpecularColor[i].Z || v1->DiffuseIllumination[i].X != v2->DiffuseIllumination[i].X || v1->DiffuseIllumination[i].Y != v2->DiffuseIllumination[i].Y || v1->DiffuseIllumination[i].Z != v2->DiffuseIllumination[i].Z || v1->Alpha[i] != v2->Alpha[i] || v1->VertexMaterialIndex[i] != v2->VertexMaterialIndex[i])
				{
					return 0;
				}
			}

			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					if (v1->TexCoord[i][j].X != v2->TexCoord[i][j].X || v1->TexCoord[i][j].Y != v2->TexCoord[i][j].Y)
					{
						UVSplitCount++;
						return 0;
					}
				}
			}

			return 1;
		}

		int MatchSmoothing(MeshBuilderClass::VertClass* v1, MeshBuilderClass::VertClass* v2)
		{
			// TODO(Mara): This should be reordered from least to most expensive
			return (v1->Vertexes[0] - v2->Vertexes[0]).Length() < 0.0001f && (v2->SmGroup & v1->SmGroup || v1->SmGroup == v2->SmGroup) && v1->Id == v2->Id;
		}

		int AddVertex(MeshBuilderClass::VertClass* vert)
		{
			// TODO(Mara): This is slow.
			int index1 = -1;
			int index2 = -1;
			float f1;

			if (fabs(extent.X) <= 0.0001f)
			{
				f1 = center.X;
			}
			else
			{
				f1 = (vert->Vertexes[0].X - center.X) / extent.X;
			}

			float f2;

			if (fabs(extent.Y) <= 0.0001f)
			{
				f2 = center.Y;
			}
			else
			{
				f2 = (vert->Vertexes[0].Y - center.Y) / extent.Y;
			}

			for (double i = f1 - 0.00009999999747378752; i < f1 + 0.00009999999747378752 + 0.0000001; i += 0.00009999999747378752)
			{
				for (double j = f2 - 0.00009999999747378752; j < f2 + 0.00009999999747378752 + 0.000001; j += 0.00009999999747378752)
				{
					int index = GetVertexID((float)i, (float)j);

					if (index != index1)
					{
						for (auto k = VertexPointers[index]; k; k = k->NextHash)
						{
							if (MatchSmoothing(vert, k) && index2 == -1)
							{
								index2 = k->UniqueIndex;
								Vertexes[index2].SharedSmGroup &= vert->SmGroup;
							}

							if (CompareVertexes(vert, k))
							{
								return k->UniqueIndex;
							}
						}
					}

					index1 = index;
				}
			}

			int count = VertexCount;
			Vertexes[VertexCount] = *vert;
			VertexCount++;
			Vertexes[count].UniqueIndex = count;

			if (index2 == -1)
			{
				Vertexes[count].ShadeIndex = count;
				Vertexes[count].SharedSmGroup = Vertexes[count].SmGroup;
			}
			else
			{
				Vertexes[count].ShadeIndex = index2;
			}

			float v1 = (vert->Vertexes[0].Y - center.Y) / extent.Y;
			float v2 = (vert->Vertexes[0].X - center.X) / extent.X;
			int index3 = GetVertexID(v2, v1);
			Vertexes[count].NextHash = VertexPointers[index3];
			VertexPointers[index3] = &Vertexes[count];
			return count;
		}

		int GetVertexCount() { return VertexCount; }
		int GetUVSplitCount() { return UVSplitCount; }
	};

	int DoFaceSort(MeshBuilderClass::FaceClass* a1, MeshBuilderClass::FaceClass* a2, int pass, int stage)
	{
		int i1 = a1->TextureIndex[pass][stage];
		int i2 = a2->TextureIndex[pass][stage];

		if (i1 < i2)
		{
			return -1;
		}

		if (i1 > i2)
		{
			return 1;
		}

		int i3 = a1->Verts[0].VertexMaterialIndex[pass];
		int i4 = a2->Verts[0].VertexMaterialIndex[pass];

		if (i3 < i4)
		{
			return -1;
		}

		return i3 > i4;
	}

	int FaceSort1(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 0, 0);
	}

	int FaceSort2(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 0, 1);
	}

	int FaceSort3(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 1, 0);
	}

	int FaceSort4(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 1, 1);
	}

	int FaceSort5(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 2, 0);
	}

	int FaceSort6(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 2, 1);
	}

	int FaceSort7(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 3, 0);
	}

	int FaceSort8(const void* v1, const void* v2)
	{
		return DoFaceSort((MeshBuilderClass::FaceClass*)v1, (MeshBuilderClass::FaceClass*)v2, 3, 1);
	}

	typedef int(__cdecl* func)(const void*, const void*);

	func FaceSortFuncs[4][2] = {
		{FaceSort1, FaceSort2},
		{FaceSort3, FaceSort4},
		{FaceSort5, FaceSort6},
		{FaceSort7, FaceSort8}
	};

	void MeshBuilderClass::Optimize_Mesh(bool keepnormals)
	{
		TT_PROFILER_SCOPE("MeshBuilderClass::Optimize_Mesh");
		MeshOptimizerClass optimizer(3 * FaceCount, !keepnormals);
		Vector3 p1 = Faces[0].Verts[0].Vertexes[0];
		Vector3 p2 = Faces[0].Verts[0].Vertexes[0];

		{
			TT_PROFILER_SCOPE("Compute Bounding Box");

			for (int i = 0; i < FaceCount; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					p1.Update_Min(Faces[i].Verts[j].Vertexes[0]); // TODO(Mara): _mm_max_ps/ss
					p2.Update_Max(Faces[i].Verts[j].Vertexes[0]);
				}
			}

			optimizer.SetPoints(p1, p2);
		}

		{
			TT_PROFILER_SCOPE("Add Vertices");

			for (int i = 0; i < FaceCount; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					Faces[i].VertIdx[j] = optimizer.AddVertex(&Faces[i].Verts[j]);
				}
			}
		}

		optimizer.UpdateSmoothingGroup();
		VertCount = optimizer.GetVertexCount();
		Vertexes = new VertClass[VertCount];

		for (int i = 0; i < VertCount; i++)
		{
			Vertexes[i] = *optimizer.GetVertex(i);
		}

		Remove_Degenerate_Faces();
		Compute_Face_Normals();

		if (keepnormals)
		{
			Compute_Vertex_Normals();
		}

		Compute_Tangents_Binormals();
		Compute_Mesh_Stats();
		Stats.UVSplitCount = optimizer.GetUVSplitCount();
		qsort(Faces, FaceCount, sizeof(FaceClass), FaceSortFuncs[PolyOrderPass][PolyOrderStage]);
		Sort_Vertices();
		Strip_Optimize_Mesh();
		Verify_Face_Normals();
	}

	void MeshBuilderClass::Build_Mesh(bool keepnormals)
	{
		State = STATE_MESH_PROCESSED;
		FaceCount = CurFace;
		Optimize_Mesh(keepnormals);
	}

	float GetMatrix3Determinant(const Matrix3& m)
	{
		return (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * m[0][0] - (m[2][2] * m[1][0] - m[1][2] * m[2][0]) * m[0][1] + (m[2][1] * m[1][0] - m[1][1] * m[2][0]) * m[0][2];
	}

	bool IsMeshHidden(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::Hide);
	}

	bool IsMeshTwoSided(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::TwoSided);
	}

	bool IsMeshCastShadow(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::Shadow);
	}

#ifndef W3X
	bool IsMeshShatterable(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::Shatter);
	}

	bool IsMeshNPatchable(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::Tangents);
	}

	bool IsMeshPrelit(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::Prelit);
	}

	bool IsMeshAlwaysDynLight(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::AlwaysDynLight);
	}

#else
	bool IsJoypadPick(INode* node)
	{
		return enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*node).GeometryFlags, W3DGeometryFlags::JoypadPick);
	}
#endif

	int GetMeshAttributes(INode* node)
	{
		int attributes = 0;

		if (HasSkin(node))
		{
			attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN;
		}
		else if (IsCameraAligned(node))
		{
			attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED;
		}
		else if (IsCameraOriented(node))
		{
			attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED;
		}
#ifndef W3X
		else if (IsCameraZOriented(node))
		{
			attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED;
		}
		else
		{
			if (IsCollidePhysical(node))
			{
				attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL;
			}

			if (IsCollideProjectile(node))
			{
				attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE;
			}

			if (IsCollideVis(node))
			{
				attributes |= W3D_MESH_FLAG_COLLISION_TYPE_VIS;
			}

			if (IsCollideCamera(node))
			{
				attributes |= W3D_MESH_FLAG_COLLISION_TYPE_CAMERA;
			}

			if (IsCollideVehicle(node))
			{
				attributes |= W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE;
			}
		}
#endif
		if (IsMeshHidden(node))
		{
			attributes |= W3D_MESH_FLAG_HIDDEN;
		}

		if (IsMeshTwoSided(node))
		{
			attributes |= W3D_MESH_FLAG_TWO_SIDED;
		}

		if (IsMeshCastShadow(node))
		{
			attributes |= W3D_MESH_FLAG_CAST_SHADOW;
		}

#ifndef W3X
		if (IsMeshShatterable(node))
		{
			attributes |= W3D_MESH_FLAG_SHATTERABLE;
		}

		if (IsMeshNPatchable(node))
		{
			attributes |= W3D_MESH_FLAG_NPATCHABLE;
		}

		if (IsMeshPrelit(node))
		{
			attributes |= W3D_MESH_FLAG_PRELIT;
		}

		if (IsMeshAlwaysDynLight(node))
		{
			attributes |= W3D_MESH_FLAG_ALWAYSDYNLIGHT;
		}

#endif
		return attributes;
	}

	class MeshSave
	{
		INode* Node;
		W3DAppDataChunk* ExportFlags;
		W3DExportSettings* ExportData;
		W3dMeshHeader3Struct Header;
		W3dMaterialDescClass Materials;
		MeshBuilderClass MeshBuilder;
		TimeValue Time;
		Matrix3 Transform;
		Matrix3 ObjectTransform;
		HierarchySave* Hierarchy;
		const char* MeshUserText;
		bool IsDetermenentNegative;
		W3dVertInfStruct* VertexInfluences;
		int* MaterialIndex;
		bool HasSmoothSkin;
#ifdef W3X
		std::vector<StringClass>* Includes;
#endif
	public:
#ifndef W3X
		MeshSave(const char* meshname, const char* containername, INode* node, Mesh* mesh, Matrix3* transform, W3DAppDataChunk* exportflags, W3DExportSettings* exportdata, HierarchySave* hierarchy, TimeValue time, MaxWorldInfoClass* info) :
			ExportData(exportdata), Node(node), ExportFlags(exportflags), MeshBuilder(1, 255, 64), Time(time), Transform(*transform), Hierarchy(hierarchy), MeshUserText(nullptr), VertexInfluences(nullptr), MaterialIndex(nullptr), HasSmoothSkin(false)
#else
		MeshSave(const char* meshname, const char* containername, INode* node, Mesh* mesh, Matrix3* transform, W3DAppDataChunk* exportflags, W3DExportSettings* exportdata, std::vector<StringClass>* includes, HierarchySave* hierarchy, TimeValue time, MaxWorldInfoClass* info) :
			Node(node), ExportFlags(exportflags), ExportData(exportdata), MeshBuilder(1, 255, 64), Time(time), Transform(*transform), Hierarchy(hierarchy), MeshUserText(nullptr), VertexInfluences(nullptr), MaterialIndex(nullptr), HasSmoothSkin(false), Includes(includes)
#endif
		{
			TT_PROFILER_SCOPE("MeshSave::MeshSave");
			TT_PROFILER_TAG("Container", containername);
			TT_PROFILER_TAG("Name", meshname);
			Mesh m(*mesh);
			Mtl* mtl = node->GetMtl();
			DWORD WireColor = node->GetWireColor();
			Matrix3 tm = Node->GetObjectTM(time);
			IsDetermenentNegative = GetMatrix3Determinant(tm) < 0.0f;
			ObjectTransform = tm * Inverse(Transform);
			ApplyTransform(m, ObjectTransform);
			memset(&Header, 0, sizeof(Header));
			CopyW3DName(Header.MeshName, meshname);
			CopyW3DName(Header.ContainerName, containername);
			Header.Version = 0x50000;
			Header.Attributes = GetMeshAttributes(node);
			LogDataDialogClass::WriteLogWindow(L"\nProcessing Mesh: %S\n", Header.MeshName);

			if (m.getNumFaces() <= 0)
			{
				throw ErrorClass(L"No Triangles in Mesh: %S", Header.MeshName);
			}

			if (m.getNumVerts() <= 0)
			{
				throw ErrorClass(L"No Vertices in Mesh: %S", Header.MeshName);
			}

			DebugPrint(L"processing materials\n");
			FindMaterials(&m, mtl);
			InitMaterials(mtl, WireColor);

			if (ExportFlags->StaticSortLevel)
			{
				Header.SortLevel = ExportFlags->StaticSortLevel;
			}

			Header.FaceChannels = W3D_FACE_CHANNEL_FACE;
			Header.VertexChannels = W3D_VERTEX_CHANNEL_LOCATION | W3D_VERTEX_CHANNEL_NORMAL;

			if ((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN && Hierarchy)
			{
				Header.VertexChannels |= W3D_VERTEX_CHANNEL_BONEID;
			}

#ifndef W3X
			if (Materials.GetNeedsTangents() || IsMeshNPatchable(Node))
#else
			if (Materials.GetNeedsTangents())
#endif
			{
				Header.VertexChannels |= (W3D_VERTEX_CHANNEL_TANGENT | W3D_VERTEX_CHANNEL_BINORMAL);
			}

			MeshBuilder.Set_World_Info(info);
			BuildMesh(&m, mtl);
			Header.NumVertices = MeshBuilder.Get_Vertex_Count();
			Header.NumTris = MeshBuilder.Get_Face_Count();
			ComputeBoundingVolumes();

			if ((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN && Hierarchy)
			{
				CalculateSkinData();
			}

			MSTR str;
			node->GetUserPropBuffer(str);

			if (str.Length())
			{
				StringClass s = str;
				MeshUserText = newstr(s);
			}
		}

		~MeshSave()
		{
			if (MeshUserText)
			{
				delete[] MeshUserText;
				MeshUserText = nullptr;
			}

			if (VertexInfluences)
			{
				delete[] VertexInfluences;
				VertexInfluences = nullptr;
			}

			if (MaterialIndex)
			{
				delete[] MaterialIndex;
				MaterialIndex = nullptr;
			}
		}

		void ComputeSkinOptimization()
		{
			int count = MeshBuilder.Get_Vertex_Count();
			int x = 0;
			int i = 0;

			if (count > 0)
			{
				do
				{
					int i1 = VertexInfluences[i].BoneIdx[0];
					int i2 = VertexInfluences[i].BoneIdx[1];
					int w1 = VertexInfluences[i].Weight[0];
					int w2 = VertexInfluences[i].Weight[1];
					if (i < count)
					{
						do
						{
							if (i1 != VertexInfluences[i].BoneIdx[0])
							{
								break;
							}
							if (i2 != VertexInfluences[i].BoneIdx[1])
							{
								break;
							}
							if (w1 != VertexInfluences[i].Weight[0])
							{
								break;
							}
							if (w2 != VertexInfluences[i].Weight[1])
							{
								break;
							}

							i++;
						} while (i < count);
					}

					x++;
				} while (i < count);

				if (x > 0)
				{
					float f1 = (float)count / (float)x;

					LogDataDialogClass::WriteLogWindow(L"Skin optimization factor: %.1f (the greater the better, less than 2.0 really bad)\n", f1);
					if (f1 < 3.0f)
					{
						WideStringClass str;
						str.Format(L"Please optimize %S skinning for better performance\nVertices do not share skinning very much", Header.MeshName);
						MessageBox(nullptr, str, L"Warning", MB_SETFOREGROUND);
					}
				}
			}
		}

#ifndef W3X
		bool SaveMeshHeader(ChunkSaveClass& csave)
		{
			return !csave.Begin_Chunk(W3DChunkType::MESH_HEADER3) || csave.Write(&Header, sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct) || !csave.End_Chunk();
		}

		bool SaveUserText(ChunkSaveClass& csave)
		{
			if (!MeshUserText)
			{
				return false;
			}

			if (csave.Begin_Chunk(W3DChunkType::MESH_USER_TEXT) && csave.Write(MeshUserText, (unsigned long)strlen(MeshUserText) + 1) == strlen(MeshUserText) + 1)
			{
				return !csave.End_Chunk();
			}

			return true;
		}

		bool SaveVertices(ChunkSaveClass& csave)
		{
			if (!csave.Begin_Chunk(W3DChunkType::VERTICES))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveVertices");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct buf;
				Vector3 v = MeshBuilder.Get_Vertex(i).Vertexes[0];
				buf.X = v.X;
				buf.Y = v.Y;
				buf.Z = v.Z;

				if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
				{
					return true;
				}
			}

			if (csave.End_Chunk())
			{
				if (!HasSmoothSkin)
				{
					return false;
				}

				if (csave.Begin_Chunk(W3DChunkType::SECONDARY_VERTICES))
				{
					for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
					{
						W3dVectorStruct buf;
						Vector3 v = MeshBuilder.Get_Vertex(i).Vertexes[1];
						buf.X = v.X;
						buf.Y = v.Y;
						buf.Z = v.Z;
						if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
						{
							return true;
						}
					}

					return !csave.End_Chunk();
				}
			}

			return true;
		}

		bool SaveVertexNormals(ChunkSaveClass& csave)
		{
			if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL))
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::VERTEX_NORMALS))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveVertexNormals");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct buf;

				if (enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*Node).GeometryFlags, W3DGeometryFlags::ZNormal))
				{
					buf.X = 0.0f;
					buf.Y = 0.0f;
					buf.Z = 1.0f;
				}
				else
				{
					Vector3 v = MeshBuilder.Get_Vertex(i).Normals[0];
					buf.X = v.X;
					buf.Y = v.Y;
					buf.Z = v.Z;
				}

				if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
				{
					return true;
				}
			}

			if (csave.End_Chunk())
			{
				if (!HasSmoothSkin)
				{
					return false;
				}

				if (csave.Begin_Chunk(W3DChunkType::SECONDARY_VERTEX_NORMALS))
				{
					for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
					{
						W3dVectorStruct buf;

						if (enum_has_flags(W3DUtilities::GetOrCreateW3DAppDataChunk(*Node).GeometryFlags, W3DGeometryFlags::ZNormal))
						{
							buf.X = 0.0f;
							buf.Y = 0.0f;
							buf.Z = 1.0f;
						}
						else
						{
							Vector3 v = MeshBuilder.Get_Vertex(i).Normals[1];
							buf.X = v.X;
							buf.Y = v.Y;
							buf.Z = v.Z;
						}

						if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
						{
							return true;
						}
					}

					return !csave.End_Chunk();
				}
			}

			return true;
		}

		bool SaveTangentsBinormals(ChunkSaveClass& csave)
		{
			if (!(Header.VertexChannels & (W3D_VERTEX_CHANNEL_TANGENT | W3D_VERTEX_CHANNEL_BINORMAL)))
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::TANGENTS))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveTangentsBinormals");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct buf;
				Vector3 v = MeshBuilder.Get_Vertex(i).Tangent;
				buf.X = v.X;
				buf.Y = v.Y;
				buf.Z = v.Z;

				if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
				{
					return true;
				}
			}

			if (!csave.End_Chunk() || !csave.Begin_Chunk(W3DChunkType::BINORMALS))
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct buf;
				Vector3 v = MeshBuilder.Get_Vertex(i).Binormal;
				buf.X = v.X;
				buf.Y = v.Y;
				buf.Z = v.Z;

				if (csave.Write(&buf, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		bool SaveVertexInfluences(ChunkSaveClass& csave)
		{
			if ((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN || !(Header.VertexChannels & W3D_VERTEX_CHANNEL_BONEID) || !VertexInfluences)
			{
				return false;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveVertexInfluences");

			if (csave.Begin_Chunk(W3DChunkType::VERTEX_INFLUENCES))
			{
				int size = MeshBuilder.Get_Vertex_Count() * sizeof(W3dVertInfStruct);

				if (csave.Write(VertexInfluences, size) == size)
				{
					return !csave.End_Chunk();
				}
			}

			return true;
		}

		bool SaveTriangles(ChunkSaveClass& csave)
		{
			if (!csave.Begin_Chunk(W3DChunkType::TRIANGLES))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveTriangles");

			for (int i = 0; i < MeshBuilder.Get_Face_Count(); i++)
			{
				W3dTriStruct tri;
				memset(&tri, 0, sizeof(tri));
				tri.Vindex[0] = MeshBuilder.Get_Face(i).VertIdx[0];
				tri.Vindex[1] = MeshBuilder.Get_Face(i).VertIdx[1];
				tri.Vindex[2] = MeshBuilder.Get_Face(i).VertIdx[2];
				tri.Attributes = MeshBuilder.Get_Face(i).SurfaceType;
				tri.Normal.X = MeshBuilder.Get_Face(i).Normal.X;
				tri.Normal.Y = MeshBuilder.Get_Face(i).Normal.Y;
				tri.Normal.Z = MeshBuilder.Get_Face(i).Normal.Z;
				tri.Dist = MeshBuilder.Get_Face(i).Dist;

				if (csave.Write(&tri, sizeof(W3dTriStruct)) != sizeof(W3dTriStruct))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		bool SaveVertexShadeIndices(ChunkSaveClass& csave)
		{
			if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL))
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::VERTEX_SHADE_INDICES))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveVertexShadeIndices");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				int shade = MeshBuilder.Get_Vertex(i).ShadeIndex;

				if (csave.Write(&shade, sizeof(shade)) != sizeof(shade))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		bool SaveMaterialInfo(ChunkSaveClass& csave)
		{
			W3dMaterialInfoStruct buf;

			if (csave.Begin_Chunk(W3DChunkType::MATERIAL_INFO) && (buf.PassCount = Materials.GetPassCount(), buf.VertexMaterialCount = Materials.GetMaterialCount(), buf.ShaderCount = Materials.GetShaderCount(), buf.TextureCount = Materials.GetTextureCount(), csave.Write(&buf, sizeof(W3dMaterialInfoStruct)) == sizeof(W3dMaterialInfoStruct)))
			{
				return !csave.End_Chunk();
			}
			else
			{
				return true;
			}
		}

		bool SaveVertexMaterials(ChunkSaveClass& csave)
		{
			if (Materials.GetMaterialCount() <= 0)
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::VERTEX_MATERIALS))
			{
				return true;
			}

			for (int i = 0; i < Materials.GetMaterialCount(); i++)
			{
				csave.Begin_Chunk(W3DChunkType::VERTEX_MATERIAL);
				auto name = Materials.GetVertexMaterialName(i);

				if (name)
				{
					csave.Begin_Chunk(W3DChunkType::VERTEX_MATERIAL_NAME);

					if (csave.Write(name, (unsigned long)strlen(name) + 1) != strlen(name) + 1)
					{
						return true;
					}

					csave.End_Chunk();
				}

				auto info = Materials.GetVertexMaterialInfo(i);
				csave.Begin_Chunk(W3DChunkType::VERTEX_MATERIAL_INFO);

				if (csave.Write(info, sizeof(W3dVertexMaterialStruct)) != sizeof(W3dVertexMaterialStruct))
				{
					return true;
				}

				csave.End_Chunk();

				auto mapperargs1 = Materials.GetVertexMaterialMapperArgs(i, 0);

				if (mapperargs1)
				{
					csave.Begin_Chunk(W3DChunkType::VERTEX_MAPPER_ARGS0);

					if (csave.Write(mapperargs1, (unsigned long)strlen(mapperargs1) + 1) != strlen(mapperargs1) + 1)
					{
						return true;
					}

					csave.End_Chunk();
				}

				auto mapperargs2 = Materials.GetVertexMaterialMapperArgs(i, 1);

				if (mapperargs2)
				{
					csave.Begin_Chunk(W3DChunkType::VERTEX_MAPPER_ARGS1);

					if (csave.Write(mapperargs2, (unsigned long)strlen(mapperargs2) + 1) != strlen(mapperargs2) + 1)
					{
						return true;
					}

					csave.End_Chunk();
				}

				csave.End_Chunk();
			}

			return !csave.End_Chunk();
		}

		bool SaveTextures(ChunkSaveClass& csave)
		{
			if (Materials.GetTextureCount() <= 0)
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::TEXTURES))
			{
				return true;
			}

			for (int i = 0; i < Materials.GetTextureCount(); i++)
			{
				auto tex = Materials.GetTexture(i);
				csave.Begin_Chunk(W3DChunkType::TEXTURE);
				csave.Begin_Chunk(W3DChunkType::TEXTURE_NAME);

				if (csave.Write(tex->TextureName, (unsigned long)strlen(tex->TextureName) + 1) != strlen(tex->TextureName) + 1)
				{
					return true;
				}

				csave.End_Chunk();

				if (tex->TextureInfo)
				{
					csave.Begin_Chunk(W3DChunkType::TEXTURE_INFO);

					if (csave.Write(tex->TextureInfo, sizeof(W3dTextureInfoStruct)) != sizeof(W3dTextureInfoStruct))
					{
						return true;
					}

					csave.End_Chunk();
				}

				csave.End_Chunk();
			}

			return !csave.End_Chunk();
		}

		bool SaveMaterialIDs(ChunkSaveClass& csave, int pass)
		{
			if (!csave.Begin_Chunk(W3DChunkType::VERTEX_MATERIAL_IDS))
			{
				return true;
			}

			if (MeshBuilder.Get_Mesh_Stats().HasPerVertexMaterial[pass])
			{
				TT_PROFILER_SCOPE("MeshSave::SaveMaterialIDs");

				for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
				{
					int id = MeshBuilder.Get_Vertex(i).VertexMaterialIndex[pass];

					if (csave.Write(&id, sizeof(id)) != sizeof(id))
					{
						return true;
					}
				}

				return !csave.End_Chunk();
			}

			int id = MeshBuilder.Get_Vertex(0).VertexMaterialIndex[pass];

			if (csave.Write(&id, sizeof(id)) == sizeof(id))
			{
				return !csave.End_Chunk();
			}

			return true;
		}

		bool SaveShaderIDs(ChunkSaveClass& csave, int pass)
		{
			if (!csave.Begin_Chunk(W3DChunkType::SHADER_IDS))
			{
				return true;
			}

			if (MeshBuilder.Get_Mesh_Stats().HasPerPolyShader[pass])
			{
				TT_PROFILER_SCOPE("MeshSave::SaveShaderIDs");

				for (int i = 0; i < MeshBuilder.Get_Face_Count(); i++)
				{
					int id = MeshBuilder.Get_Face(i).ShaderIndex[pass];

					if (csave.Write(&id, sizeof(id)) != sizeof(id))
					{
						return true;
					}
				}

				return !csave.End_Chunk();
			}

			int id = MeshBuilder.Get_Face(0).ShaderIndex[pass];

			if (csave.Write(&id, sizeof(id)) == sizeof(id))
			{
				return !csave.End_Chunk();
			}

			return true;
		}

		bool SaveFXShaderIDs(ChunkSaveClass& csave, int pass)
		{
			if (!csave.Begin_Chunk(W3DChunkType::FXSHADER_IDS))
			{
				return true;
			}

			if (MeshBuilder.Get_Mesh_Stats().HasPerPolyFXShader[pass])
			{
				TT_PROFILER_SCOPE("MeshSave::SaveFXShaderIDs");

				for (int i = 0; i < MeshBuilder.Get_Face_Count(); i++)
				{
					int id = MeshBuilder.Get_Face(i).FXShaderIndex[pass];

					if (csave.Write(&id, sizeof(id)) != sizeof(id))
					{
						return true;
					}
				}

				return !csave.End_Chunk();
			}

			int id = MeshBuilder.Get_Face(0).FXShaderIndex[pass];

			if (csave.Write(&id, sizeof(id)) == sizeof(id))
			{
				return !csave.End_Chunk();
			}

			return true;
		}

		bool SaveDCG(ChunkSaveClass& csave, int pass)
		{
			if (!csave.Begin_Chunk(W3DChunkType::DCG))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveDCG");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dRGBAStruct rgba;
				rgba.R = (uint8)(MeshBuilder.Get_Vertex(i).DiffuseColor[pass].X * 255.0f);
				rgba.G = (uint8)(MeshBuilder.Get_Vertex(i).DiffuseColor[pass].Y * 255.0f);
				rgba.B = (uint8)(MeshBuilder.Get_Vertex(i).DiffuseColor[pass].Z * 255.0f);
				rgba.A = (uint8)(MeshBuilder.Get_Vertex(i).Alpha[pass] * 255.0f);

				if (csave.Write(&rgba, sizeof(W3dRGBAStruct)) != sizeof(W3dRGBAStruct))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		bool SaveTextureIDs(ChunkSaveClass& csave, int pass, int stage)
		{
			if (!csave.Begin_Chunk(W3DChunkType::TEXTURE_IDS))
			{
				return true;
			}

			if (MeshBuilder.Get_Mesh_Stats().HasPerPolyTexture[pass][stage])
			{
				TT_PROFILER_SCOPE("MeshSave::SaveTextureIDs");

				for (int i = 0; i < MeshBuilder.Get_Face_Count(); i++)
				{
					int id = MeshBuilder.Get_Face(i).TextureIndex[pass][stage];

					if (csave.Write(&id, sizeof(id)) != sizeof(id))
					{
						return true;
					}
				}

				return !csave.End_Chunk();
			}

			int id = MeshBuilder.Get_Face(0).TextureIndex[pass][stage];

			if (csave.Write(&id, sizeof(id)) == sizeof(id))
			{
				return !csave.End_Chunk();
			}

			return true;
		}

		bool SaveStageTexcoords(ChunkSaveClass& csave, int pass, int stage)
		{
			if (!MeshBuilder.Get_Mesh_Stats().HasTexCoords[pass][stage])
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::STAGE_TEXCOORDS))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveStageTexcoords");

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dTexCoordStruct buf;
				buf.U = MeshBuilder.Get_Vertex(i).TexCoord[pass][stage].X;
				buf.V = MeshBuilder.Get_Vertex(i).TexCoord[pass][stage].Y;

				if (csave.Write(&buf, sizeof(W3dTexCoordStruct)) != sizeof(W3dTexCoordStruct))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		int GenerateAABTree(ChunkSaveClass& csave, bool new_format)
		{
			TT_PROFILER_SCOPE("MeshSave::GenerateAABTree");
			int facecount = MeshBuilder.Get_Face_Count();

			if (facecount >= 8 && (Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL)
			{
				int vertcount = MeshBuilder.Get_Vertex_Count();
				Vector3* verts = new Vector3[vertcount];
				TriIndex* polys = new TriIndex[facecount];

				for (int i = 0; i < vertcount; i++)
				{
					verts[i] = MeshBuilder.Get_Vertex(i).Vertexes[0];
				}

				for (int i = 0; i < facecount; i++)
				{
					polys[i].I = MeshBuilder.Get_Face(i).VertIdx[0];
					polys[i].J = MeshBuilder.Get_Face(i).VertIdx[1];
					polys[i].K = MeshBuilder.Get_Face(i).VertIdx[2];
				}

				AABTreeBuilderClass builder;
				builder.Build_AABTree(facecount, polys, vertcount, verts, new_format);
				builder.Export(csave);
				delete[] verts;
				delete[] polys;
			}

			return 0;
		}
#endif

		int FindMaterials(Mesh* mesh, Mtl* material)
		{
			if (material && material->NumSubMtls() > 1)
			{
				TT_PROFILER_SCOPE("MeshSave::FindMaterials");
				int count = material->NumSubMtls();
				MaterialIndex = new int[count];

				for (int i = 0; i < count; i++)
				{
					MaterialIndex[i] = -1;
				}

				for (int i = 0; i < mesh->numFaces; i++)
				{
					MaterialIndex[HIWORD(mesh->faces[i].flags) % count] = 1;
				}

				int val = 0;

				for (int i = 0; i < count; i++)
				{
					int* index = &MaterialIndex[i];

					if (*index == 1)
					{
						*index = val++;
					}
				}

				return val;
			}
			else
			{
				MaterialIndex = new int[1];
				MaterialIndex[0] = 0;
				return 1;
			}
		}

		void InitMaterials(Mtl* material, int color)
		{
			TT_PROFILER_SCOPE("MeshSave::InitMaterials");

			if (!material)
			{
				Header.NumMaterials = 1;
				W3dMaterialDescClass::W3dMat mat;
				W3dVertexMaterialStruct vertmat;
				W3d_Vertex_Material_Reset(&vertmat);
				W3dShaderStruct shader;
				W3d_Shader_Reset(&shader);
				vertmat.Diffuse.R = GetRValue(color);
				vertmat.Diffuse.G = GetGValue(color);
				vertmat.Diffuse.B = GetBValue(color);
				mat.SetPassCount(1);
				mat.SetVertexMaterial(&vertmat, 0);
				mat.SetShader(&shader, 0);
				Materials.AddMaterial(&mat, "WireColor");
			}
			else if (!material->IsMultiMtl())
			{
				Header.NumMaterials = 1;
				W3dMaterialDescClass::W3dMat mat;
				mat.InitFromMaxMaterial(material);
				StringClass str = material->GetName();
				Materials.AddMaterial(&mat, str);
			}
			else
			{
				Header.NumMaterials = material->NumSubMtls();
				W3dMaterialDescClass::W3dMat mat;

				for (unsigned int i = 0; i < Header.NumMaterials; i++)
				{
					if (MaterialIndex[i] != -1)
					{
						mat.InitFromMaxMaterial(material->GetSubMtl(i));
						StringClass str = material->GetSubMtl(i)->GetName();
						int ret = Materials.AddMaterial(&mat, str);
						static wchar_t buf[512];

						switch (ret) {
						case 1:
							swprintf(buf, 512, L"Exporting Materials for Mesh: %S\nMaterial %s has %d passes.\nThe other materials have %d passes.\nAll Materials must have the same number of passes.\n", Header.MeshName, material->GetSubMtl(i)->GetName().data(), Materials.GetPassCount(), mat.GetPassCount());
							LogDataDialogClass::WriteLogWindow(buf);
							throw ErrorClass(buf);
						case 3:
							swprintf(buf, 512, L"Exporting Materials for Mesh: %S\nMaterial %s does not have the same Static Sort Level as other materials used on the mesh.\nAll materials for a mesh must use the same Static Sort Level value.\n", Header.MeshName, material->GetSubMtl(i)->GetName().data());
							LogDataDialogClass::WriteLogWindow(buf);
							throw ErrorClass(buf);
						}
					}
				}
			}

			Header.SortLevel = Materials.GetSortLevel();
		}

		void ComputeBoundingVolumes()
		{
			TT_PROFILER_SCOPE("MeshSave::ComputeBoundingVolumes");
			Vector3 min;
			Vector3 max;
			Vector3 center;
			float radius;
			MeshBuilder.Compute_Bounding_Box(&min, &max, -1);
			MeshBuilder.Compute_Bounding_Sphere(&center, &radius, -1);
			Header.SphCenter.X = center.X;
			Header.SphCenter.Y = center.Y;
			Header.SphCenter.Z = center.Z;
			Header.SphRadius = radius;
			Header.Min.X = min.X;
			Header.Min.Y = min.Y;
			Header.Min.Z = min.Z;
			Header.Max.X = max.X;
			Header.Max.Y = max.Y;
			Header.Max.Z = max.Z;
		}

		void ApplyTransform(Mesh& mesh, Matrix3& tm)
		{
			TT_PROFILER_SCOPE("MeshSave::ApplyTransform");

			for (int i = 0; i < mesh.numVerts; i++)
			{
				mesh.verts[i] = mesh.verts[i] * tm;
			}

			mesh.buildNormals();
		}

		void BuildMesh(Mesh* mesh, Mtl* material)
		{
			TT_PROFILER_SCOPE("MeshSave::BuildMesh");
			TT_PROFILER_SCOPE_START("Prepare Data");
			MeshBuilder.Reset(1, mesh->numFaces, mesh->numFaces / 3);
#ifndef W3X
			float* AlphaModifierData;

			if (mesh->vDataSupport(10))
			{
				AlphaModifierData = (float*)mesh->vData[10].data;
			}
			else
			{
				AlphaModifierData = nullptr;
			}
#else

			Point3* AlphaVertex = nullptr;
			TVFace* AlphaFace = nullptr;

			if (mesh->mapSupport(MAP_ALPHA))
			{
				AlphaVertex = mesh->mapVerts(MAP_ALPHA);
				AlphaFace = mesh->mapFaces(MAP_ALPHA);
			}
#endif

			Modifier* skinMod = FindSkinModifier(Node);
			ISkin* skin = nullptr;
			ISkinContextData* context = nullptr;

			if (skinMod)
			{
				skin = (ISkin*)skinMod->GetInterface(I_SKIN);

				if (skin)
				{
					context = skin->GetContextInterface(Node);
				}
			}

			bool hasskin = false;

			if ((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN)
			{
				if (Hierarchy)
				{
					if (skin && context)
					{
						hasskin = true;
					}
				}
			}

			bool keepnormals = enum_has_flags(ExportFlags->GeometryFlags, W3DGeometryFlags::KeepNml) == 0;
			auto normalspec = mesh->GetSpecifiedNormals();

			if (!normalspec)
			{
				keepnormals = true;
			}

			if (hasskin)
			{
				keepnormals = true;
			}

			MeshBuilderClass::FaceClass face;

			for (int i = 0; i < mesh->numFaces; i++)
			{
				Face f = mesh->faces[i];
				int index = 0;

				if (Header.NumMaterials)
				{
					index = MaterialIndex[(f.flags >> 16) % Header.NumMaterials];
				}

				for (int j = 0; j < Materials.GetPassCount(); j++)
				{
					face.ShaderIndex[j] = Materials.GetShaderIndex(index, j);
					face.FXShaderIndex[j] = Materials.GetFXShaderIndex(index, j);
					face.TextureIndex[j][0] = Materials.GetTextureIndex(index, j, 0);
					face.TextureIndex[j][1] = Materials.GetTextureIndex(index, j, 1);
				}

				face.SmGroup = f.smGroup;
				face.Index = i;
				face.Attributes = 0;
				face.SurfaceType = 0;
				Mtl* mat = material;

				if (material)
				{
					if (material->NumSubMtls() > 1)
					{
						mat = material->GetSubMtl(HIWORD(f.flags) % material->NumSubMtls());
					}

					if (mat)
					{
						if (mat->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
						{
							face.SurfaceType = ((W3DMaterial*)mat)->GetSurfaceType();
						}
					}
				}

				int corner = 0;

				for (int j = 2; j >= 0; j--)
				{
					MeshBuilderClass::VertClass* vert = &face.Verts[corner];
					int vidx;

					if (IsDetermenentNegative)
					{
						vidx = j;
					}
					else
					{
						vidx = corner;
					}

					int id = f.v[vidx];
					vert->Id = id;
					vert->Vertexes[0].X = mesh->verts[id].x;
					vert->Vertexes[0].Y = mesh->verts[id].y;
					vert->Vertexes[0].Z = mesh->verts[id].z;

#ifndef W3X
					if (AlphaModifierData != nullptr)
					{
						for (int k = 0; k < Materials.GetPassCount(); k++)
						{
							if (Materials.IsAlpha(k))
							{
								vert->Alpha[k] = AlphaModifierData[id] * 0.01f;
							}
						}
					}
#else

					if (AlphaVertex)
					{
						if (AlphaFace)
						{
							DWORD av = AlphaFace[i].t[vidx];

							for (int p = 0; p < Materials.GetPassCount(); p++)
							{
								face.Verts[0].Alpha[p] = AlphaVertex[av].x;
							}
						}
					}
#endif

					if (normalspec)
					{
						Point3& p = normalspec->GetNormal(i, corner);
						vert->Normals[0].X = p.x;
						vert->Normals[0].Y = p.y;
						vert->Normals[0].Z = p.z;
						vert->Normals[0].Normalize();
					}

					for (int pass = 0; pass < Materials.GetPassCount(); pass++)
					{
						for (int stage = 0; stage < 8; stage++)
						{
							vert->TexCoord[pass][stage].X = 0;
							vert->TexCoord[pass][stage].Y = 0;
							int uv;

							if (Materials.HasFXShader(index))
							{
								int shaderindex = Materials.GetFXShaderIndex(index, pass);

								if (shaderindex == -1)
								{
									continue;
								}

								uv = Materials.GetFXShaderUVSourceForStage(shaderindex, stage);

								if (uv == -10000)
								{
									continue;
								}
							}
							else
							{
								if (stage >= 2)
								{
									continue;
								}

								uv = Materials.GetUVSource(index, pass, stage);
							}

							UVVert* uvverts = mesh->mapVerts(uv);
							TVFace* tvfaces = mesh->mapFaces(uv);

							if (uvverts && tvfaces)
							{
								vert->TexCoord[pass][stage].X = uvverts[tvfaces[i].t[vidx]].x;
								vert->TexCoord[pass][stage].Y = uvverts[tvfaces[i].t[vidx]].y;
							}
						}
					}

					if (mesh->vcFace)
					{
						int vcidx = mesh->vcFace[i].t[vidx];
						VertColor vc = mesh->vertCol[vcidx];

						if (enum_has_flags(ExportFlags->GeometryFlags, W3DGeometryFlags::VAlpha) == 0)
						{
							vert->DiffuseColor[0].X = vc.x;
							vert->DiffuseColor[0].Y = vc.y;
							vert->DiffuseColor[0].Z = vc.z;
						}
						else
						{
							float alpha = (vc.x + vc.y + vc.z) * 0.33333334f;

							for (int k = 0; k < Materials.GetPassCount(); k++)
							{
								if (Materials.IsAlpha(k))
								{
									vert->Alpha[k] = alpha;
								}
							}
						}

						vert->MaxVertColIndex = vcidx;
					}
					else
					{
						vert->MaxVertColIndex = 0;
					}

					for (int k = 0; k < Materials.GetPassCount(); k++)
					{
						vert->VertexMaterialIndex[k] = Materials.GetMaterialIndex(index, k);
					}

					vert->MaterialRemapIndex = index;
					vert->Attribute1 = i;
					vert->Attribute0 = id;
					vert->BoneIndexes[0] = 0;
					vert->BoneIndexes[1] = 0;
					vert->BoneWeights[0] = 100;
					vert->BoneWeights[1] = 0;

					if (hasskin)
					{
						if (context->GetNumAssignedBones(id) > 2)
						{
							MessageBox(nullptr, L"Warning: Vertex is weighted to more than 2 bones", L"Warning", 0);
						}

						for (int k = 0; k < 2; k++)
						{
							if (k < context->GetNumAssignedBones(id))
							{
								// TODO(Mara): We're getting the bone index from Max to get the bone name to then look up the bone index again...
								//             We could at least pre-cache the max index -> w3d index mapping.
								INode* bone = skin->GetBone(context->GetAssignedBone(id, k));

								if (bone)
								{
									vert->BoneIndexes[k] = Hierarchy->GetBoneIndexForNode(bone);
									vert->BoneWeights[k] = (int)(context->GetBoneWeight(id, k) * 100);
								}
							}
						}

#ifndef W3X
						if (!vert->BoneWeights[0])
						{
							vert->BoneWeights[0] = vert->BoneWeights[1];
							vert->BoneIndexes[0] = vert->BoneIndexes[1];
							vert->BoneWeights[1] = 0;
							vert->BoneIndexes[1] = 0;

							if (!vert->BoneWeights[0])
							{
								vert->BoneWeights[0] = 100;
								vert->BoneIndexes[0] = 0;
							}
						}
#endif
					}

					corner++;
				}

				MeshBuilder.Add_Face(&face);
			}

			TT_PROFILER_SCOPE_STOP();
			MeshBuilder.Build_Mesh(keepnormals);
			LogDataDialogClass::WriteLogWindow(L" triangle count: %d\n", mesh->numFaces);
			LogDataDialogClass::WriteLogWindow(L" final vertex count: %d\n", MeshBuilder.Get_Vertex_Count());
			LogDataDialogClass::WriteLogWindow(L" vertex/triangle ratio: %f\n", (float)MeshBuilder.Get_Vertex_Count() / (float)mesh->numFaces);
			LogDataDialogClass::WriteLogWindow(L" strip count: %d\n", MeshBuilder.Get_Mesh_Stats().StripCount);
			LogDataDialogClass::WriteLogWindow(L" average strip length: %f\n", MeshBuilder.Get_Mesh_Stats().AvgStripLength);
			LogDataDialogClass::WriteLogWindow(L" longest strip: %d\n", MeshBuilder.Get_Mesh_Stats().MaxStripLength);
			LogDataDialogClass::AddToTotalVertexCount(MeshBuilder.Get_Vertex_Count());
		}

		void CalculateSkinData()
		{
			TT_PROFILER_SCOPE("MeshSave::CalculateSkinData");
			VertexInfluences = new W3dVertInfStruct[MeshBuilder.Get_Vertex_Count()];
			memset(VertexInfluences, 0, MeshBuilder.Get_Vertex_Count() * sizeof(W3dVertInfStruct));
			Header.VertexChannels |= W3D_VERTEX_CHANNEL_BONEID;
			HasSmoothSkin = false;

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				MeshBuilderClass::VertClass& vert = MeshBuilder.Get_Vertex(i);

				if (!vert.BoneIndexes[0])
				{
					VertexInfluences[i].BoneIdx[0] = 0;
					VertexInfluences[i].BoneIdx[1] = 0;
					VertexInfluences[i].Weight[0] = 100;
					VertexInfluences[i].Weight[1] = 0;
				}
				else
				{
					int count = 1;

					if (vert.BoneIndexes[1])
					{
						HasSmoothSkin = true;
						Header.VertexChannels |= W3D_VERTEX_CHANNEL_SMOOTHSKIN;
						vert.Vertexes[1] = vert.Vertexes[0];
						vert.Normals[1] = vert.Normals[0];
						count = 2;
					}

					Matrix3 m[2];
					Matrix3D tm[2];

					for (int j = 0; j < count; j++)
					{
						Hierarchy->GetFinalTransform(m[j], vert.BoneIndexes[j]);
						MakeMatrix3D(m[j], tm[j]);
						Vector3 v = vert.Vertexes[j];
						Matrix3D::Inverse_Transform_Vector(tm[j], v, &v);
						vert.Vertexes[j] = v;
						tm[j].Set_Translation(Vector3(0, 0, 0));
						Vector3 n = vert.Normals[j];
						Matrix3D::Inverse_Transform_Vector(tm[j], n, &n);
						vert.Normals[j] = n;
					}

					if (count == 1)
					{
						vert.Vertexes[1] = vert.Vertexes[0];
						vert.Normals[1] = vert.Normals[0];
					}

					VertexInfluences[i].BoneIdx[0] = vert.BoneIndexes[0];
					VertexInfluences[i].BoneIdx[1] = vert.BoneIndexes[1];
					VertexInfluences[i].Weight[0] = vert.BoneWeights[0];
					VertexInfluences[i].Weight[1] = vert.BoneWeights[1];
				}
			}

			ComputeSkinOptimization();
		}

#ifndef W3X
		bool SaveShaders(ChunkSaveClass& csave)
		{
			if (Materials.GetShaderCount() <= 0)
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::SHADERS))
			{
				return false;
			}

			for (int i = 0; i < Materials.GetShaderCount(); i++)
			{
				auto shader = Materials.GetShader(i);

				if (csave.Write(shader, sizeof(W3dShaderStruct)) != sizeof(W3dShaderStruct))
				{
					return true;
				}
			}

			return !csave.End_Chunk();
		}

		bool SaveFXShaders(ChunkSaveClass& csave)
		{
			if (Materials.GetFXShaderCount() <= 0)
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::FX_SHADERS))
			{
				return true;
			}

			int i = 0;

			if (Materials.GetFXShaderCount() <= 0)
			{
				return !csave.End_Chunk();
			}

			while (csave.Begin_Chunk(W3DChunkType::FX_SHADER))
			{
				if (!csave.Begin_Chunk(W3DChunkType::FX_SHADER_INFO))
				{
					break;
				}

				char version = 1;

				if (csave.Write(&version, 1) != 1)
				{
					break;
				}

				W3dFXShaderStruct* header = Materials.GetFXShaderHeader(i);

				if (csave.Write(header, sizeof(W3dFXShaderStruct)) != sizeof(W3dFXShaderStruct) || !csave.End_Chunk())
				{
					break;
				}

				ShaderParam* params = Materials.GetFXShaderParams(i);

				while (params->Type)
				{
					if (!csave.Begin_Chunk(W3DChunkType::FX_SHADER_CONSTANT) || csave.Write(&params->Type, 4) != 4)
					{
						return true;
					}

					int len = params->ParameterName.Get_Length() + 1;

					if (csave.Write(&len, 4) != 4)
					{
						return true;
					}

					if (csave.Write(params->ParameterName.Peek_Buffer(), len) != len)
					{
						return true;
					}

					int Type = params->Type;

					if (Type == CONSTANT_TYPE_TEXTURE)
					{
						int tlen = params->TextureParam.Get_Length() + 1;

						if (csave.Write(&tlen, 4) != 4)
						{
							return true;
						}

						if (csave.Write(params->TextureParam.Peek_Buffer(), tlen) != tlen)
						{
							return true;
						}
					}
					else if (Type < CONSTANT_TYPE_FLOAT1 || Type > CONSTANT_TYPE_FLOAT4)
					{
						if (Type == CONSTANT_TYPE_INT)
						{
							if (csave.Write(&params->IntParam, 4) != 4)
							{
								return true;
							}
						}
						else
						{
							if (Type != CONSTANT_TYPE_BOOL)
							{
								return true;
							}

							if (csave.Write(&params->BoolParam, 1) != 1)
							{
								return true;
							}
						}
					}
					else
					{
						if (csave.Write(&params->FloatParam, 4 * Type - 4) != 4 * Type - 4)
						{
							return true;
						}
					}

					if (!csave.End_Chunk())
					{
						return true;
					}

					params++;
				}

				if (!csave.End_Chunk())
				{
					return true;
				}

				i++;

				if (i >= Materials.GetFXShaderCount())
				{
					return !csave.End_Chunk();
				}
			}

			return true;
		}

		bool SaveTextureStage(ChunkSaveClass& csave, int pass, int stage)
		{
			if (!MeshBuilder.Get_Mesh_Stats().HasTexture[pass][stage])
			{
				return false;
			}

			if (!csave.Begin_Chunk(W3DChunkType::TEXTURE_STAGE))
			{
				return true;
			}

			SaveTextureIDs(csave, pass, stage);
			SaveStageTexcoords(csave, pass, stage);
			return !csave.End_Chunk();
		}

		bool SaveMaterialPass(ChunkSaveClass& csave, int pass)
		{
			if (!csave.Begin_Chunk(W3DChunkType::MATERIAL_PASS))
			{
				return true;
			}

			TT_PROFILER_SCOPE("MeshSave::SaveMaterialPass");

			if (MeshBuilder.Get_Mesh_Stats().HasVertexMaterial[pass])
			{
				SaveMaterialIDs(csave, pass);
			}

			if (MeshBuilder.Get_Mesh_Stats().HasShader[pass])
			{
				SaveShaderIDs(csave, pass);
			}

			if (MeshBuilder.Get_Mesh_Stats().HasDiffuseColor[pass])
			{
				SaveDCG(csave, pass);
			}

			if (MeshBuilder.Get_Mesh_Stats().HasFXShader[pass])
			{
				SaveFXShaderIDs(csave, pass);

				for (int i = 0; i < 8; i++)
				{
					SaveStageTexcoords(csave, pass, i);
				}
			}
			else
			{
				SaveTextureStage(csave, pass, 0);
				SaveTextureStage(csave, pass, 1);
			}

			if (!pass && IsMeshNPatchable(Node))
			{
				SaveTangentsBinormals(csave);
			}

			return !csave.End_Chunk();
		}

		bool Save(ChunkSaveClass& csave, bool optimizecollision, bool new_format)
		{
			TT_PROFILER_SCOPE("MeshSave::Save");

			if (!csave.Begin_Chunk(W3DChunkType::MESH))
			{
				return true;
			}

			if (SaveMeshHeader(csave))
			{
				return true;
			}

			if (SaveUserText(csave))
			{
				return true;
			}

			if (SaveVertices(csave))
			{
				return true;
			}

			if (SaveVertexNormals(csave))
			{
				return true;
			}

			if (Materials.GetNeedsTangents())
			{
				if (SaveTangentsBinormals(csave))
				{
					return true;
				}
			}

			if (SaveTriangles(csave))
			{
				return true;
			}

			if (SaveVertexInfluences(csave))
			{
				return true;
			}

			if (SaveVertexShadeIndices(csave))
			{
				return true;
			}

			if (SaveMaterialInfo(csave))
			{
				return true;
			}

			if (SaveVertexMaterials(csave))
			{
				return true;
			}

			if (SaveShaders(csave))
			{
				return true;
			}

			if (SaveTextures(csave))
			{
				return true;
			}

			if (SaveFXShaders(csave))
			{
				return true;
			}

			for (int i = 0; i < Materials.GetPassCount(); i++)
			{
				if (SaveMaterialPass(csave, i))
				{
					return true;
				}
			}

			if (optimizecollision == 1 && GenerateAABTree(csave, new_format))
			{
				return true;
			}

			return !csave.End_Chunk();
		}

#else
		bool SaveMeshHeader(XMLWriter& csave)
		{
			StringClass str = Header.ContainerName;
			str += ".";
			str += Header.MeshName;

			if (!csave.SetStringAttribute("id", str))
			{
				return true;
			}

			if (Header.Attributes & W3D_MESH_FLAG_HIDDEN && !csave.SetBoolAttribute("Hidden", true))
			{
				return true;
			}

			if (Header.Attributes & W3D_MESH_FLAG_CAST_SHADOW && !csave.SetBoolAttribute("CastShadow", true))
			{
				return true;
			}

			const char* geometry;

			switch (Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK)
			{
			case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
				geometry = "Normal";
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
				geometry = "CameraAligned";
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
				geometry = "Skin";
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
				geometry = "CameraOriented";
				break;
			default:
				throw ErrorClass(L"Obsolete geometry type (0x%08x) on mesh %S", Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK, Header.MeshName);
			}

			if (csave.SetStringAttribute("GeometryType", geometry) && (Header.SortLevel <= 0 || csave.SetIntAttribute("SortLevel", Header.SortLevel)) && csave.EndTag()
				&& csave.StartTag("BoundingBox", 1) && csave.EndTag() && csave.WriteVector("Min", Header.Min) && csave.WriteVector("Max", Header.Max) && csave.WriteClosingTag()
				&& csave.StartTag("BoundingSphere", 1) && csave.SetFloatAttribute("Radius", Header.SphRadius) && csave.EndTag() && csave.WriteVector("Center", Header.SphCenter) && csave.WriteClosingTag())
			{
				return false;
			}

			return true;
		}

		bool SaveVertices(XMLWriter& csave)
		{
			if (!csave.StartTag("Vertices", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct v;
				v.X = MeshBuilder.Get_Vertex(i).Vertexes[0].X;
				v.Y = MeshBuilder.Get_Vertex(i).Vertexes[0].Y;
				v.Z = MeshBuilder.Get_Vertex(i).Vertexes[0].Z;

				if (!csave.WriteVector("V", v))
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			if (HasSmoothSkin)
			{
				if (!csave.StartTag("Vertices", 1) || !csave.EndTag())
				{
					return true;
				}

				for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
				{
					W3dVectorStruct v;
					v.X = MeshBuilder.Get_Vertex(i).Vertexes[1].X;
					v.Y = MeshBuilder.Get_Vertex(i).Vertexes[1].Y;
					v.Z = MeshBuilder.Get_Vertex(i).Vertexes[1].Z;

					if (!csave.WriteVector("V", v))
					{
						return true;
					}
				}

				if (!csave.WriteClosingTag())
				{
					return true;
				}
			}

			return false;
		}

		bool SaveVertexNormals(XMLWriter& csave)
		{
			if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL))
			{
				return false;
			}

			if (!csave.StartTag("Normals", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct v;

				if (enum_has_flags(ExportFlags->GeometryFlags, W3DGeometryFlags::ZNormal))
				{
					v.X = 0.0f;
					v.Y = 0.0f;
					v.Z = 1.0f;
				}
				else
				{
					v.X = MeshBuilder.Get_Vertex(i).Normals[0].X;
					v.Y = MeshBuilder.Get_Vertex(i).Normals[0].Y;
					v.Z = MeshBuilder.Get_Vertex(i).Normals[0].Z;
				}

				if (!csave.WriteVector("N", v))
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			if (HasSmoothSkin)
			{
				if (!csave.StartTag("Normals", 1) || !csave.EndTag())
				{
					return true;
				}

				for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
				{
					W3dVectorStruct v;

					if (enum_has_flags(ExportFlags->GeometryFlags, W3DGeometryFlags::ZNormal))
					{
						v.X = 0.0f;
						v.Y = 0.0f;
						v.Z = 1.0f;
					}
					else
					{
						v.X = MeshBuilder.Get_Vertex(i).Normals[1].X;
						v.Y = MeshBuilder.Get_Vertex(i).Normals[1].Y;
						v.Z = MeshBuilder.Get_Vertex(i).Normals[1].Z;
					}

					if (!csave.WriteVector("N", v))
					{
						return true;
					}
				}

				if (!csave.WriteClosingTag())
				{
					return true;
				}
			}

			return false;
		}

		bool SaveTangentBinormals(XMLWriter& csave)
		{
			if (!(Header.VertexChannels & (W3D_VERTEX_CHANNEL_TANGENT | W3D_VERTEX_CHANNEL_BINORMAL)))
			{
				return false;
			}

			if (!csave.StartTag("Tangents", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct v;
				v.X = MeshBuilder.Get_Vertex(i).Tangent.X;
				v.Y = MeshBuilder.Get_Vertex(i).Tangent.Y;
				v.Z = MeshBuilder.Get_Vertex(i).Tangent.Z;

				if (!csave.WriteVector("T", v))
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			if (!csave.StartTag("Binormals", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				W3dVectorStruct v;
				v.X = MeshBuilder.Get_Vertex(i).Binormal.X;
				v.Y = MeshBuilder.Get_Vertex(i).Binormal.Y;
				v.Z = MeshBuilder.Get_Vertex(i).Binormal.Z;

				if (!csave.WriteVector("B", v))
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}

		bool SaveVertexColors(XMLWriter& csave)
		{
			if (!MeshBuilder.Get_Mesh_Stats().HasDiffuseColor[0])
			{
				return false;
			}

			if (!csave.StartTag("VertexColors", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				if (!csave.StartTag("C", 0) || !csave.SetFloatAttribute("R", MeshBuilder.Get_Vertex(i).DiffuseColor[0].X) || !csave.SetFloatAttribute("G", MeshBuilder.Get_Vertex(i).DiffuseColor[0].Y) || !csave.SetFloatAttribute("B", MeshBuilder.Get_Vertex(i).DiffuseColor[0].Z) || !csave.SetFloatAttribute("A", MeshBuilder.Get_Vertex(i).Alpha[0]) || !csave.EndTag())
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}

		bool SaveTexcoords(XMLWriter& csave)
		{
			for (int t = 0; t < 8; t++)
			{
				if (!MeshBuilder.Get_Mesh_Stats().HasTexCoords[t / 2][t % 2])
				{
					return false;
				}

				if (!csave.StartTag("TexCoords", 1) || !csave.EndTag())
				{
					return true;
				}

				for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
				{
					if (!csave.StartTag("T", 0) || !csave.SetFloatAttribute("X", MeshBuilder.Get_Vertex(i).TexCoord[t / 2][t % 2].X) || !csave.SetFloatAttribute("Y", MeshBuilder.Get_Vertex(i).TexCoord[t / 2][t % 2].Y) || !csave.EndTag())
					{
						return true;
					}
				}

				if (!csave.WriteClosingTag())
				{
					return true;
				}
			}

			return false;
		}

		bool SaveVertexInfluences(XMLWriter& csave)
		{
			if (((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) || !(Header.VertexChannels & W3D_VERTEX_CHANNEL_BONEID) || !VertexInfluences)
			{
				return false;
			}

			if (!csave.StartTag("BoneInfluences", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				if (!csave.StartTag("I", 0) || !csave.SetIntAttribute("Bone", VertexInfluences[i].BoneIdx[0]) || !csave.SetFloatAttribute("Weight", VertexInfluences[i].Weight[0] / 100.0f) || !csave.EndTag())
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			if (HasSmoothSkin)
			{
				if (!csave.StartTag("BoneInfluences", 1) || !csave.EndTag())
				{
					return true;
				}

				for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
				{
					if (!csave.StartTag("I", 0) || !csave.SetIntAttribute("Bone", VertexInfluences[i].BoneIdx[1]) || !csave.SetFloatAttribute("Weight", VertexInfluences[i].Weight[1] / 100.0f) || !csave.EndTag())
					{
						return true;
					}
				}

				if (!csave.WriteClosingTag())
				{
					return true;
				}
			}

			return false;
		}

		bool SaveVertexShadeIndices(XMLWriter& csave)
		{
			if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL))
			{
				return false;
			}

			if (!csave.StartTag("ShadeIndices", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Vertex_Count(); i++)
			{
				if (!csave.WriteUnsignedInt("I", MeshBuilder.Get_Vertex(i).ShadeIndex))
				{
					return true;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}

		bool SaveTriangles(XMLWriter& csave)
		{
			if (!csave.StartTag("Triangles", 1) || !csave.EndTag())
			{
				return true;
			}

			for (int i = 0; i < MeshBuilder.Get_Face_Count(); i++)
			{
				W3dTriStruct t;
				memset(&t, 0, sizeof(t));
				t.Vindex[0] = MeshBuilder.Get_Face(i).VertIdx[0];
				t.Vindex[1] = MeshBuilder.Get_Face(i).VertIdx[1];
				t.Vindex[2] = MeshBuilder.Get_Face(i).VertIdx[2];
				t.Attributes = MeshBuilder.Get_Face(i).SurfaceType;
				t.Normal.X = MeshBuilder.Get_Face(i).Normal.X;
				t.Normal.Y = MeshBuilder.Get_Face(i).Normal.Y;
				t.Normal.Z = MeshBuilder.Get_Face(i).Normal.Z;
				t.Dist = MeshBuilder.Get_Face(i).Dist;

				if (!csave.StartTag("T", 1) || !csave.EndTag() || !csave.WriteUnsignedInt("V", t.Vindex[0]) || !csave.WriteUnsignedInt("V", t.Vindex[1]) || !csave.WriteUnsignedInt("V", t.Vindex[2]) || !csave.WriteVector("Nrm", t.Normal) || !csave.WriteFloat("Dist", t.Dist) || !csave.WriteClosingTag())
				{
					return false;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}

		bool SaveFXShaders(XMLWriter& csave)
		{
			if (Materials.GetFXShaderCount() != 1)
			{
				WideStringClass str;
				str.Format(L"Mesh \"%S\" has invalid material. Materials have to use \"DirectX 9 Shader Material\" and there can only be one per mesh", Header.MeshName);
				LogDataDialogClass::WriteLogWindow(str);
				throw ErrorClass(str);
			}

			if (!csave.StartTag("FXShader", 1) || !csave.SetStringAttribute("ShaderName", Materials.GetFXShaderHeader(0)->shadername) || !csave.SetIntAttribute("TechniqueIndex", Materials.GetFXShaderHeader(0)->technique) || !csave.EndTag() || !csave.StartTag("Constants", 1) || !csave.EndTag())
			{
				return true;
			}

			ShaderParam* params = Materials.GetFXShaderParams(0);

			for (int i = 0; i < 64; i++)
			{
				if (!params[i].Type)
				{
					break;
				}

				switch (params[i].Type)
				{
				case CONSTANT_TYPE_TEXTURE:
				{
					if (!csave.StartTag("Texture", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteString("Value", params[i].TextureParam) || !csave.WriteClosingTag())
					{
						return true;
					}

					StringClass str = StringClass("ART:") + params[i].TextureParam + StringClass(".xml");
					Includes->push_back(str);
				}
				break;
				case CONSTANT_TYPE_FLOAT1:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[0]))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				case CONSTANT_TYPE_FLOAT2:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[0]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[1]))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				case CONSTANT_TYPE_FLOAT3:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[0]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[1]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[2]))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				case CONSTANT_TYPE_FLOAT4:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[0]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[1]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[2]))
					{
						return true;
					}

					if (!csave.WriteFloat("Value", params[i].FloatParam[3]))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				case CONSTANT_TYPE_INT:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteInt("Int", params[i].IntParam))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				case CONSTANT_TYPE_BOOL:
					if (!csave.StartTag("Float", 1))
					{
						return true;
					}

					if (!csave.SetStringAttribute("Name", params[i].ParameterName))
					{
						return true;
					}

					if (!csave.EndTag())
					{
						return true;
					}

					if (!csave.WriteInt("Bool", params[i].BoolParam))
					{
						return true;
					}

					if (!csave.WriteClosingTag())
					{
						return true;
					}

					break;
				}
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			if (!csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}

		bool GenerateAABTree(XMLWriter& csave)
		{
			int facecount = MeshBuilder.Get_Face_Count();

			if (facecount >= 8 && (Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL)
			{
				int vertcount = MeshBuilder.Get_Vertex_Count();
				Vector3* verts = new Vector3[vertcount];
				TriIndex* polys = new TriIndex[facecount];

				for (int i = 0; i < vertcount; i++)
				{
					verts[i] = MeshBuilder.Get_Vertex(i).Vertexes[0];
				}

				for (int i = 0; i < facecount; i++)
				{
					polys[i].I = MeshBuilder.Get_Face(i).VertIdx[0];
					polys[i].J = MeshBuilder.Get_Face(i).VertIdx[1];
					polys[i].K = MeshBuilder.Get_Face(i).VertIdx[2];
				}

				AABTreeBuilderClass builder;
				builder.Build_AABTree(facecount, polys, vertcount, verts, false);
				builder.Export(csave);
				delete[] verts;
				delete[] polys;
			}

			return false;
		}
		bool Save(XMLWriter& csave, bool optimizecollision)
		{
			return !csave.StartTag("W3DMesh", 1) || SaveMeshHeader(csave) || SaveVertices(csave) || SaveVertexNormals(csave) || SaveTangentBinormals(csave) || SaveVertexColors(csave) || SaveTexcoords(csave) || SaveVertexInfluences(csave) || SaveVertexShadeIndices(csave) || SaveTriangles(csave) || SaveFXShaders(csave) || (optimizecollision && GenerateAABTree(csave)) || !csave.WriteClosingTag();
		}
#endif
	};

	class MeshGeometryExportTaskClass : public GeometryExportTaskClass
	{
		Mesh Mesh;
		W3DAppDataChunk ExportFlags;
		Mtl* Material;
		Point3 Point1;
		Point3 Point2;
		Box3 Box;
		bool ValidMesh = false;
		bool ExportMesh = true;

	public:
		MeshGeometryExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod), Material(nullptr)
		{
			TT_PROFILER_SCOPE("MeshGeometryExportTaskClass()");
			W3DAppDataChunk* data = &W3DUtilities::GetOrCreateW3DAppDataChunk(*Node);
			memcpy(&ExportFlags, data, sizeof(ExportFlags));
			Object* obj = Node->EvalWorldState(Time).obj;

#ifndef W3X
			if (MeshDeduplication)
			{
				if (ObjectMap.find(obj) != ObjectMap.end() && IsExportBone(Node))
				{
					ExportMesh = false;
					CopyW3DName(Name, ObjectMap[obj]);
				}
				else
				{
					ObjectMap[obj] = Name;
				}
			}
#endif

			if (obj->ConvertToType(Time, triObjectClassID) != nullptr)
			{
				Mesh = ((TriObject*)obj->ConvertToType(Time, triObjectClassID))->mesh;
				ValidMesh = true;
				Initialize();
			}
		}

		void Initialize()
		{
			TT_PROFILER_SCOPE("MeshGeometryExportTaskClass::Initialize");
			Material = nullptr;
			Mtl* mat = Node->GetMtl();

			if (mat)
			{
				if (mat->NumSubMtls() > 1)
				{
					int count = mat->NumSubMtls();
					assert(count > 0);
					bool* foundmtls = new bool[count];
					memset(foundmtls, 0, count);

					for (int i = 0; i < Mesh.numFaces; i++)
					{
						int m = HIWORD(Mesh.faces[i].flags) % count;
						foundmtls[m] = true;
					}

					int foundcount = 0;

					for (int i = 0; i < count; i++)
					{
						if (foundmtls[i])
						{
							Material = mat->GetSubMtl(i);
							foundcount++;
						}
					}

					delete[] foundmtls;

					if (foundcount > 1)
					{
						Material = nullptr;
					}
				}
				else
				{
					Material = mat;
				}
			}
			else
			{
				Material = nullptr;
			}

			int vertcount = Mesh.numVerts;
			Point3 p1(0, 0, 0);
			Point3 p2(0, 0, 0);

			if (vertcount > 0)
			{
				Point3* verts = Mesh.verts;
				p1 = verts[1];
				p2 = verts[0];

				for (int i = vertcount; i; i--)
				{
					if (p1.x > verts->x)
					{
						p1.x = verts->x;
					}

					if (p1.y > verts->y)
					{
						p1.y = verts->y;
					}

					if (p1.z > verts->z)
					{
						p1.z = verts->z;
					}

					if (p2.x < verts->x)
					{
						p2.x = verts->x;
					}

					if (p2.y < verts->y)
					{
						p2.y = verts->y;
					}

					if (p2.z < verts->z)
					{
						p2.z = verts->z;
					}

					verts++;
				}
			}

			p2 += Point3(0.1f, 0.1f, 0.1f);
			p1 -= Point3(0.1f, 0.1f, 0.1f);
			Point1 = (p2 + p1) * 0.5f;
			Point2 = (p2 - p1) * 0.5f;
			Box3 box(p1, p2);
			Box = box * Node->GetObjectTM(Time);
		}

		virtual void Save(LodData& lod)
		{
			if (ExportMesh)
			{
				lod.Info->Set_Current_Geometry_Task(this);
				lod.Info->Set_Transform(Transform);
#ifndef W3X
				MeshSave* m = new MeshSave(Name, ContainerName, Node, &Mesh, &Transform, &ExportFlags, lod.ExportData, lod.Hierarchy, lod.Time, lod.Info);
				m->Save(*lod.ChunkSave, lod.ExportData->OptimiseCollisions, lod.ExportData->NewAABTree);
#else
				MeshSave* m = new MeshSave(Name, ContainerName, Node, &Mesh, &Transform, &ExportFlags, lod.ExportData, lod.Includes, lod.Hierarchy, lod.Time, lod.Info);
				m->Save(*lod.ChunkSave, lod.ExportData->OptimiseCollisions);
#endif
				delete m;
			}
		}

		virtual Point3 Build_Vertex_Normal_For_Point(Point3& point, int smoothing)
		{
			// TODO(Mara): it might be possible to build a temporary AABTree beforehand and then use it here? this is incredibly expensive
			Point3 p(0, 0, 0);

			if (Box.Contains(point))
			{
				Matrix3 tm = Node->GetObjectTM(Time);
				Point3 p2 = point * Inverse(tm);

				for (int i = 0; i < Mesh.numFaces; i++)
				{
					Face& face = Mesh.faces[i];

					if (face.smGroup & smoothing || face.smGroup == smoothing)
					{
						bool b = false;

						for (int j = 0; j < 3; j++)
						{
							if (b)
							{
								break;
							}

							Point3 p3 = p2 - Mesh.verts[face.v[j]];

							if (fabs(p3.x) < 0.001f && fabs(p3.y) < 0.001f && fabs(p3.z) < 0.001f)
							{
								Point3 v0 = Mesh.verts[face.v[0]];
								Point3 v1 = Mesh.verts[face.v[1]];
								Point3 v2 = Mesh.verts[face.v[2]];
								p = p + Normalize((v1 - v0) ^ (v2 - v1));
								b = true;
							}
						}
					}
				}

				tm.NoTrans();
				p = tm.PointTransform(p);
			}

			return p;
		}

		virtual int GetType()
		{
			return 0;
		}

		bool IsValidMesh()
		{
			return ValidMesh;
		}
	};

	class CollisionBoxSave
	{
		W3dBoxStruct Box;
#ifdef W3X
		char ContainerName[W3D_NAME_LEN];
		bool JoypadPick;
#endif
	public:
		CollisionBoxSave(const char* name, const char* containername, INode* node, Matrix3& transform, TimeValue time)
		{
#ifdef W3X
			JoypadPick = false;
#endif
			Object* o = node->EvalWorldState(time).obj;
			TriObject* tri = (TriObject*)o->ConvertToType(time, triObjectClassID);
			Mesh m(tri->GetMesh());
			DWORD color = node->GetWireColor();

			if (!m.getNumVerts())
			{
				throw ErrorClass(L"Mesh %S has no vertices!\n", name);
			}

			memset(&Box, 0, sizeof(Box));
			Box.Version = 0x10000;

#ifdef W3X
			if (containername && strlen(containername))
			{
				strcpy(ContainerName, containername);
			}
			else
			{
				ContainerName[0] = 0;
			}
#endif

#ifndef W3X
			char newname[128];
			memset(newname, 0, 128);

			if (containername && containername[0])
			{
				strcat(newname, containername);
				strcat(newname, ".");
			}

			strcat(newname, name);
			strncpy(Box.Name, newname, W3D_NAME_LEN * 2);
#else
			strncpy(Box.Name, name, W3D_NAME_LEN * 2);
#endif
			Box.Attributes = 0;

			if (IsAABox(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBUTE_ALIGNED;
			}
			else
			{
				Box.Attributes |= W3D_BOX_ATTRIBUTE_ORIENTED;
			}

#ifdef W3X
			JoypadPick = IsJoypadPick(node);
#else

			if (IsCollidePhysical(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL;
			}

			if (IsCollideProjectile(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE;
			}

			if (IsCollideVis(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS;
			}

			if (IsCollideCamera(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA;
			}

			if (IsCollideVehicle(node))
			{
				Box.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE;
			}

#endif
			Box.Color.R = GetRValue(color);
			Box.Color.G = GetGValue(color);
			Box.Color.B = GetBValue(color);

			if (IsAABox(node))
			{
				transform.NoRot();
			}

			Matrix3 tm = node->GetObjectTM(time) * Inverse(transform);

			for (int i = 0; i < m.numVerts; i++)
			{
				m.verts[i] = m.verts[i] * tm;
			}

			Point3* verts = m.verts;
			Point3 p1 = verts[0];
			Point3 p2 = verts[1];

			for (int i = m.numVerts; i; i--)
			{
				if (p1.x > verts->x)
				{
					p1.x = verts->x;
				}

				if (p1.y > verts->y)
				{
					p1.y = verts->y;
				}

				if (p1.z > verts->z)
				{
					p1.z = verts->z;
				}

				if (p2.x < verts->x)
				{
					p2.x = verts->x;
				}

				if (p2.y < verts->y)
				{
					p2.y = verts->y;
				}

				if (p2.z < verts->z)
				{
					p2.z = verts->z;
				}

				verts++;
			}

			Point3 Center = (p2 + p1) * 0.5f;
			Point3 Extent = (p2 - p1) * 0.5f;
			Box.Center.X = Center.x;
			Box.Center.Y = Center.y;
			Box.Center.Z = Center.z;
			Box.Extent.X = Extent.x;
			Box.Extent.Y = Extent.y;
			Box.Extent.Z = Extent.z;
		}

#ifndef W3X
		bool Save(ChunkSaveClass& csave)
		{
			csave.Begin_Chunk(W3DChunkType::BOX);
			csave.Write(&Box, sizeof(Box));
			csave.End_Chunk();
			return false;
		}
#else

		bool Save(XMLWriter& csave)
		{
			StringClass str(ContainerName);
			str += ".";
			str += Box.Name;

			if (csave.StartTag("W3DCollisionBox", 1) && csave.SetStringAttribute("id", str) && (!JoypadPick || csave.SetBoolAttribute("JoypadPickingOnly", true)) && csave.EndTag() && csave.WriteVector("Center", Box.Center) && csave.WriteVector("Extent", Box.Extent) && csave.WriteClosingTag())
			{
				return true;
			}

			return false;
		}
#endif
	};

	class CollisionBoxGeometryExportTaskClass : public GeometryExportTaskClass
	{
		bool ValidMesh = false;
	public:
		CollisionBoxGeometryExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod)
		{
			Object* o = node->EvalWorldState(Time).obj;

			if (o->ConvertToType(Time, triObjectClassID))
			{
				ValidMesh = true;
			}
		}

		virtual void Save(LodData& lod)
		{
			CollisionBoxSave* e = new CollisionBoxSave(Name, ContainerName, Node, Transform, Time);
			e->Save(*lod.ChunkSave);
			delete e;
		}

		bool IsValidMesh()
		{
			return ValidMesh;
		}

		virtual int GetType()
		{
			return 1;
		}
	};

#ifndef W3X
	class NullExportTaskClass : public GeometryExportTaskClass
	{
	public:
		NullExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod)
		{
			memset(ContainerName, 0, sizeof(ContainerName));
			memset(Name, 0, sizeof(Name));
			strcpy(Name, "NULL");
		}

		virtual void Save(LodData& lod)
		{
		}

		virtual int GetType()
		{
			return 3;
		}
	};

	class AggregateExportTaskClass : public GeometryExportTaskClass
	{
	public:
		AggregateExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod)
		{
			memset(ContainerName, 0, sizeof(ContainerName));
		}

		virtual void Save(LodData& lod)
		{
		}

		virtual bool IsAggregate() { return true; }

		virtual int GetType()
		{
			return 4;
		}
	};
#endif

	class ProxyExportTaskClass : public GeometryExportTaskClass
	{
	public:
		ProxyExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod)
		{
			memset(ContainerName, 0, sizeof(ContainerName));
			char* s = strrchr(Name, '~');
			memset(s, 0, Name - s + W3D_NAME_LEN);
		}

		virtual void Save(LodData& lod)
		{
		}

		virtual bool IsProxy() { return true; }

		virtual int GetType()
		{
			return 5;
		}
	};

#ifndef W3X
	class DazzleExportTaskClass : public GeometryExportTaskClass
	{
	public:
		DazzleExportTaskClass(INode* node, LodData& lod) : GeometryExportTaskClass(node, lod)
		{
		}

		virtual void Save(LodData& lod)
		{
			char name[128];
			memset(name, 0, 128);

			if (ContainerName[0])
			{
				strcat(name, ContainerName);
				strcat(name, ".");
			}

			strcat(name, Name);
			lod.ChunkSave->Begin_Chunk(W3DChunkType::DAZZLE);
			lod.ChunkSave->Begin_Chunk(W3DChunkType::DAZZLE_NAME);
			lod.ChunkSave->Write(name, (int)strlen(name) + 1);
			lod.ChunkSave->End_Chunk();
			lod.ChunkSave->Begin_Chunk(W3DChunkType::DAZZLE_TYPENAME);
			const char* dazzle = W3DUtilities::GetDazzleTypeFromAppData(Node);
			lod.ChunkSave->Write(dazzle, (int)strlen(dazzle) + 1);
			lod.ChunkSave->End_Chunk();
			lod.ChunkSave->End_Chunk();
		}

		virtual int GetType()
		{
			return 2;
		}
	};
#endif

	GeometryExportTaskClass* CreateGeometryExportTask(INode* node, LodData& lod)
	{
		if (!GetExportGeometry(node))
		{
			return nullptr;
		}

		if (wcsrchr(node->GetName(), '~'))
		{
			return new ProxyExportTaskClass(node, lod);
		}

#ifndef W3X
		if (IsNormalGeometry(node) || IsCameraAligned(node) || IsCameraOriented(node) || IsCameraZOriented(node) || HasSkin(node))
#else
		if (IsNormalGeometry(node) || IsCameraAligned(node) || IsCameraOriented(node) || HasSkin(node))
#endif
		{
			MeshGeometryExportTaskClass* geoExportTask = new MeshGeometryExportTaskClass(node, lod);

			if (geoExportTask->IsValidMesh())
			{
				return geoExportTask;
			}

			delete geoExportTask;
			return nullptr;
		}

		if (IsAABox(node) || IsOBBox(node))
		{
			CollisionBoxGeometryExportTaskClass* colBoxExportTask = new CollisionBoxGeometryExportTaskClass(node, lod);

			if (colBoxExportTask->IsValidMesh())
			{
				return colBoxExportTask;
			}

			delete colBoxExportTask;
			return nullptr;
		}

#ifndef W3X
		if (IsNullGeometry(node))
		{
			return new NullExportTaskClass(node, lod);
		}

		if (IsDazzle(node))
		{
			return new DazzleExportTaskClass(node, lod);
		}

		if (IsAggregate(node))
		{
			return new AggregateExportTaskClass(node, lod);
		}
#endif

		return nullptr;
	}

	MeshConnection::MeshConnection(DynamicVectorClass<GeometryExportTaskClass*> vector, LodData& lod) : Time(lod.Time), Node(lod.Node)
	{
		TT_PROFILER_SCOPE("MeshConnection::MeshConnection");
		CopyW3DName(Name, lod.Name);

		for (int i = 0; i < vector.Count(); i++)
		{
			ConnectionStruct con;
			vector[i]->GetSubObjectName(con.Name, sizeof(con.Name));
			con.BoneIndex = vector[i]->BoneIndex;
			con.Node = vector[i]->Node;
#ifdef W3X
			con.Type = vector[i]->GetType();
#endif

			if (vector[i]->IsAggregate())
			{
				Aggregates.Add(con);
			}
			else if (vector[i]->IsProxy())
			{
				Proxies.Add(con);
			}
			else
			{
				Meshes.Add(con);
			}
		}
	};

	INodeListClass* W3DExport::CreateOriginNodeList()
	{
		if (!OriginNodeList)
		{
			TT_PROFILER_SCOPE("W3DExport::CreateOriginNodeList");
			static OriginFilterClass filter;
			OriginNodeList = new INodeListClass(ExpInt->theScene, Time, &filter);

			if (!OriginNodeList->GetNodeCount())
			{
				OriginNodeList->AddNode(Int->GetRootNode());
			}
		}

		return OriginNodeList;
	}

#ifndef W3X
	void W3DExport::ExportData(char* name, ChunkSaveClass& csave)
#else
	void W3DExport::ExportData(char* name, XMLWriter& csave)
#endif
	{
#ifdef W3X
		size_t namelen = strlen(name);

		if (namelen > 2)
		{
			char* c = &name[namelen - 1];

			if (!_stricmp(c, "_L") || !_stricmp(c, "_M"))
			{
				LogDataDialogClass::WriteLogWindow(L"LOD export detected. Truncating asset name '%S'", name);
				*c = 0;
				LogDataDialogClass::WriteLogWindow(L" to '%S'\n", name);
			}
		}
#endif

		TT_PROFILER_SCOPE("W3DExport::ExportData");

#ifdef W3X

		RAMFileClass file(nullptr, 10240);
		file.Set_Reallocate(true);
		file.Open(2);
		XMLWriter writer(&file, false);
		writer.SetPaddingOffset(1);
#else
		ChunkSaveClass writer = csave;
#endif

		INodeListClass* list = CreateOriginNodeList();
		INode* node = nullptr;
		int NodeCount = list->GetNodeCount();

		for (INode* n : *list)
		{
			if (IsHierarchyRootNode(n))
			{
				node = n;
				break;
			}
		}

		if (!ExportHierarchy(name, writer, node))
		{
			MessageBox(nullptr, L"Hierarchy Export Failure!", L"Error", MB_SETFOREGROUND);
			return;
		}

		if (!ExportAnimation(name, writer, node))
		{
			MessageBox(nullptr, L"Animation Export Failure!", L"Error", MB_SETFOREGROUND);
			return;
		}

		MeshConnection** connections = new MeshConnection * [NodeCount];

		if (!connections)
		{
			MessageBox(nullptr, L"Memory allocation failure!", L"Error", MB_SETFOREGROUND);
			return;
		}

		memset(connections, 0, 8 * NodeCount);
		size_t len = strlen(name);
		name[len + 1] = 0;

		if (!m_Settings.UseExistingSkeleton && !m_Settings.ExportSkeleton)
		{
			NodeCount = 1;
		}

		for (int i = 0; i < NodeCount; i++)
		{
			MeshConnection* connection = nullptr;

			if (!ExportGeometry(name, writer, list->GetNode(i), &connection))
			{
				MessageBox(nullptr, L"Geometry Export Failure!", L"Error", MB_SETFOREGROUND);
				return;
			}

			int level = GetLODLevelFromNode(list->GetNode(i));

			if (level >= NodeCount || connections[NodeCount - level - 1])
			{
				WideStringClass str;
				str.Format(L"Origin Naming Error! There are %d models defined in this scene, therefore your origin names should be\n\"Origin.00\" through \"Origin.%02d\", 00 being the high-poly model and %02d being the lowest detail LOD.", NodeCount, NodeCount - 1, NodeCount - 1);
				MessageBox(nullptr, str, L"Error", MB_SETFOREGROUND);
				return;
			}

			connections[NodeCount - level - 1] = connection;
		}

		if (m_Settings.UseExistingSkeleton || m_Settings.ExportSkeleton)
		{
			name[len] = 0;
			HierarchySave* hierarchy = GetHierarchy();

			if (hierarchy)
			{
				if (!ExportHlod(name, hierarchy->GetHierarchyName(), writer, connections, NodeCount))
				{
					MessageBox(nullptr, L"HLOD Generation Failure!", L"Error", MB_SETFOREGROUND);
					return;
				}
			}
		}

#ifdef W3X
		csave.StartTag("AssetDeclaration", 1);
		csave.SetStringAttribute("xmlns", "uri:ea.com:eala:asset");
		csave.SetStringAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		csave.EndTag();

		if (csave.StartTag("Includes", 1) && csave.EndTag())
		{
			for (int i = 0; i < includes.size(); i++)
			{
				if (!csave.StartTag("Include", 0) || !csave.SetStringAttribute("type", "all") || !csave.SetStringAttribute("source", includes[i]) || !csave.EndTag())
				{
					break;
				}
			}

			if (csave.WriteClosingTag())
			{
				csave.WriteFile(file.Get_Buffer(), file.Get_Length());
				csave.WriteClosingTag();
			}
		}
#endif

		for (int i = 0; i < NodeCount; i++)
		{
			if (connections[i])
			{
				delete connections[i];
			}
		}

		delete[] connections;
	}

#ifndef W3X
	bool W3DExport::ExportHierarchy(const char* name, ChunkSaveClass& csave, INode* node)
#else
	bool W3DExport::ExportHierarchy(const char* name, XMLWriter& csave, INode* node)
#endif
	{
		TT_PROFILER_SCOPE("W3DExport::ExportHierarchy");

		if (!m_Settings.ExportSkeleton)
		{
#ifdef W3X
			if (m_Settings.UseExistingSkeleton)
			{
				wchar_t* str = m_Settings.ExistingSkeletonFileName;
				wchar_t buf[524];

				if (m_Settings.ExistingSkeletonFileName[0])
				{
					wchar_t* s1 = wcsrchr(m_Settings.ExistingSkeletonFileName, L'/');
					wchar_t* s2 = wcsrchr(m_Settings.ExistingSkeletonFileName, L'\\');
					wchar_t* s3 = s1;

					if (s2 >= s1)
					{
						s3 = s2;
					}

					if (s3)
					{
						wcscpy(buf, s3 + 1);
					}
					else
					{
						wchar_t* s6 = m_Settings.ExistingSkeletonFileName;
						wcscpy(buf, s6);
					}

					_wcslwr(buf);

					for (wchar_t* i = buf; *i; i++)
					{
						if (*i == L'\\')
						{
							*i = L'/';
						}
					}

					WideStringClass str = WideStringClass(L"ART:") + buf;
					StringClass str2 = str;
					includes.push_back(str2);
				}
			}
#endif

			return true;
		}

		ExportAsTerrain = m_Settings.ExportAsTerrain;

		if (node)
		{
			try
			{
				HierarchyStruct = new HierarchySave(node, Time, name, 2, nullptr);
			}
			catch (ErrorClass& e)
			{
				MessageBox(nullptr, e.GetError(), L"Error", MB_SETFOREGROUND);
				return false;
			}

			HierarchyStruct->SaveHierarchy(csave);
			return true;
		}

		return false;
	}

	Vector3 MaxWorldInfoClass::Get_Shared_Vertex_Normal(Vector3 v, int smoothing)
	{
		Point3 p;
		p.x = v.X;
		p.y = v.Y;
		p.z = v.Z;
		Vector3 p2(0, 0, 0);
		Point3 p3 = Transform * p;

		for (int i = 0; i < Vector->Count(); i++)
		{
			GeometryExportTaskClass* task = (*Vector)[i];

			if (task != CurrentGeometryTask)
			{
				Point3& p4 = task->Build_Vertex_Normal_For_Point(p3, smoothing);
				p2.X += p4.x;
				p2.Y += p4.y;
				p2.Z += p4.z;
			}
		}

		return p2;
	}

#ifndef W3X
	AnimationSave::AnimationSave(IScene* scene, INode* node, HierarchySave* hierarchy, W3DExportSettings* exportstr, int framerate, const char* name, Matrix3& mat) : Scene(scene), Node(node), Tree(nullptr), Hierarchy(hierarchy), StartFrame(exportstr->AnimFramesStart), EndFrame(exportstr->AnimFramesEnd), FrameRate(framerate), Matrix(mat)
#else
	AnimationSave::AnimationSave(IScene* scene, INode* node, HierarchySave* hierarchy, W3DExportSettings* exportstr, int framerate, const char* name, Matrix3& mat) : Scene(scene), Node(node), Tree(nullptr), Hierarchy(hierarchy), StartFrame(exportstr->AnimFramesStart), EndFrame(exportstr->AnimFramesEnd), FrameRate(framerate), Matrix(mat), ExportStr(exportstr)
#endif
	{
		LogDataDialogClass::WriteLogWindow(L"Initializing Capture....\n");
		CaptureBones();
		CopyW3DName(Name, name);
	}

	void AnimationSave::CaptureBones()
	{
		NumFrames = EndFrame - StartFrame + 1;
		LogDataDialogClass::WriteLogWindow(L"Extracting %d frames of animation from Max\n", NumFrames);
		LogDataDialogClass::WriteLogWindow(L"\n");
		Transforms = new Matrix3 * [Hierarchy->GetBoneCount()];

		if (!Transforms)
		{
			throw ErrorClass(L"Out Of Memory!");
		}

		Angles = new Point3 * [Hierarchy->GetBoneCount()];

		if (!Angles)
		{
			throw ErrorClass(L"Out Of Memory!");
		}

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			Transforms[i] = new Matrix3[NumFrames];

			if (!Transforms[i])
			{
				throw ErrorClass(L"Out Of Memory!");
			}

			for (int j = 0; j < NumFrames; j++)
			{
				Transforms[i][j];
			}
		}

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			Angles[i] = new Point3[NumFrames];

			if (!Angles[i])
			{
				throw ErrorClass(L"Out Of Memory!");
			}

			for (int j = 0; j < NumFrames; j++)
			{
				Angles[i][j].x = 0.0f;
				Angles[i][j].y = 0.0f;
				Angles[i][j].z = 0.0f;
			}
		}

		BitChannels = new BooleanVectorClass[Hierarchy->GetBoneCount()];
		VisibilityChannels = new VectorClass<float>[Hierarchy->GetBoneCount()];

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			BitChannels[i].Resize(NumFrames);
			VisibilityChannels[i].Resize(NumFrames);

			for (int j = 0; j < NumFrames; j++)
			{
				BitChannels[i][j] = true;
				VisibilityChannels[i][j] = 1.0f;
			}
		}

		BinaryMove = new BooleanVectorClass[Hierarchy->GetBoneCount()];

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			BinaryMove[i].Resize(NumFrames);

			for (int j = 0; j < NumFrames; j++)
			{
				BinaryMove[i][j] = true;
			}
		}

		HasData.Resize(Hierarchy->GetBoneCount());

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			HasData[i] = false;
		}

		for (int j = 0; j < NumFrames; j++)
		{
			CaptureFrame(j);
		}

		LogDataDialogClass::WriteLogWindow(L"Extraction Complete.\n");
	}

	void AnimationSave::CaptureFrame(int frame)
	{
		TimeValue time = (frame + StartFrame) * GetTicksPerFrame();
		INode* node = Node;
		HierarchySave* new_hierarchy;

		if (node)
		{
			new_hierarchy = new HierarchySave(Node, time, "NoName", 0, Hierarchy);
		}
		else
		{
			new_hierarchy = new HierarchySave(Tree, time, "NoName", 0, Hierarchy, &Matrix);
		}

		if (!new_hierarchy)
		{
			throw ErrorClass(L"Out Of Memory!");
		}

		for (int i = 0; i < new_hierarchy->GetBoneCount(); i++)
		{
			int pivot = Hierarchy->FindBone(new_hierarchy->GetName(i));

			if (pivot != -1)
			{
				Matrix3& m3 = DoMatrixFixup(new_hierarchy->GetTransform(i) * Inverse(Hierarchy->GetTransform(pivot)));
				CopyTransform(pivot, frame, m3);
				Matrix3D m4;
				MakeMatrix3D(m3, m4);
				EulerAngles e;
				e.FromMatrix3D(m4);
				SetAngles(pivot, frame, (float)e[0], (float)e[1], (float)e[2]);
				node = new_hierarchy->GetNode(i);
				float floatvis = 1.0f;
				bool vis = true;

				if (node)
				{
					floatvis = node->GetVisibility(time, nullptr);

					if (floatvis <= 0.0f)
					{
						vis = false;
					}
				}

				CopyVisibility(pivot, frame, vis, floatvis);
				bool move = false;

				if (node)
				{
					if (vis)
					{
						if (frame)
						{
							int tpf = GetTicksPerFrame();
							int time3 = time - tpf;
							int time2 = (time - tpf + time) / 2;
							Matrix3 m1;
							Matrix3 m2;
							Matrix3 m3;
							memcpy(&m1, &node->GetNodeTM(time3), sizeof(m1));
							memcpy(&m2, &node->GetNodeTM(time2), sizeof(m2));
							memcpy(&m3, &node->GetNodeTM(time), sizeof(m3));

							if (m1 == m2)
							{
								if (m2 != m3)
								{
									move = true;
									LogDataDialogClass::WriteLogWindow(L"Binary move: bone %S, frame %d\n", new_hierarchy->GetName(i), frame); // NOTE(Mara): This is too expensive to be in the inner loop.
								}
							}
						}
					}
				}

				BinaryMove[pivot][frame] = move;
			}
		}

		delete new_hierarchy;
	}

	void AnimationSave::CopyTransform(int bone, int frame, Matrix3& transform)
	{
		Transforms[bone][frame] = transform;
		HasData[bone] = true;
	}

	void AnimationSave::SetAngles(int bone, int frame, float xrot, float yrot, float zrot)
	{
		if (frame > 0)
		{
			float x = xrot + 3.1415927f;
			float y = 3.1415927f - yrot;
			float z = zrot + 3.1415927f;

			if (x > 3.1415926535f)
			{
				x = x - 6.283185307f;
			}

			if (y > 3.1415926535f)
			{
				y = y - 6.283185307f;
			}

			if (z > 3.1415926535f)
			{
				z = z - 6.283185307f;
			}

			float xangle = Angles[bone][frame - 1].x;
			float yangle = Angles[bone][frame - 1].y;
			float zangle = Angles[bone][frame - 1].z;

			if ((z - zangle) * (z - zangle) + (y - yangle) * (y - yangle) + (x - xangle) * (x - xangle) < (zrot - zangle) * (zrot - zangle) + (yrot - yangle) * (yrot - yangle) + (xrot - xangle) * (xrot - xangle))
			{
				xrot = x;
				yrot = y;
				zrot = z;
			}
		}

		Angles[bone][frame].x = xrot;
		Angles[bone][frame].y = yrot;
		Angles[bone][frame].z = zrot;
		HasData[bone] = true;
	}

	void AnimationSave::CopyVisibility(int bone, int frame, bool vis, float floatvis)
	{
		BitChannels[bone][frame] = vis;
		VisibilityChannels[bone][frame] = floatvis;
		HasData[bone] = true;
	}

	Matrix3 AnimationSave::GetTransform(int node, int frame)
	{
		return Transforms[node][frame];
	}

#ifndef W3X
	bool AnimationSave::WriteAnimationHeader(ChunkSaveClass& csave)
	{
		LogDataDialogClass::WriteLogWindow(L"Save Header Type: ");
		LogDataDialogClass::WriteLogWindow(L"Non-Compressed.\n");

		if (!csave.Begin_Chunk(W3DChunkType::ANIMATION_HEADER))
		{
			return false;
		}

		W3dAnimHeaderStruct header;
		header.Version = 0x40001;
		CopyW3DName(header.Name, Name);
		CopyW3DName(header.HierarchyName, Hierarchy->GetHierarchyName());
		header.NumFrames = NumFrames;
		header.FrameRate = FrameRate;

		if (csave.Write(&header, sizeof(W3dAnimHeaderStruct)) != sizeof(W3dAnimHeaderStruct))
		{
			return false;
		}

		if (!csave.End_Chunk())
		{
			return false;
		}

		return true;
	}
#else
	bool AnimationSave::WriteAnimationHeader(XMLWriter& csave)
	{
		char name[256];
		CopyW3DName(name, Name);
		char hname[256];
		CopyW3DName(hname, Hierarchy->GetHierarchyName());

		if (csave.SetStringAttribute("id", name) && csave.SetStringAttribute("Hierarchy", hname), csave.SetUnsignedIntAttribute("NumFrames", NumFrames), csave.SetUnsignedIntAttribute("FrameRate", FrameRate) && csave.EndTag())
		{
			if (!ExportStr->NonDefaultCompressionSettings || (csave.StartTag("CompressionSettings", 0) && csave.SetBoolAttribute("AllowTimeCoded", ExportStr->CompressionTypes & 1) && csave.SetBoolAttribute("AllowAdaptiveDelta", (ExportStr->CompressionTypes >> 1) & 1) && csave.SetFloatAttribute("MaxTranslationError", ExportStr->MaxTranslationError) && csave.SetFloatAttribute("MaxRotationError", ExportStr->MaxRotationError) && csave.SetFloatAttribute("MaxVisibilityError", ExportStr->MaxVisibilityError) && csave.SetFloatAttribute("MaxAdaptiveDeltaError", ExportStr->MaxAdaptiveDeltaError) && csave.SetFloatAttribute("ForceReductionRate", !ExportStr->ForceKeyReduction ? 1.0f : ExportStr->KeyReduction / 100.0f) && csave.EndTag()))
			{
				return true;
			}
		}

		return false;
	}
#endif

	class BitChannel
	{
		int Pivot;
		int Flags;
		int NumFrames;
		bool HasNoData;
		bool DefaultVal;
		BooleanVectorClass Vector;
		int FirstFrame;
		int LastFrame;

	public:
		bool GetData(int frame);
		void UpdateStartEnd();
		void SetData(int frame, bool data);
		void Init(BooleanVectorClass& vector);
#ifndef W3X
		bool WriteChannel(ChunkSaveClass& csave);
#endif
		BitChannel(int pivot, int numframes, int flags, bool defaultval);
		~BitChannel();

		friend class AnimationSave;
	};

	bool BitChannel::GetData(int frame)
	{
		return Vector[frame];
	}

	void BitChannel::UpdateStartEnd()
	{
		for (FirstFrame = 0; FirstFrame < NumFrames; FirstFrame++)
		{
			if (Vector[FirstFrame] != DefaultVal)
			{
				break;
			}
		}

		for (LastFrame = NumFrames - 1; LastFrame >= 0; LastFrame--)
		{
			if (Vector[LastFrame] != DefaultVal)
			{
				break;
			}
		}
	}

	void BitChannel::SetData(int frame, bool data)
	{
		Vector[frame] = data;

		if (data != DefaultVal)
		{
			HasNoData = false;
		}
	}

	void BitChannel::Init(BooleanVectorClass& vector)
	{
		for (int i = 0; i < vector.Length(); i++)
		{
			SetData(i, vector[i]);
		}
	}

#ifndef W3X
	bool BitChannel::WriteChannel(ChunkSaveClass& csave)
	{
		if (HasNoData)
		{
			return true;
		}

		if (!csave.Begin_Chunk(W3DChunkType::BIT_CHANNEL))
		{
			return false;
		}

		UpdateStartEnd();
		int length = (LastFrame - FirstFrame + 8) / 8 + 9;
		char* c = new char[length];
		W3dBitChannelStruct* w = (W3dBitChannelStruct*)c;

		if (!w)
		{
			return false;
		}

		w->FirstFrame = FirstFrame;
		w->LastFrame = LastFrame;
		w->Flags = Flags;
		w->Pivot = Pivot;
		w->DefaultVal = DefaultVal;

		for (int i = 0; i < LastFrame - FirstFrame + 1; i++)
		{
			Set_Bit(w->Data, i, Vector[i + FirstFrame]);
		}

		if (csave.Write(w, length) != length)
		{
			return false;
		}

		delete[] c;
		return csave.End_Chunk();
	}
#endif

	BitChannel::BitChannel(int pivot, int numframes, int flags, bool defaultval) : Pivot(pivot), Flags(flags), NumFrames(numframes), HasNoData(true), DefaultVal(defaultval), Vector(numframes), FirstFrame(NumFrames), LastFrame(0)
	{
	}

	BitChannel::~BitChannel()
	{
	}

	class AnimChannel
	{
		int Pivot;
		int Flags;
		int NumFrames;
		int VectorLen;
		bool HasNoData;
		float* DefaultVector;
		float* Vector;
		int FirstFrame;
		int LastFrame;

	public:
		AnimChannel(int pivot, int numframes, int flags, int vectorlen, float* defaultvector);
		~AnimChannel();
		bool IsDefault(float* value);
		void UpdateStartEnd();
		void SetData(int frame, float* data);
		void ApplyVisibility(BitChannel& channel);
#ifndef W3X
		bool WriteChannel(ChunkSaveClass& csave, BitChannel* bmove);
#else
		bool WriteChannel(XMLWriter& csave, BitChannel* bmove);
#endif

		friend class AnimationSave;
	};

	AnimChannel::AnimChannel(int pivot, int numframes, int flags, int vectorlen, float* defaultvector) : Pivot(pivot), Flags(flags), NumFrames(numframes), VectorLen(vectorlen), HasNoData(true), DefaultVector(nullptr), Vector(nullptr), FirstFrame(0), LastFrame(0)
	{
		DefaultVector = new float[vectorlen];
		Vector = new float[vectorlen * numframes];
		memcpy(DefaultVector, defaultvector, vectorlen * 4);
		memset(Vector, 0, 4 * VectorLen * NumFrames);
		FirstFrame = NumFrames;
		LastFrame = 0;
	}

	AnimChannel::~AnimChannel()
	{
		if (Vector)
		{
			delete[] Vector;
		}

		if (DefaultVector)
		{
			delete[] DefaultVector;
		}
	}

	bool AnimChannel::IsDefault(float* value)
	{
		float f = 0.0f;

		for (int i = VectorLen; i; i--)
		{
			float f2 = value[i - 1] - DefaultVector[i - 1];
			f = f + f2 * f2;
		}

		if (f < 0.0000000025f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void AnimChannel::UpdateStartEnd()
	{
		for (FirstFrame = 0; FirstFrame < NumFrames; FirstFrame++)
		{
			if (!IsDefault(&Vector[FirstFrame * VectorLen]))
			{
				break;
			}
		}

		for (LastFrame = NumFrames - 1; LastFrame >= 0; LastFrame--)
		{
			if (!IsDefault(&Vector[LastFrame * VectorLen]))
			{
				break;
			}
		}
	}

	void AnimChannel::SetData(int frame, float* data)
	{
		for (int i = 0; i < VectorLen; i++)
		{
			Vector[i + frame * VectorLen] = data[i];
		}

		if (!IsDefault(data))
		{
			HasNoData = false;
		}
	}

	void AnimChannel::ApplyVisibility(BitChannel& channel)
	{
		bool data = channel.GetData(0);
		float* vector = Vector;

		for (int i = 0; i < NumFrames; i++)
		{
			bool b = channel.GetData(i);

			if (b != data)
			{
				data = b;
				vector = &Vector[i * VectorLen];
			}

			if (!b)
			{
				for (int j = 0; j < VectorLen; j++)
				{
					Vector[j + i * VectorLen] = vector[j];
				}

				if (!IsDefault(vector))
				{
					HasNoData = false;
				}
			}
		}
	}

#ifndef W3X
	bool AnimChannel::WriteChannel(ChunkSaveClass& csave, BitChannel* bmove)
	{
		if (HasNoData)
		{
			return true;
		}

		UpdateStartEnd();

		if (LastFrame < FirstFrame)
		{
			HasNoData = true;
			return true;
		}

		if (!csave.Begin_Chunk(W3DChunkType::ANIMATION_CHANNEL))
		{
			return false;
		}

		int frames = LastFrame - FirstFrame + 1;
		int frames2 = 4 * frames * VectorLen + 12;
		char* c = new char[frames2];
		W3dAnimChannelStruct* w = (W3dAnimChannelStruct*)c;

		if (!w)
		{
			return false;
		}

		w->FirstFrame = FirstFrame;
		w->LastFrame = LastFrame;
		w->VectorLen = VectorLen;
		w->Pivot = Pivot;
		w->Flags = Flags;
		w->pad = 0; // fill padding with zeros to have a deterministic output

		for (int i = 0; i < frames; i++)
		{
			for (int j = 0; j < VectorLen; j++)
			{
				w->Data[j + i * VectorLen] = Vector[j + VectorLen * (i + FirstFrame)];
			}
		}

		if (csave.Write(w, frames2) == frames2)
		{
			delete[] w;
			return csave.End_Chunk();
		}

		return false;
	}
#else

	bool AnimChannel::WriteChannel(XMLWriter& csave, BitChannel* bmove)
	{
		if (HasNoData)
		{
			return true;
		}

		UpdateStartEnd();

		if (LastFrame < FirstFrame)
		{
			HasNoData = true;
			return true;
		}

		const char* typestr = "ChannelQuaternion";

		if (Flags != ANIM_CHANNEL_Q)
		{
			typestr = "ChannelScalar";
		}

		if (!csave.StartTag(typestr, 1) || !csave.SetUnsignedIntAttribute("Pivot", Pivot))
		{
			return false;
		}

		const char* type;

		switch (Flags)
		{
		case ANIM_CHANNEL_X:
			type = "XTranslation";
			break;
		case ANIM_CHANNEL_Y:
			type = "YTranslation";
			break;
		case ANIM_CHANNEL_Z:
			type = "ZTranslation";
			break;
		case ANIM_CHANNEL_Q:
			type = "Orientation";
			break;
		case ANIM_CHANNEL_VIS:
			type = "Visibility";
			break;
		default:
			throw ErrorClass(L"Trying to export unsupported channel type: %d", Flags);
		}

		if (!csave.SetStringAttribute("Type", type) || !csave.SetIntAttribute("FirstFrame", FirstFrame) || !csave.EndTag())
		{
			return false;
		}

		int frames = LastFrame - FirstFrame + 1;

		for (int i = 0; i < frames; i++)
		{
			if (Flags == ANIM_CHANNEL_Q)
			{
				if (!csave.StartTag("Frame", 0) || !csave.SetFloatAttribute("X", Vector[VectorLen * i + FirstFrame]) || !csave.SetFloatAttribute("Y", Vector[VectorLen * i + FirstFrame + 1]) || !csave.SetFloatAttribute("Z", Vector[VectorLen * i + FirstFrame + 2]) || !csave.SetFloatAttribute("W", Vector[VectorLen * i + FirstFrame + 3]))
				{
					return false;
				}

				if (bmove && bmove->GetData(i) && !csave.SetStringAttribute("BinaryMove", "true"))
				{
					return false;
				}

				if (!csave.EndTag())
				{
					return false;
				}
			}
			else
			{
				if (!csave.StartTag("Frame", 1))
				{
					return false;
				}

				if ((bmove && bmove->GetData(i) && !csave.SetStringAttribute("BinaryMove", "true")) || !csave.EndTag() || !csave.WriteFormatted("%f", Vector[VectorLen * (i + FirstFrame)]))
				{
					return false;
				}

				if (!csave.WriteClosingTag())
				{
					return false;
				}
			}
		}

		return csave.WriteClosingTag();
	}
#endif

#ifndef W3X
	bool AnimationSave::WriteAnimationChannels(ChunkSaveClass& csave)
#else
	bool AnimationSave::WriteAnimationChannels(XMLWriter& csave)
#endif
	{
		int BoneCount = Hierarchy->GetBoneCount();
		LogDataDialogClass::WriteLogWindow(L"\nSaving Channel Data for %d Nodes\n", Hierarchy->GetBoneCount());

#ifdef W3X
		if (!csave.StartTag("Channels", 1) || !csave.EndTag())
		{
			return false;
		}
#endif

		float data[4];
		data[0] = 0.0f;

		for (int i = 0; i < BoneCount; i++)
		{
			LogDataDialogClass::WriteLogWindow(L"\nnode: %d ", i);

			if (HasData[i])
			{
				float defaultvector[4];
				defaultvector[0] = 0.0f;
				defaultvector[1] = 0.0f;
				defaultvector[2] = 0.0f;
				defaultvector[3] = 1.0f;
				float defaultvis = 1.0;

				AnimChannel xchannel(i, NumFrames, ANIM_CHANNEL_X, 1, defaultvector);
				AnimChannel ychannel(i, NumFrames, ANIM_CHANNEL_Y, 1, defaultvector);
				AnimChannel zchannel(i, NumFrames, ANIM_CHANNEL_Z, 1, defaultvector);
				AnimChannel xrchannel(i, NumFrames, ANIM_CHANNEL_XR, 1, defaultvector);
				AnimChannel yrchannel(i, NumFrames, ANIM_CHANNEL_YR, 1, defaultvector);
				AnimChannel zrchannel(i, NumFrames, ANIM_CHANNEL_ZR, 1, defaultvector);
				AnimChannel qchannel(i, NumFrames, ANIM_CHANNEL_Q, 4, defaultvector);
				BitChannel vchannel(i, NumFrames, BIT_CHANNEL_VIS, true);
				vchannel.Init(BitChannels[i]);
				AnimChannel vfchannel(i, NumFrames, ANIM_CHANNEL_VIS, 1, &defaultvis);
				BitChannel bmovechannel(i, NumFrames, BIT_CHANNEL_VIS, false);
				bmovechannel.Init(BinaryMove[i]);

				for (int j = 0; j < NumFrames; j++)
				{
					Matrix3 tm = GetTransform(i, j);
					Point3 angles = Angles[i][j];
					Quat q;
					Point3 p;
					Point3 s;
					DecomposeMatrix(tm, p, q, s);
					q.x = -q.x;
					q.y = -q.y;
					q.z = -q.z;

					data[0] = p.x;
					xchannel.SetData(j, data);
					data[0] = p.y;
					ychannel.SetData(j, data);
					data[0] = p.z;
					zchannel.SetData(j, data);
					data[0] = angles.x;
					xrchannel.SetData(j, data);
					data[0] = angles.y;
					yrchannel.SetData(j, data);
					data[0] = angles.z;
					zrchannel.SetData(j, data);

					data[0] = q.x;
					data[1] = q.y;
					data[2] = q.z;
					data[3] = q.w;
					qchannel.SetData(j, data);
					vchannel.SetData(j, BitChannels[i][j]);
					float vis = VisibilityChannels[i][j];
					vfchannel.SetData(j, &vis);
					bmovechannel.SetData(j, BinaryMove[i][j]);
				}

				if (!vchannel.HasNoData)
				{
					if (!xchannel.HasNoData)
					{
						xchannel.ApplyVisibility(vchannel);
					}

					if (!ychannel.HasNoData)
					{
						ychannel.ApplyVisibility(vchannel);
					}

					if (!zchannel.HasNoData)
					{
						zchannel.ApplyVisibility(vchannel);
					}

					if (!qchannel.HasNoData)
					{
						qchannel.ApplyVisibility(vchannel);
					}

					if (!vfchannel.HasNoData)
					{
						vfchannel.ApplyVisibility(vchannel);
					}
				}

				if (!xchannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"x");
					xchannel.WriteChannel(csave, &bmovechannel);
				}

				if (!ychannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"y");
					ychannel.WriteChannel(csave, &bmovechannel);
				}

				if (!zchannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"z");
					zchannel.WriteChannel(csave, &bmovechannel);
				}

				if (!qchannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"q");
					qchannel.WriteChannel(csave, &bmovechannel);
				}

#ifndef W3X
				if (!vchannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"v");
					vchannel.WriteChannel(csave);
				}
#endif

				if (!vfchannel.HasNoData)
				{
					LogDataDialogClass::WriteLogWindow(L"vf");
					vfchannel.WriteChannel(csave, nullptr);
				}
			}
		}

#ifdef W3X
		if (!csave.WriteClosingTag())
		{
			return false;
		}
#endif

		LogDataDialogClass::WriteLogWindow(L"\n\nSave Channel Data Complete.\n");
		return true;
	}

	AnimationSave::~AnimationSave()
	{
		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			if (Transforms[i])
			{
				delete[] Transforms[i];
			}
		}

		if (Transforms)
		{
			delete[] Transforms;
		}

		for (int i = 0; i < Hierarchy->GetBoneCount(); i++)
		{
			if (Angles[i])
			{
				delete[] Angles[i];
			}
		}

		if (Angles)
		{
			delete[] Angles;
		}

		if (BitChannels)
		{
			delete[] BitChannels;
		}

		if (VisibilityChannels)
		{
			delete[] VisibilityChannels;
		}

		if (BinaryMove)
		{
			delete[] BinaryMove;
		}

		LogDataDialogClass::WriteLogWindow(L"Destroy Log..%d,%d,%d,%d, %S..\n", 1, 2, 3, 4, "go");
	}

#ifndef W3X
	bool AnimationSave::WriteAnimation(ChunkSaveClass& csave)
	{
		LogDataDialogClass::WriteLogWindow(L"\nBegin Save Motion Data\n");

		if (csave.Begin_Chunk(W3DChunkType::ANIMATION) && AnimationSave::WriteAnimationHeader(csave) && AnimationSave::WriteAnimationChannels(csave))
		{
			return csave.End_Chunk();
		}

		return false;
	}
#else
	bool AnimationSave::WriteAnimation(XMLWriter& csave)
	{
		LogDataDialogClass::WriteLogWindow(L"\nBegin Save Motion Data\n");
		return csave.StartTag("W3DAnimation", 1) && WriteAnimationHeader(csave) && WriteAnimationChannels(csave) && csave.WriteClosingTag();
	}
#endif

	HierarchySave* W3DExport::ImportHierarchy(const char* filename)
	{
#ifndef W3X
		BufferedFileClass file(filename);

		if (!file.Open(true))
		{
			return nullptr;
		}

		ChunkLoadClass cload(&file);
		cload.Open_Chunk();

		if (cload.Cur_Chunk_ID() != enum_to_value(W3DChunkType::HIERARCHY))
		{
			file.Close();
			return nullptr;
		}

		HierarchySave* hierarchy = new HierarchySave();
		hierarchy->LoadHierarchy(cload);
		cload.Close_Chunk();
		file.Close();
		return hierarchy;
#else
		HierarchySave* hierarchy = new HierarchySave();

		if (!hierarchy->LoadHierarchy(filename))
		{
			delete hierarchy;
			hierarchy = 0;
		}

		return hierarchy;
#endif
	}

	HierarchySave* W3DExport::GetHierarchy()
	{
		HierarchySave* hierarchy = HierarchyStruct;

		if (!hierarchy)
		{
			if (!m_Settings.ExportSkeleton)
			{
				hierarchy = ImportHierarchy(SkeletonPath);
				HierarchyStruct = hierarchy;

				if (!hierarchy)
				{
					WideStringClass str;
					str.Format(L"Unable to load hierarchy file: %S\nIf this Max file has been moved, please re-select the hierarchy file.", SkeletonPath);
					MessageBox(Int->GetMAXHWnd(), str, L"Error", MB_SETFOREGROUND);
				}
			}
		}

		return hierarchy;
	}

#ifndef W3X
	bool W3DExport::ExportAnimation(const char* name, ChunkSaveClass& csave, INode* node)
#else
	bool W3DExport::ExportAnimation(const char* name, XMLWriter& csave, INode* node)
#endif
	{
		if (!m_Settings.ExportAnimation)
		{
			return true;
		}

		TT_PROFILER_SCOPE("W3DExport::ExportAnimation");
		HierarchySave* pose = GetHierarchy();

		if (node && pose)
		{
			Matrix3 mat;
			AnimationSave* anim;

			try
			{
				anim = new AnimationSave(ExpInt->theScene, node, pose, &m_Settings, FrameRate, name, mat);
			}
			catch (ErrorClass& e)
			{
				MessageBox(nullptr, e.GetError(), L"Error", MB_SETFOREGROUND);
				return false;
			}

			anim->WriteAnimation(csave);
			delete anim;
			return true;
		}

		return false;
	}

	bool FindDuplicateNodes(INodeListClass* list)
	{
		TT_PROFILER_SCOPE("W3DExport::FindDuplicateNodes");
		std::unordered_set<WideStringClass, hash_wstring, equals_wstring> names;
		names.reserve(list->GetNodeCount());

		for (INode* node : *list)
		{
			// NOTE(Mara): Even with extra allocations this is faster than converting each character to lower case one-by-one in the hash/equals function.
			WideStringClass name = node->GetName();
			_wcslwr(name.Peek_Buffer());

#ifndef W3X
			if (!IsAggregate(node)) // aggregates don't care about being duplicates, as per the original code
#endif
			{
				if (names.find(name) != names.end())
				{
					WideStringClass str;
					str.Format(L"Geometry Nodes with duplicated names found!\nDuplicated Name: %s\n", name.Peek_Buffer());
					MessageBox(nullptr, str, L"Error", MB_SETFOREGROUND);
					return true;
				}
			}

			names.emplace(std::move(name));
		}

		return false;
	}

#ifndef W3X
	bool W3DExport::ExportGeometry(const char* name, ChunkSaveClass& csave, INode* node, MeshConnection** connection)
#else
	bool W3DExport::ExportGeometry(const char* name, XMLWriter& csave, INode* node, MeshConnection** connection)
#endif
	{
		if (!m_Settings.ExportGeometry)
		{
			return true;
		}

		TT_PROFILER_SCOPE("W3DExport::ExportGeometry");
		HierarchySave* hierarchy = nullptr;

		if (m_Settings.UseExistingSkeleton || m_Settings.ExportSkeleton)
		{
			hierarchy = GetHierarchy();
		}

		DynamicVectorClass<GeometryExportTaskClass*> v;
		GeometryFilterClass filter;
		TT_PROFILER_SCOPE_START("Create Geo Node List");
		INodeListClass* list = new INodeListClass(node, Time, &filter);
		TT_PROFILER_SCOPE_STOP();

		if (!FindDuplicateNodes(list))
		{
			MaxWorldInfoClass info(&v);
			info.Set_Are_Meshes_Smoothed(m_Settings.SmoothVertexNormals);
#ifndef W3X
			LodData lod(name, &csave, &info, &m_Settings, hierarchy, node, CreateOriginNodeList(), Time);
#else
			LodData lod(name, &csave, &includes, &info, &m_Settings, hierarchy, node, CreateOriginNodeList(), Time);
#endif
			int count = list->GetNodeCount();

			if (!hierarchy && count > 1)
			{
				count = 1;
				LogDataDialogClass::WriteLogWindow(L"\nDiscarding extra meshes since we are not exporting a hierarchical model.\n");
			}

			v.Resize(count);

			{
				TT_PROFILER_SCOPE("Create Geo Export Tasks");
				for (int i = 0; i < count; i++)
				{
					GeometryExportTaskClass* c = CreateGeometryExportTask(list->GetNode(i), lod);
					if (c)
					{
						v.Add(c);
					}
				}
			}

			if (v.Count() == 1)
			{
				if (!hierarchy)
				{
					strncpy((v[0])->Name, name, W3D_NAME_LEN);
					strncpy((v[0])->ContainerName, "", W3D_NAME_LEN);
					goto l1;
				}
			}
			else if (!hierarchy)
			{
			l1:
				for (int i = 0; i < v.Count(); i++)
				{
					try
					{
						v[i]->Save(lod);
					}
					catch (ErrorClass& e)
					{
						MessageBox(nullptr, e.GetError(), L"Error", MB_SETFOREGROUND);
					}
				}

				for (int i = 0; i < v.Count(); i++)
				{
					if (v[i])
					{
						delete v[i];
					}
				}

				v.Delete_All();

				if (list)
				{
					delete list;
				}

				return true;
			}

			try
			{
				*connection = new MeshConnection(v, lod);
			}
			catch (ErrorClass& e)
			{
				MessageBox(nullptr, e.GetError(), L"Error", MB_SETFOREGROUND);
				return false;
			}

			goto l1;
		}

		return false;
	}

	class HLodSave
	{
		class HLodSubObjectArray
		{
		public:
			W3dHLodArrayHeaderStruct Header;
			W3dHLodSubObjectStruct* SubObjects;
			int SubObjectCount;

			HLodSubObjectArray() : Header(), SubObjects(nullptr), SubObjectCount(0)
			{
			}

			~HLodSubObjectArray()
			{
				delete[] SubObjects;
			}
		};

		W3dHLodHeaderStruct Header;
		HLodSubObjectArray* Lods;
		HLodSubObjectArray Aggregates;
		HLodSubObjectArray Proxies;
	public:
		HLodSave(MeshConnection** connections, int nodecount, TimeValue time, const char* name, const char* hierarchyname);
		~HLodSave();
#ifndef W3X
		bool SaveHeader(ChunkSaveClass& csave);
		bool SaveSubSubobjectArray(ChunkSaveClass& csave, HLodSubObjectArray& subobj);
		bool SaveLodArray(ChunkSaveClass& csave);
		bool SaveAggregateArray(ChunkSaveClass& csave);
		bool SaveProxyArray(ChunkSaveClass& csave);
		bool Save(ChunkSaveClass& csave);
#else
		bool SaveHeader(XMLWriter& csave);
		bool SaveLodArray(XMLWriter& csave);
		bool Save(XMLWriter& csave);
#endif
	};

	HLodSave::HLodSave(MeshConnection** connections, int nodecount, TimeValue time, const char* name, const char* hierarchyname) : Header(), Lods(nullptr)
	{
		TT_PROFILER_SCOPE("HLodSave::HLodSave");
		Header.Version = 0x10000;
		Header.LodCount = nodecount;
		CopyW3DName(Header.Name, name);
		CopyW3DName(Header.HierarchyName, hierarchyname);
		LogDataDialogClass::WriteLogWindow(L"\nExporting HLOD object: %S\n", Header.Name);
		LogDataDialogClass::WriteLogWindow(L" lod count: %u\n", Header.LodCount);
		Lods = new HLodSubObjectArray[nodecount];

		if (!Lods)
		{
			throw ErrorClass(L"Out Of Memory!");
		}

		{
			TT_PROFILER_SCOPE("Get Sub-Objects");

			for (int i = 0; i < nodecount; i++)
			{
				LogDataDialogClass::WriteLogWindow(L" Exporting LOD Array %d\n", i);
				int count = connections[i]->Meshes.Count();
				INode* node = connections[i]->Node;

				if (count > 0)
				{
					Lods[i].SubObjects = new W3dHLodSubObjectStruct[count];
					Lods[i].SubObjectCount = count;
					Lods[i].Header.ModelCount = count;
#ifndef W3X
					float size = 1.0f;
#else
					float size = WWMATH_FLOAT_MAX;
#endif

					if (node)
					{
#ifndef W3X
						size = GetScreenSizeFromNode(node);
#else
						node->GetUserPropFloat(L"MaxScreenSize", size);
#endif
					}

					Lods[i].Header.MaxScreenSize = size;
					LogDataDialogClass::WriteLogWindow(L" sub-object count: %d\n", count);

					for (int j = 0; j < count; j++)
					{
						const char* subobjname;
						int subobjbone;
						INode* subobjnode;

#ifndef W3X
						if (!connections[i]->GetMeshConnectionInfo(j, &subobjname, &subobjbone, &subobjnode))
#else
						int type;
						if (!connections[i]->GetMeshConnectionInfo(j, &subobjname, &subobjbone, &subobjnode, &type))
#endif
						{
							throw ErrorClass(L"Model %S  is missing connection data!", name);
						}

						strncpy(Lods[i].SubObjects[j].Name, subobjname, W3D_NAME_LEN * 2);
						Lods[i].SubObjects[j].BoneIndex = subobjbone;
#ifdef W3X
						Lods[i].SubObjects[j].Type = type;
#endif
						LogDataDialogClass::WriteLogWindow(L"  Sub Object: %S Bone: %d\n", subobjname, subobjbone); // NOTE(Mara): This is too expensive to be in the inner loop.
					}
				}
			}
		}

		int count = connections[nodecount - 1]->Aggregates.Count();

		if (count > 0)
		{
			TT_PROFILER_SCOPE("Get Aggregates");
			Aggregates.SubObjects = new W3dHLodSubObjectStruct[count];
			Aggregates.SubObjectCount = count;
			Aggregates.Header.ModelCount = count;
			Aggregates.Header.MaxScreenSize = 0.0f;
			LogDataDialogClass::WriteLogWindow(L" Exporting Aggregates:\n");
			LogDataDialogClass::WriteLogWindow(L" aggregate count: %d\n", count);

			for (int j = 0; j < count; j++)
			{
				const char* subobjname;
				int subobjbone;
				INode* subobjnode;
#ifndef W3X
				connections[nodecount - 1]->GetAggregateConnectionInfo(j, &subobjname, &subobjbone, &subobjnode);
#else
				int type;
				connections[nodecount - 1]->GetAggregateConnectionInfo(j, &subobjname, &subobjbone, &subobjnode, &type);
#endif
				strncpy(Aggregates.SubObjects[j].Name, subobjname, W3D_NAME_LEN * 2);
				Aggregates.SubObjects[j].BoneIndex = subobjbone;
#ifdef W3X
				Aggregates.SubObjects[j].Type = type;
#endif
				LogDataDialogClass::WriteLogWindow(L"  Aggregate object: %S Bone: %d\n", subobjname, subobjbone); // NOTE(Mara): This is too expensive to be in the inner loop.
			}
		}

		count = connections[nodecount - 1]->Proxies.Count();

		if (count > 0)
		{
			TT_PROFILER_SCOPE("Get Proxies");
			Proxies.SubObjects = new W3dHLodSubObjectStruct[count];
			Proxies.SubObjectCount = count;
			Proxies.Header.ModelCount = count;
			Proxies.Header.MaxScreenSize = 0.0f;
			LogDataDialogClass::WriteLogWindow(L" Exporting Proxies\n");
			LogDataDialogClass::WriteLogWindow(L" proxy count: %d\n", count);

			for (int j = 0; j < count; j++)
			{
				const char* subobjname;
				int subobjbone;
				INode* subobjnode;
#ifndef W3X
				connections[nodecount - 1]->GetProxyConnectionInfo(j, &subobjname, &subobjbone, &subobjnode);
#else
				int type;
				connections[nodecount - 1]->GetProxyConnectionInfo(j, &subobjname, &subobjbone, &subobjnode, &type);
#endif
				strncpy(Proxies.SubObjects[j].Name, subobjname, W3D_NAME_LEN * 2);
				Proxies.SubObjects[j].BoneIndex = subobjbone;
#ifdef W3X
				Proxies.SubObjects[j].Type = type;
#endif
				LogDataDialogClass::WriteLogWindow(L"  Proxy object: %S Bone: %d\n", subobjname, subobjbone); // NOTE(Mara): This is too expensive to be in the inner loop.
			}
		}
	}

	HLodSave::~HLodSave()
	{
		if (Lods)
		{
			delete[] Lods;
		}
	}

#ifndef W3X
	bool HLodSave::SaveHeader(ChunkSaveClass& csave)
	{
		return csave.Begin_Chunk(W3DChunkType::HLOD_HEADER) && csave.Write(&Header, sizeof(W3dHLodHeaderStruct)) == sizeof(W3dHLodHeaderStruct) && csave.End_Chunk();
	}

	bool HLodSave::SaveSubSubobjectArray(ChunkSaveClass& csave, HLodSubObjectArray& subobj)
	{
		if (csave.Begin_Chunk(W3DChunkType::HLOD_SUB_OBJECT_ARRAY_HEADER))
		{
			if (csave.Write(&subobj.Header, sizeof(W3dHLodArrayHeaderStruct)) == sizeof(W3dHLodArrayHeaderStruct) && csave.End_Chunk())
			{
				for (int i = 0; i < subobj.SubObjectCount; i++)
				{
					if (!csave.Begin_Chunk(W3DChunkType::HLOD_SUB_OBJECT) || !(csave.Write(&subobj.SubObjects[i], sizeof(W3dHLodSubObjectStruct)) == sizeof(W3dHLodSubObjectStruct)) || !csave.End_Chunk())
					{
						return false;
					}
				}

				return true;
			}
		}

		return false;
	}

	bool HLodSave::SaveLodArray(ChunkSaveClass& csave)
	{
		TT_PROFILER_SCOPE("HLodSave::SaveLodArray");

		for (unsigned int i = 0; i < Header.LodCount; i++)
		{
			if (!csave.Begin_Chunk(W3DChunkType::HLOD_LOD_ARRAY) || !SaveSubSubobjectArray(csave, Lods[i]) || !csave.End_Chunk())
			{
				return false;
			}
		}

		return true;
	}

	bool HLodSave::SaveAggregateArray(ChunkSaveClass& csave)
	{
		TT_PROFILER_SCOPE("HLodSave::SaveAggregateArray");
		return Aggregates.SubObjectCount <= 0 || csave.Begin_Chunk(W3DChunkType::HLOD_AGGREGATE_ARRAY) && SaveSubSubobjectArray(csave, Aggregates) && csave.End_Chunk();
	}

	bool HLodSave::SaveProxyArray(ChunkSaveClass& csave)
	{
		TT_PROFILER_SCOPE("HLodSave::SaveProxyArray");
		return Proxies.SubObjectCount <= 0 || csave.Begin_Chunk(W3DChunkType::HLOD_PROXY_ARRAY) && SaveSubSubobjectArray(csave, Proxies) && csave.End_Chunk();
	}

	bool HLodSave::Save(ChunkSaveClass& csave)
	{
		TT_PROFILER_SCOPE("HLodSave::Save");

		if (!Lods)
		{
			return false;
		}

		if (csave.Begin_Chunk(W3DChunkType::HLOD) && SaveHeader(csave) && SaveLodArray(csave) && SaveAggregateArray(csave) && SaveProxyArray(csave))
		{
			return csave.End_Chunk();
		}

		return false;
	}
#else
	bool HLodSave::SaveHeader(XMLWriter& csave)
	{
		return csave.SetStringAttribute("id", Header.Name) && csave.SetStringAttribute("Hierarchy", Header.HierarchyName);
	}

	bool HLodSave::SaveLodArray(XMLWriter& csave)
	{
		if (Header.LodCount != 1)
		{
			throw ErrorClass(L"Only one LOD level supported for hierarchies now");
		}

		for (int i = 0; i < Lods->SubObjectCount; i++)
		{
			if (!csave.StartTag("SubObject", 1))
			{
				return false;
			}

			char* s1 = strrchr(Lods->SubObjects[i].Name, '.');
			char* s2 = s1 ? s1 + 1 : Lods->SubObjects[i].Name;

			if (!csave.SetStringAttribute("SubObjectID", s2) || !csave.SetUnsignedIntAttribute("BoneIndex", Lods->SubObjects[i].BoneIndex) || !csave.EndTag() || !csave.StartTag("RenderObject", 1) || !csave.EndTag())
			{
				return false;
			}

			bool b;

			switch (Lods->SubObjects[i].Type)
			{
			case 0:
				b = csave.WriteString("Mesh", Lods->SubObjects[i].Name);
				break;
			case 1:
				b = csave.WriteString("CollisionBox", Lods->SubObjects[i].Name);
				break;
			default:
				throw ErrorClass(L"Unrecognized sub object type (%d) for object %S", Lods->SubObjects[i].Type, Lods->SubObjects[i].Name);
				break;
			}

			if (!b || !csave.WriteClosingTag() || !csave.WriteClosingTag())
			{
				return false;
			}
		}

		return true;
	}

	bool HLodSave::Save(XMLWriter& csave)
	{
		if (!Lods)
		{
			return false;
		}

		if (csave.StartTag("W3DContainer", 1) && SaveHeader(csave) && csave.EndTag() && SaveLodArray(csave))
		{
			return csave.WriteClosingTag() != 0;
		}

		return false;
	}
#endif

#ifndef W3X
	bool W3DExport::ExportHlod(const char* name, const char* hierarchyname, ChunkSaveClass& csave, MeshConnection** connections, int nodecount)
#else
	bool W3DExport::ExportHlod(const char* name, const char* hierarchyname, XMLWriter& csave, MeshConnection** connections, int nodecount)
#endif
	{
		if (m_Settings.ExportGeometry)
		{
			HLodSave e(connections, nodecount, Time, name, hierarchyname);
			if (!e.Save(csave))
			{
				return false;
			}
		}
		return true;
	}

	ClassDesc2* W3DExportClassDesc::Instance()
	{
		static W3DExportClassDesc s_instance;
		return &s_instance;
	}
}
