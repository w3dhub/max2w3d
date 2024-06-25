#include "general.h"

#include "SimpleFileFactoryClass.h"
#include "w3dexport.h"
#include "w3dmaterial.h"
#ifndef W3X
#include "w3dutilities.h"
#else
#include "w3xutilities.h"
#endif

#include "w3dskin.h"

class FileFactoryClass;

HINSTANCE hInstance;


FileFactoryClass* _TheWritingFileFactory;
FileFactoryClass* _TheFileFactory;
SimpleFileFactoryClass _DefaultFileFactory;
bool isClient = true;

const std::array<ClassDesc*, 5>& ClassDescriptions()
{
	static const std::array<ClassDesc*, 5> classes
	{
		W3D::MaxTools::W3DExportClassDesc::Instance(),
		W3D::MaxTools::W3DMaterialClassDesc::Instance(),
		W3D::MaxTools::W3DUtilitiesClassDesc::Instance(),
		W3D::MaxTools::SkinWSMObjectClassDesc::Instance(),
		W3D::MaxTools::SkinModifierClassDesc::Instance()
	};

	return classes;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fwdReason, LPVOID lpvReserved)
{
	if (fwdReason == DLL_PROCESS_ATTACH)
	{
		MaxSDK::Util::UseLanguagePackLocale();
		hInstance = hinstDLL;
		DisableThreadLibraryCalls(hInstance);

		char modulePath[MAX_PATH];
		char moduleDirectory[MAX_PATH];
		GetModuleFileNameA(hInstance, modulePath, MAX_PATH);
		SplitFilename(modulePath, modulePath, MAX_PATH, NULL, NULL, NULL, NULL);
		snprintf(moduleDirectory, MAX_PATH, "%s\\", modulePath);

		_TheFileFactory = _TheWritingFileFactory = &_DefaultFileFactory;
		_DefaultFileFactory.Set_Sub_Directory(moduleDirectory);
	}

	return TRUE;
}

__declspec(dllexport) const TCHAR* LibDescription()
{
	return _T("W3D Tools for 3DS Max 2018. (c) Tiberian Technologies");
}

__declspec(dllexport) int LibNumberClasses()
{
	return static_cast<int>(ClassDescriptions().size());
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	return ClassDescriptions()[i];
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

void SetFileFactories()
{
}
