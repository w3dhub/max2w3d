#include "general.h"
#include "w3dappdatachunk.h"

namespace W3D::MaxTools
{
	W3DAppDataChunk::W3DAppDataChunk()
		: ExportFlags(W3DExportFlags::ExportGeometry | W3DExportFlags::ExportTransform)
		  , GeometryType(W3DGeometryType::Normal)
		  , GeometryFlags(W3DGeometryFlags::None)
		  , CollisionFlags(W3DCollisionFlags::None)
		  , StaticSortLevel(0), unk14(0), unk18(0), unk1C(0)
	{
	}
}
