#include "general.h"
#include <max.h>
#include <map>
__pragma(push_macro("SAFE_DELETE")) __pragma(push_macro("SAFE_DELETE_ARRAY"))
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#include <rtmax.h>
__pragma(pop_macro("SAFE_DELETE")) __pragma(pop_macro("SAFE_DELETE_ARRAY"))
#include "dxutil.h"
#include <Shlwapi.h>

class texture_key
{
public:
	std::wstring name;
	float gamma;
	texture_key(std::wstring Name, float Gamma) : name(Name), gamma(Gamma)
	{
	}
	bool operator<(const texture_key& that) const
	{
		return std::tie(name, gamma) < std::tie(that.name, that.gamma);
	}
};
template <class T> class ResourceCache
{
public:
	virtual ~ResourceCache()
	{
		auto i = m_mapLoaded.find(texture_key(name, gamma));
		if (i != m_mapLoaded.end())
		{
			while (CompareFileTime(&i->second->filetime, &filetime))
			{
				i++;
				if (i == m_mapLoaded.end())
				{
					break;
				}
			}
			if (i != m_mapLoaded.end())
			{
				m_mapLoaded.erase(i);
			}
		}
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}
	virtual long OnLostDevice() = 0;
	virtual long OnResetDevice() = 0;
	T resource;
	int refcount;
	std::wstring name;
	FILETIME filetime;
	float gamma;
	static long LostDevice()
	{
		bool b = true;
		for (auto i = m_mapLoaded.begin(); i != m_mapLoaded.end(); i++)
		{
			b &= i->second->OnLostDevice() >= 0;
		}
		if (b)
		{
			return S_OK;
		}
		return E_FAIL;
	}
	static long ResetDevice()
	{
		bool b = true;
		for (auto i = m_mapLoaded.begin(); i != m_mapLoaded.end(); i++)
		{
			b &= i->second->OnResetDevice() >= 0;
		}
		if (b)
		{
			return S_OK;
		}
		return E_FAIL;
	}
	static ResourceCache<T>* getExisting(texture_key& key, bool noref)
	{
		FILETIME lastwritetime;
		auto i = m_mapLoaded.find(key);
		if (i == m_mapLoaded.end())
		{
			return nullptr;
		}
		HANDLE handle = INVALID_HANDLE_VALUE;
		for (int i = 0; i < 5; i++)
		{
			handle = CreateFile(key.name.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
			if (handle == INVALID_HANDLE_VALUE)
			{
				Sleep(500);
			}
			if (handle != INVALID_HANDLE_VALUE)
			{
				break;
			}
		}
		if (handle != INVALID_HANDLE_VALUE)
		{
			FlushFileBuffers(handle);
			GetFileTime(handle, nullptr, nullptr, &lastwritetime);
			CloseHandle(handle);
		}
		if (CompareFileTime(&lastwritetime, &i->second->filetime))
		{
			return 0;
		}
		if (!noref)
		{
			i->second->refcount++;
		}
		return i->second;
	}
	ResourceCache(std::wstring const& Name, float Gamma, T const Resource) : resource(Resource), refcount(1), name(Name), gamma(Gamma)
	{
		filetime.dwLowDateTime = 0;
		filetime.dwHighDateTime = 0;
		if (name != L"Buffer")
		{
			HANDLE handle = INVALID_HANDLE_VALUE;
			for (int i = 0; i < 5; i++)
			{
				handle = CreateFile(name.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
				if (handle == INVALID_HANDLE_VALUE)
				{
					Sleep(500);
				}
				if (handle != INVALID_HANDLE_VALUE)
				{
					break;
				}
			}
			if (handle != INVALID_HANDLE_VALUE)
			{
				FlushFileBuffers(handle);
				GetFileTime(handle, nullptr, nullptr, &filetime);
				CloseHandle(handle);
			}
		}
		else
		{
			filetime.dwLowDateTime = 0;
			filetime.dwHighDateTime = 0;
		}
		m_mapLoaded.insert({ texture_key(name, Gamma), this });
	}
	static std::multimap<texture_key, ResourceCache<T>*> m_mapLoaded;
};
std::multimap<texture_key, ResourceCache<ID3DXEffect*>*> ResourceCache<ID3DXEffect*>::m_mapLoaded;
typedef D3DXPARAMETER_TYPE D3D_EFFECT_VARIABLE_TYPE;
typedef D3DXPARAMETER_CLASS D3D_EFFECT_VARIABLE_CLASS;

class CD3DBaseEffect
{
public:
	virtual int GetNumberOfGlobalVariables() = 0;
	virtual char const* GetGlobalVariableName(int parameter) = 0;
	virtual bool IsGlobalVariableValid(char const* name) = 0;
	virtual D3D_EFFECT_VARIABLE_TYPE GetGlobalVariableType(int parameter) = 0;
	virtual D3D_EFFECT_VARIABLE_CLASS GetGlobalVariableClass(int parameter) = 0;
	virtual int GetNumberOfTechniques() = 0;
	virtual bool IsTechniqueValid(int technique) = 0;
	virtual char const* GetTechniqueName(int technique) = 0;
	virtual int GetNumberOfAnnotations(char const* name) = 0;
	virtual char const* GetAnnotationName(char const* name, int i) = 0;
	virtual bool IsAnnotationValid(char const* name, char const* annotation) = 0;
	virtual char const* GetVariableSemantic(char const* name) = 0;
	virtual bool GetString(char const* name, char** value) = 0;
	virtual bool GetString(char const* name, int index, char** value) = 0;
	virtual bool GetString(char const* name, char const* annotation, char** value) = 0;
	virtual bool GetInt(char const* name, int index, int* value) = 0;
	virtual bool GetInt(char const* name, int* value) = 0;
	virtual bool GetInt(char const* name, char const* annotation, int* value) = 0;
	virtual bool GetFloat(char const* name, char const* annotation, float* value) = 0;
	virtual bool GetFloat(char const* name, float* value) = 0;
	virtual bool GetBool(char const* name, int* value) = 0;
	virtual bool GetBool(char const* name, char const* annotation, int* value) = 0;
	virtual bool GetVector(char const* name, D3DXVECTOR4* value) = 0;
};

class CD3DEffect9 : public ResourceCache<ID3DXEffect*>, public CD3DBaseEffect
{
public:
	CD3DEffect9(const wchar_t* name, LPD3DXEFFECT effect, LPD3DXBUFFER errors) : ResourceCache<ID3DXEffect*>(name, 1.0f, effect), CompileErrors(errors)
	{
	}
	~CD3DEffect9()
	{
		if (CompileErrors)
		{
			CompileErrors->Release();
			CompileErrors = nullptr;
		}
	}
	static long Create(IDirect3DDevice9* device, wchar_t const* name, bool isString, CD3DEffect9** effect)
	{
		*effect = nullptr;
		ResourceCache<ID3DXEffect*> *cache = ResourceCache<ID3DXEffect*>::getExisting(texture_key(name, 1.0f), false);
		if (cache)
		{
			*effect = (CD3DEffect9*)cache;
			if (cache->resource)
			{
				return S_OK;
			}
			return E_OUTOFMEMORY;
		}
		LPD3DXEFFECT ppEffect = nullptr;
		LPD3DXBUFFER ppCompilationErrors = nullptr;
		D3DXMACRO pDefines[7];
		pDefines[0].Name = "_3DSMAX_";
		pDefines[0].Definition = "0";
		pDefines[1].Name = "_MAX_";
		pDefines[1].Definition = "0";
		pDefines[2].Name = "MAX";
		pDefines[2].Definition = "0";
		pDefines[3].Name = "3DSMAX";
		pDefines[3].Definition = "0";
		pDefines[4].Name = nullptr;
		pDefines[4].Definition = nullptr;
		WStr Name(name);
		UTF8Str Data;
		WStr FXPath;
		if (isString)
		{
			Data = Name.ToUTF8();
		}
		else if (!MaxDXUtilites::LoadFileToString(Name, Data))
		{
			return E_FAIL;
		}
		WStr MaxPath;
		MaxPath.Resize(4096);
		GetModuleFileName(nullptr, MaxPath.dataForWrite(), 4095);
		PathRemoveFileSpec(MaxPath.dataForWrite());
		if (!isString)
		{
			SplitFilename(Name, &FXPath, nullptr, nullptr);
		}
		MaxPath += L"\\HardwareShaders\\";
		MaxDXUtilites::D3DShaderIncludeImp include(MaxPath, FXPath, 1);
		IDirect3D9* d3d = nullptr;
		int supportNVDST = 0;
		int supportFETCH4 = 0;
		device->GetDirect3D(&d3d);
		MaxDXUtilites::CheckHardwareCaps(d3d, supportNVDST, supportFETCH4);
		if (d3d)
		{
			d3d->Release();
			d3d = nullptr;
		}
		pDefines[4].Name = "NVDST_SUPPORTED";
		const char* c;
		if (!supportNVDST || (c = "1\\\n", supportFETCH4))
		{
			c = "0\\\n";
		}
		pDefines[4].Definition = c;
		pDefines[5].Name = "FETCH4_SUPPORTED";
		pDefines[5].Definition = "0\\\n";
		pDefines[6].Name = nullptr;
		pDefines[6].Definition = nullptr;
		HRESULT res;
		const char* s = Data;
		if (s[0] == 1 && s[1] == 9 && s[2] == -1 && s[3] == -2)
		{
			pDefines[0].Name = "_3DSMAX_";
			pDefines[0].Definition = nullptr;
			pDefines[1].Name = "_MAX_";
			pDefines[1].Definition = nullptr;
			pDefines[2].Name = "MAX";
			pDefines[2].Definition = nullptr;
			pDefines[3].Name = "3DSMAX";
			pDefines[3].Definition = nullptr;
			pDefines[4].Name = nullptr;
			pDefines[4].Definition = nullptr;
			res = D3DXCreateEffectFromFile(device, Name, pDefines, nullptr, 0, nullptr, &ppEffect, &ppCompilationErrors);
		}
		else
		{
			res = D3DXCreateEffect(device, Data, (UINT)strlen(Data), pDefines, (LPD3DXINCLUDE)&include, D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, nullptr, &ppEffect, &ppCompilationErrors);
		}
		WStr buffer(L"Buffer");
		if (isString)
		{
			name = buffer;
		}
		*effect = new CD3DEffect9(name, ppEffect, ppCompilationErrors);
		if (res < 0)
		{
			return res;
		}
		if (*effect)
		{
			return S_OK;
		}
		return E_OUTOFMEMORY;
	}
	virtual int GetNumberOfGlobalVariables() override
	{
		D3DXEFFECT_DESC desc;
		resource->GetDesc(&desc);
		return desc.Parameters;
	}
	virtual char const* GetGlobalVariableName(int parameter) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetParameter(nullptr, parameter), &desc);
		return desc.Name;
	}
	virtual bool IsGlobalVariableValid(char const* name) override
	{
		return resource->GetParameterByName(nullptr, name) != 0;
	}
	virtual D3D_EFFECT_VARIABLE_TYPE GetGlobalVariableType(int parameter) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetParameter(nullptr, parameter), &desc);
		if (desc.Type == D3DXPT_SAMPLER2D)
		{
			return D3DXPT_SAMPLER;
		}
		return desc.Type;
	}
	virtual D3D_EFFECT_VARIABLE_CLASS GetGlobalVariableClass(int parameter) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetParameter(nullptr, parameter), &desc);
		return desc.Class;
	}
	virtual int GetNumberOfTechniques() override
	{
		D3DXEFFECT_DESC desc;
		resource->GetDesc(&desc);
		return desc.Techniques;
	}
	virtual bool IsTechniqueValid(int technique) override
	{
		return resource->ValidateTechnique(resource->GetTechnique(technique)) == 0;
	}
	virtual char const* GetTechniqueName(int technique) override
	{
		D3DXTECHNIQUE_DESC desc;
		resource->GetTechniqueDesc(resource->GetTechnique(technique), &desc);
		return desc.Name;
	}
	virtual int GetNumberOfAnnotations(char const* name) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetParameterByName(nullptr, name), &desc);
		return desc.Annotations;
	}
	virtual char const* GetAnnotationName(char const* name, int i) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetAnnotation(name, i), &desc);
		return desc.Name;
	}
	virtual bool IsAnnotationValid(char const* name, char const* annotation) override
	{
		return resource->GetAnnotationByName(name, annotation) != 0;
	}
	virtual char const* GetVariableSemantic(char const* name) override
	{
		D3DXPARAMETER_DESC desc;
		resource->GetParameterDesc(resource->GetParameterByName(nullptr, name), &desc);
		return desc.Semantic;
	}
	virtual bool GetString(char const* name, char const* annotation, char** value) override
	{
		resource->GetString(resource->GetAnnotationByName(name, annotation), (LPCSTR*)value);
		return true;
	}
	virtual bool GetString(char const* name, int index, char** value) override
	{
		resource->GetString(resource->GetAnnotation(name, index), (LPCSTR*)value);
		return true;
	}
	virtual bool GetString(char const* name, char** value) override
	{
		resource->GetString(resource->GetParameterByName(0, name), (LPCSTR*)value);
		return true;
	}
	virtual bool GetInt(char const* name, char const* annotation, int* value) override
	{
		resource->GetInt(resource->GetAnnotationByName(name, annotation), value);
		return true;
	}
	virtual bool GetInt(char const* name, int* value) override
	{
		resource->GetInt(resource->GetParameterByName(0, name), value);
		return true;
	}
	virtual bool GetInt(char const* name, int index, int* value) override
	{
		resource->GetInt(resource->GetAnnotation(name, index), value);
		return true;
	}
	virtual bool GetFloat(char const* name, float* value) override
	{
		resource->GetFloat(resource->GetParameterByName(0, name), value);
		return true;
	}
	virtual bool GetFloat(char const* name, char const* annotation, float* value) override
	{
		resource->GetFloat(resource->GetAnnotationByName(name, annotation), value);
		return true;
	}
	virtual bool GetBool(char const* name, char const* annotation, int* value) override
	{
		resource->GetBool(resource->GetAnnotationByName(name, annotation), value);
		return true;
	}
	virtual bool GetBool(char const* name, int* value) override
	{
		resource->GetBool(resource->GetParameterByName(0, name), value);
		return true;
	}
	virtual bool GetVector(char const* name, D3DXVECTOR4* value) override
	{
		resource->GetVector(resource->GetParameterByName(0, name), value);
		return true;
	}
	virtual long OnLostDevice() override
	{
		if (resource)
		{
			return resource->OnLostDevice();
		}
		return D3DERR_INVALIDCALL;
	}
	virtual long OnResetDevice() override
	{
		if (resource)
		{
			return resource->OnResetDevice();
		}
		return D3DERR_INVALIDCALL;
	}
	LPD3DXBUFFER CompileErrors;
};

