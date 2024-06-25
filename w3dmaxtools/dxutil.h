#pragma once
#include <max.h>
#include <d3dx9.h>
namespace MaxDXUtilites {
	class D3DShaderIncludeImp : public ID3DXInclude
	{
	public:
		__declspec(dllimport) D3DShaderIncludeImp(WStr const& hardwareShaderPath, WStr const& shaderPath, int defineShadowFunctors);
		__declspec(dllimport) D3DShaderIncludeImp(int defineShadowFunctors);
		__declspec(dllimport) IFACEMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
		__declspec(dllimport) IFACEMETHOD(Close)(LPCVOID pData);
		__declspec(dllimport) virtual ~D3DShaderIncludeImp();
		WStr hardwareshaderpath;
		WStr shaderpath;
		int defineshadowfunctors;
	};
	__declspec(dllimport) bool LoadFileToString(WStr const& fileName, UTF8Str& string);
	__declspec(dllimport) void CheckHardwareCaps(IDirect3D9* d3d, int& supportNVDST, int& supportFETCH4);
};