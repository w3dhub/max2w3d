#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_APPDATA_CHUNK_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_APPDATA_CHUNK_H

// ReSharper disable once CppUnusedIncludeDirective
#include "EnumUtilities.h"


namespace W3D::MaxTools
{
	enum class W3DGeometryType : uint32
	{
		CamParal = 1,
		Normal,
		OBBox,
		AABox,
		CamOrient,
		NullLOD,
		Dazzle,
		Aggregate,
		CamZOrient,

		Num
	};

	enum class W3DGeometryFlags : uint32
	{
		None = 0,
		Hide = 1 << 0,
		TwoSided = 1 << 1,
		Shadow = 1 << 2,
		VAlpha = 1 << 3,
		ZNormal = 1 << 4,
		Shatter = 1 << 5,
		Tangents = 1 << 6,
		KeepNml = 1 << 7,
		Prelit = 1 << 8,
		AlwaysDynLight = 1 << 9
	};

	template<>
	struct EnableEnumClassBitWiseOperators<W3DGeometryFlags>
		: public std::true_type
	{ };

	enum class W3DCollisionFlags : uint32
	{
		None = 0,
		Physical = 1 << 0,
		Projectile = 1 << 1,
		Vis = 1 << 2,
		Camera = 1 << 3,
		Vehicle = 1 << 4
	};

	template<>
	struct EnableEnumClassBitWiseOperators<W3DCollisionFlags>
		: public std::true_type
	{ };

	enum class W3DExportFlags : uint32
	{
		None = 0,
		ExportTransform = 1 << 0,
		ExportGeometry = 1 << 1
	};

	template<>
	struct EnableEnumClassBitWiseOperators<W3DExportFlags>
		: public std::true_type
	{ };

	struct W3DAppDataChunk
	{
		W3DAppDataChunk();

		W3DExportFlags    ExportFlags;
		W3DGeometryType   GeometryType;
		W3DGeometryFlags  GeometryFlags;
		W3DCollisionFlags CollisionFlags;
		int            StaticSortLevel;
		int            unk14;
		int            unk18;
		int            unk1C;
	};

	struct DazzleAppData {
		int f0;
		int f1;
		int f2;
		int f3;
		char name[128];
		DazzleAppData()
		{
			f0 = 0;
			f1 = 0;
			f2 = 0;
			f3 = 0;
			memset(name, 0, sizeof(name));
			strcpy(name, "DEFAULT");
		}
	};
}

#endif //W3D_MAX_TOOLS_INCLUDE_W3D_APPDATA_CHUNK_H