class MaxTextureData;
class LightData;
class TexcoordData;
class CD3DEffect10;
class ID3D10InputLayout;
class MaxMappingData
{
public:
	Tab<TexcoordData*> TexcoordData;
};
class MaxEffectParser : public IEffectParser
{
public:
	Tab<MaxTextureData*> TextureData;
	Tab<LightData*> LightData;
	MaxMappingData MappingData;
	Tab<int> MappingValues;
	bool UseLPRT;
	bool IsD3D9;
	bool NeedsRendering;
	bool NoPixelShader;
	CD3DEffect9* Effect9;
	CD3DEffect10* Effect10;
	ID3D10InputLayout* InputLayout;
	WStr DefaultErrors;
	WStr Errors;
};

typedef bool (*MaxLoadEffectPtr)(MaxEffectParser*, void*, IEffectManager*, char const*, bool, bool);
MaxLoadEffectPtr MaxLoadEffectAddr = nullptr;
bool MaxLoadEffect(MaxEffectParser* p, void* device, IEffectManager* em, char const* effect, bool fileType, bool forceReload)
{
	p->IsD3D9 = em->GetDirectXVersion() == IEffectManager::kDirectX9;
	if (p->IsD3D9)
	{
		WStr str = WStr::FromUTF8(effect);
		bool isString = fileType == false;
		return CD3DEffect9::Create((IDirect3DDevice9*)device, str, isString, &p->Effect9) >= 0;
	}
	else
	{
		return MaxLoadEffectAddr(p, device, em, effect, fileType, forceReload);
	}
}

typedef void (*MaxDevicePtr)(MaxEffectParser*);
MaxDevicePtr MaxLostDeviceAddr = nullptr;
void MaxLostDevice(MaxEffectParser* p)
{
	ResourceCache<ID3DXEffect*>::LostDevice();
	MaxLostDeviceAddr(p);
}

MaxDevicePtr MaxResetDeviceAddr = nullptr;
void MaxResetDevice(MaxEffectParser* p)
{
	ResourceCache<ID3DXEffect*>::ResetDevice();
	MaxResetDeviceAddr(p);
}

bool VersionMatch(wchar_t* szVersionFile)
{
	DWORD  verHandle = 0;
	UINT   size = 0;
	LPBYTE lpBuffer = NULL;
	DWORD  verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);

	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];

		if (GetFileVersionInfo(szVersionFile, verHandle, verSize, verData))
		{
			if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
						if (verInfo->dwFileVersionMS == 0x00190003 && verInfo->dwFileVersionLS == 0x00000e38)
						{
							return true;
						}
					}
				}
			}
		}
		delete[] verData;
	}
	return false;
}

__declspec(dllexport) int LibInitialize()
{
	wchar_t path[MAX_PATH];
	wcscpy_s(path, GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR));
	wcscat_s(path, L"\\stdplugs\\DxPlugins\\FXParsers\\MaxEffectParser.dll");
	if (VersionMatch(path))
	{
		DWORD old;
		DWORD old2;
		HMODULE h = LoadLibrary(path);
		typedef EffectDescriptor* (*GetEffectDescriptor)(int);
		GetEffectDescriptor ged = (GetEffectDescriptor)GetProcAddress(h, "GetEffectDescriptor");

		EffectDescriptor* e = ged(0);
		IEffectParser* maxparser = e->CreateParser();
		char* c = *(char**)maxparser;

		c += 0x40;
		MaxLoadEffectPtr* p = (MaxLoadEffectPtr*)c;
		MaxLoadEffectAddr = *p;
		VirtualProtectEx(GetCurrentProcess(), p, sizeof(*p), PAGE_EXECUTE_READWRITE, &old);
		*p = MaxLoadEffect;
		VirtualProtectEx(GetCurrentProcess(), p, sizeof(*p), old, &old2);

		c += 0x30;
		MaxDevicePtr* px = (MaxDevicePtr*)c;
		MaxLostDeviceAddr = *px;
		VirtualProtectEx(GetCurrentProcess(), px, sizeof(*px), PAGE_EXECUTE_READWRITE, &old);
		*px = MaxLostDevice;
		VirtualProtectEx(GetCurrentProcess(), px, sizeof(*px), old, &old2);

		c += 0x8;
		MaxDevicePtr* py = (MaxDevicePtr*)c;
		MaxResetDeviceAddr = *py;
		VirtualProtectEx(GetCurrentProcess(), py, sizeof(*py), PAGE_EXECUTE_READWRITE, &old);
		*py = MaxResetDevice;
		VirtualProtectEx(GetCurrentProcess(), py, sizeof(*py), old, &old2);
		maxparser->DestroyParser();
	}
	return TRUE;
}
