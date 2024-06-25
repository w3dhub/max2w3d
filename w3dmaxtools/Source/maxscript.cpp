#include "general.h"
#include <stdmat.h>
#include <iInstanceMgr.h>
#include <maxscript\maxscript.h>
#include <maxscript\maxwrapper\mxsobjects.h>
#include <maxscript\maxwrapper\mxsmaterial.h>
#include <maxscript\foundation\numbers.h>
#include <maxscript\foundation\colors.h>
#include <maxscript\macros\define_instantiation_functions.h>
#include "w3d.h"
#include "w3dmaterial.h"
#ifndef W3X
#include "w3dutilities.h"
#else
#include "w3xutilities.h"
#endif
using namespace W3D::MaxTools;
def_visible_primitive(wwSetType, "wwSetType");
Value *wwSetType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetType, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Type", arg_list[1], class_tag(Integer));
	}
	INode *node = arg_list[0]->to_node();
	int type = arg_list[1]->to_int();
	INodeTab nodes;
	IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
	for (int i = 0; i < nodes.Count(); ++i)
	{
		W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryType = (W3DGeometryType)type;
	}
	return &ok;
}

#ifndef W3X
def_visible_primitive(wwSetDazzle, "wwSetDazzle");
Value *wwSetDazzle_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDazzle, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_string(arg_list[1]))
	{
		throw TypeError(L"Type", arg_list[1], class_tag(String));
	}
	INode *node = arg_list[0]->to_node();
	CStr str = CStr::FromMSTR(arg_list[1]->to_string());
	W3DUtilities::SetDazzleTypeInAppData(node, str);
	return &ok;
}

def_visible_primitive(wwSetFlags, "wwSetFlags");
Value *wwSetFlags_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetFlags, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Flags", arg_list[1], class_tag(Integer));
	}
	INode *node = arg_list[0]->to_node();
	int flags = arg_list[1]->to_int();
	W3DAppDataChunk &str = W3DUtilities::GetOrCreateW3DAppDataChunk(*node);
	if (flags & W3D_MESH_FLAG_HIDDEN)
	{
		str.GeometryFlags |= W3DGeometryFlags::Hide;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::Hide;
	}
	if (flags & W3D_MESH_FLAG_TWO_SIDED)
	{
		str.GeometryFlags |= W3DGeometryFlags::TwoSided;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::TwoSided;
	}

	if (flags & W3D_MESH_FLAG_CAST_SHADOW)
	{
		str.GeometryFlags |= W3DGeometryFlags::Shadow;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::Shadow;
	}

	if (flags & W3D_MESH_FLAG_SHATTERABLE)
	{
		str.GeometryFlags |= W3DGeometryFlags::Shatter;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::Shatter;
	}

	if (flags & W3D_MESH_FLAG_NPATCHABLE)
	{
		str.GeometryFlags |= W3DGeometryFlags::Tangents;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::Tangents;
	}

	if (flags & W3D_MESH_FLAG_PRELIT)
	{
		str.GeometryFlags |= W3DGeometryFlags::Prelit;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::Prelit;
	}

	if (flags & W3D_MESH_FLAG_ALWAYSDYNLIGHT)
	{
		str.GeometryFlags |= W3DGeometryFlags::AlwaysDynLight;
	}
	else
	{
		str.GeometryFlags &= ~W3DGeometryFlags::AlwaysDynLight;
	}
	
	int gt = flags & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
	switch (gt)
	{
	case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
		str.GeometryType = W3DGeometryType::Normal;
		break;
	case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
		str.GeometryType = W3DGeometryType::CamParal;
		break;
	case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
		str.GeometryType = W3DGeometryType::CamOrient;
		break;
	case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED:
		str.GeometryType = W3DGeometryType::CamZOrient;
		break;
	}

	if (flags & W3D_BOX_ATTRIBUTE_ORIENTED)
	{
		str.GeometryType = W3DGeometryType::OBBox;
		str.GeometryFlags |= W3DGeometryFlags::Hide;
	}
	if (flags & W3D_BOX_ATTRIBUTE_ALIGNED)
	{
		str.GeometryType = W3DGeometryType::AABox;
	}
	int ct = flags & W3D_MESH_FLAG_COLLISION_TYPE_MASK;
	if (ct & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL)
	{
		str.CollisionFlags |= W3DCollisionFlags::Physical;
	}
	else
	{
		str.CollisionFlags &= ~W3DCollisionFlags::Physical;
	}

	if (ct & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE)
	{
		str.CollisionFlags |= W3DCollisionFlags::Projectile;
	}
	else
	{
		str.CollisionFlags &= ~W3DCollisionFlags::Projectile;
	}

	if (ct & W3D_MESH_FLAG_COLLISION_TYPE_VIS)
	{
		str.CollisionFlags |= W3DCollisionFlags::Vis;
	}
	else
	{
		str.CollisionFlags &= ~W3DCollisionFlags::Vis;
	}

	if (ct & W3D_MESH_FLAG_COLLISION_TYPE_CAMERA)
	{
		str.CollisionFlags |= W3DCollisionFlags::Camera;
	}
	else
	{
		str.CollisionFlags &= ~W3DCollisionFlags::Camera;
	}

	if (ct & W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE)
	{
		str.CollisionFlags |= W3DCollisionFlags::Vehicle;
	}
	else
	{
		str.CollisionFlags &= ~W3DCollisionFlags::Vehicle;
	}
	return &ok;
}
#endif

def_visible_primitive(wwSetSorting, "wwSetSorting");
Value *wwSetSorting_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSorting, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Sorting", arg_list[1], class_tag(Integer));
	}
	INode *node = arg_list[0]->to_node();
	int type = arg_list[1]->to_int();
	INodeTab nodes;
	IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
	for (int i = 0; i < nodes.Count(); ++i)
	{
		W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).StaticSortLevel = type;
	}
	return &ok;
}

def_visible_primitive(wwSetExportGeoFlag, "wwSetExportGeoFlag");
Value *wwSetExportGeoFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetExportGeoFlag, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"export bone value", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).ExportFlags |= W3DExportFlags::ExportGeometry;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).ExportFlags &= ~W3DExportFlags::ExportGeometry;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetExportBoneFlag, "wwSetExportBoneFlag");
Value *wwSetExportBoneFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetExportBoneFlag, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"export bone value", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).ExportFlags |= W3DExportFlags::ExportTransform;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).ExportFlags &= ~W3DExportFlags::ExportTransform;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetHide, "wwSetHide");
Value *wwSetHide_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetHide, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::Hide;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::Hide;
		}
	}
	return &ok;
}

def_visible_primitive(wwSet2Side, "wwSet2Side");
Value *wwSet2Side_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSet2Side, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::TwoSided;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::TwoSided;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetShadow, "wwSetShadow");
Value *wwSetShadow_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetShadow, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::Shadow;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::Shadow;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetVAlpha, "wwSetVAlpha");
Value *wwSetVAlpha_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetVAlpha, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::VAlpha;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::VAlpha;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetZNormal, "wwSetZNormal");
Value *wwSetZNormal_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetZNormal, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::ZNormal;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::ZNormal;
		}
	}
	return &ok;
}

#ifndef W3X
def_visible_primitive(wwSetShatter, "wwSetShatter");
Value *wwSetShatter_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetShatter, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::Shatter;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::Shatter;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetNPatch, "wwSetNPatch");
Value *wwSetNPatch_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetNPatch, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::Tangents;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::Tangents;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetPrelit, "wwSetPrelit");
Value *wwSetPrelit_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetPrelit, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::Prelit;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::Prelit;
		}
	}
	return &ok;
}
#else
def_visible_primitive(wwSetJoypadPick, "wwSetJoypadPick");
Value* wwSetJoypadPick_cf(Value** arg_list, int count)
{
	check_arg_count(wwSetJoypadPick, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode* node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::JoypadPick;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::JoypadPick;
		}
	}
	return &ok;
}
#endif

def_visible_primitive(wwSetKeepNormal, "wwSetKeepNormal");
Value *wwSetKeepNormal_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetKeepNormal, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags |= W3DGeometryFlags::KeepNml;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).GeometryFlags &= ~W3DGeometryFlags::KeepNml;
		}
	}
	return &ok;
}

#ifndef W3X
def_visible_primitive(wwSetCollidePhysical, "wwSetCollidePhysical");
Value *wwSetCollidePhysical_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetCollidePhysical, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags |= W3DCollisionFlags::Physical;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags &= ~W3DCollisionFlags::Physical;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetCollideProjectile, "wwSetCollideProjectile");
Value *wwSetCollideProjectile_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetCollideProjectile, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags |= W3DCollisionFlags::Projectile;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags &= ~W3DCollisionFlags::Projectile;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetCollideVis, "wwSetCollideVis");
Value *wwSetCollideVis_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetCollideVis, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags |= W3DCollisionFlags::Vis;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags &= ~W3DCollisionFlags::Vis;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetCollideCamera, "wwSetCollideCamera");
Value *wwSetCollideCamera_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetCollideCamera, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags |= W3DCollisionFlags::Camera;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags &= ~W3DCollisionFlags::Camera;
		}
	}
	return &ok;
}

def_visible_primitive(wwSetCollideVehicle, "wwSetCollideVehicle");
Value *wwSetCollideVehicle_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetCollideVehicle, 2, count);
	if (!is_node(arg_list[0]))
	{
		throw TypeError(L"Max INode", arg_list[0], class_tag(MAXNode));
	}
	if (!is_bool(arg_list[1]))
	{
		throw TypeError(L"Boolean", arg_list[1], class_tag(Boolean));
	}
	INode *node = arg_list[0]->to_node();
	BOOL hide = arg_list[1]->to_bool();
	if (hide)
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags |= W3DCollisionFlags::Vehicle;
		}
	}
	else
	{
		INodeTab nodes;
		IInstanceMgr::GetInstanceMgr()->GetInstances(*node, nodes);
		for (int i = 0; i < nodes.Count(); ++i)
		{
			W3DUtilities::GetOrCreateW3DAppDataChunk(*nodes[i]).CollisionFlags &= ~W3DCollisionFlags::Vehicle;
		}
	}
	return &ok;
}
#endif

#define is_material(mat) mat->is_kind_of(class_tag(MAXMaterial))
def_visible_primitive(wwSetSortLevel, "wwSetSortLevel");
Value *wwSetSortLevel_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSortLevel, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Value", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int value = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::StaticSortLevel), 0, value);
	}
	return &ok;
}

def_visible_primitive(wwGetSortLevel, "wwGetSortLevel");
Value *wwGetSortLevel_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetSortLevel, 1, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		return Integer::intern(((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->GetInt(enum_to_value(W3DMaterialParamID::StaticSortLevel), 0));
	}
	return &undefined;
}

def_visible_primitive(wwSetSurfaceType, "wwSetSurfaceType");
Value *wwSetSurfaceType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSurfaceType, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Value", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int value = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->SetValue(enum_to_value(W3DMaterialParamID::SurfaceType), 0, value);

	}
	return &ok;
}

def_visible_primitive(wwGetSurfaceType, "wwGetSurfaceType");
Value *wwGetSurfaceType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetSurfaceType, 1, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		return Integer::intern(((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::SurfaceTypeBlock)))->GetInt(enum_to_value(W3DMaterialParamID::SurfaceType), 0));
	}
	return &undefined;
}

def_visible_primitive(wwSetPassCount, "wwSetPassCount");
Value *wwSetPassCount_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetPassCount, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Value", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int value = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::PassCountBlock)))->SetValue(enum_to_value(W3DMaterialParamID::PassCount), 0, value);
		gm->RefreshPasses();
	}
	return &ok;
}

def_visible_primitive(wwGetPassCount, "wwGetPassCount");
Value *wwGetPassCount_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetPassCount, 1, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		return Integer::intern(((IParamBlock2 *)gm->GetReference(enum_to_value(W3DMaterialRefID::PassCountBlock)))->GetInt(enum_to_value(W3DMaterialParamID::PassCount), 0));
	}
	return &undefined;
}

def_visible_primitive(wwSetWriteZBuffer, "wwSetWriteZBuffer");
Value *wwSetWriteZBuffer_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetWriteZBuffer, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_bool(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	BOOL value = arg_list[2]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::BlendWriteZBuffer), 0, value);
	}
	return &ok;
}

def_visible_primitive(wwGetWriteZBuffer, "wwGetWriteZBuffer");
Value *wwGetWriteZBuffer_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetWriteZBuffer, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		bool b = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::BlendWriteZBuffer), 0);
		if (b)
		{
			return &true_value;
		}
		else
		{
			return &false_value;
		}
	}
	return &undefined;
}

def_visible_primitive(wwSetDestBlend, "wwSetDestBlend");
Value *wwSetDestBlend_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDestBlend, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::CustomDestMode), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetSrcBlend, "wwSetSrcBlend");
Value *wwSetSrcBlend_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSrcBlend, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::CustomSrcMode), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetAlphaTestFlag, "wwSetAlphaTestFlag");
Value *wwSetAlphaTestFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetAlphaTestFlag, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"pass number", arg_list[1], class_tag(Integer));
	}
	if (!is_bool(arg_list[2]))
	{
		throw TypeError(L"display flag value", arg_list[2], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	BOOL value = arg_list[2]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::AlphaTest), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetPriGradient, "wwSetPriGradient");
Value *wwSetPriGradient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetPriGradient, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::PriGradient), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetDepthCompare, "wwSetDepthCompare");
Value *wwSetDepthCompare_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDepthCompare, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DepthCmp), 0, value);
	}
	return &ok;
}

def_visible_primitive(wwSetDetailColor, "wwSetDetailColor");
Value *wwSetDetailColor_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDetailColor, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DetailColour), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetDetailAlpha, "wwSetDetailAlpha");
Value *wwSetDetailAlpha_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDetailAlpha, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DetailAlpha), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetSecGradient, "wwSetSecGradient");
Value *wwSetSecGradient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSecGradient, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SecGradient), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetOpacity, "wwSetOpacity");
Value *wwSetOpacity_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetOpacity, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Float));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	float value = arg_list[2]->to_float();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Opacity), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetShininess, "wwSetShininess");
Value *wwSetShininess_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetShininess, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Float));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	float value = arg_list[2]->to_float();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Shininess), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetTranslucency, "wwSetTranslucency");
Value *wwSetTranslucency_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetTranslucency, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Float));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	float value = arg_list[2]->to_float();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Translucency), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetUV, "wwSetUV");
Value *wwSetUV_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetUV, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1MappingUVChannel : W3DMaterialParamID::Stage0MappingUVChannel), 0, value);
		auto texmap = gm->GetMaterialPass(pass).ParamBlock->GetTexmap(enum_to_value(stage ? W3DMaterialParamID::Stage1TextureMap : W3DMaterialParamID::Stage0TextureMap));
		if (texmap)
		{
			auto uvgen = texmap->GetTheUVGen();
			if (uvgen)
			{
				uvgen->SetMapChannel(value);
			}
		}
		gm->InvalidateDisplayTexture();
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetUV, "wwGetUV");
Value *wwGetUV_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetUV, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int uv = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1MappingUVChannel : W3DMaterialParamID::Stage0MappingUVChannel), 0);
		return Integer::intern(uv);
	}
	return &undefined;
}

def_visible_primitive(wwSetAmbient, "wwSetAmbient");
Value *wwSetAmbient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetAmbient, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_color(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	COLORREF value = arg_list[2]->to_colorref();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::AmbientColour), 0, (Color)value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetDiffuse, "wwSetDiffuse");
Value *wwSetDiffuse_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDiffuse, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_color(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	COLORREF value = arg_list[2]->to_colorref();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::DiffuseColour), 0, (Color)value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetSpecular, "wwSetSpecular");
Value *wwSetSpecular_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSpecular, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_color(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	COLORREF value = arg_list[2]->to_colorref();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SpecularColour), 0, (Color)value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetEmissive, "wwSetEmissive");
Value *wwSetEmissive_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetEmissive, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_color(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	COLORREF value = arg_list[2]->to_colorref();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::EmissiveColour), 0, (Color)value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetSpecularToDiffuse, "wwSetSpecularToDiffuse");
Value *wwSetSpecularToDiffuse_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetSpecularToDiffuse, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_bool(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	BOOL value = arg_list[2]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SpecularToDiffuse), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetMapType, "wwSetMapType");
Value *wwSetMapType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetMapType, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Mapping : W3DMaterialParamID::Stage0Mapping), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetFrames, "wwSetFrames");
Value *wwSetFrames_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetFrames, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Frames : W3DMaterialParamID::Stage0Frames), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetFrameRate, "wwSetFrameRate");
Value *wwSetFrameRate_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetFrameRate, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Float));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	float value = arg_list[3]->to_float();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1FPS : W3DMaterialParamID::Stage0FPS), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetPublish, "wwSetPublish");
Value *wwSetPublish_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetPublish, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Publish : W3DMaterialParamID::Stage0Publish), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetClampU, "wwSetClampU");
Value *wwSetClampU_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetClampU, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampU : W3DMaterialParamID::Stage0ClampU), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetClampV, "wwSetClampV");
Value *wwSetClampV_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetClampV, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampV : W3DMaterialParamID::Stage0ClampV), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetNoLod, "wwSetNoLod");
Value *wwSetNoLod_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetNoLod, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1NoLOD : W3DMaterialParamID::Stage0NoLOD), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetAlphaBitmap, "wwSetAlphaBitmap");
Value *wwSetAlphaBitmap_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetAlphaBitmap, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1AlphaBitmap : W3DMaterialParamID::Stage0AlphaBitmap), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetAnimType, "wwSetAnimType");
Value *wwSetAnimType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetAnimType, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1AnimMode : W3DMaterialParamID::Stage0AnimMode), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetPassHint, "wwSetPassHint");
Value *wwSetPassHint_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetPassHint, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1PassHint : W3DMaterialParamID::Stage0PassHint), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwSetStageFlag, "wwSetStageFlag");
Value *wwSetStageFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetStageFlag, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"pass number", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"stage number", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"stage flag value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1TextureEnabled : W3DMaterialParamID::Stage0TextureEnabled), 0, value);
		if (!value)
		{
			gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Display : W3DMaterialParamID::Stage0Display), 0, false);
			gm->InvalidateDisplayTexture();
			gm->MaterialDirty();
		}
	}
	return &ok;
}

def_visible_primitive(wwSetDisplayFlag, "wwSetDisplayFlag");
Value *wwSetDisplayFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetDisplayFlag, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"pass number", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"stage number", arg_list[2], class_tag(Integer));
	}
	if (!is_bool(arg_list[3]))
	{
		throw TypeError(L"display flag value", arg_list[3], class_tag(Boolean));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	BOOL value = arg_list[3]->to_bool();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		if (value)
		{
			gm->ClearDisplayFlags();
		}
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Display : W3DMaterialParamID::Stage0Display), 0, value);
		gm->InvalidateDisplayTexture();
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetStageFlag, "wwGetStageFlag");
Value *wwGetStageFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetStageFlag, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		bool b = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1TextureEnabled : W3DMaterialParamID::Stage0TextureEnabled), 0);
		if (b)
		{
			return &true_value;
		}
		else
		{
			return &false_value;
		}
	}
	return &undefined;
}

def_visible_primitive(wwSetMapperArgs, "wwSetMapperArgs");
Value *wwSetMapperArgs_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetMapperArgs, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_string(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(String));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	const wchar_t *value = arg_list[3]->to_string();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1MappingArgs : W3DMaterialParamID::Stage0MappingArgs), 0, value);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetMapperArgs, "wwGetMapperArgs");
Value *wwGetMapperArgs_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetMapperArgs, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		const wchar_t *str = gm->GetMaterialPass(pass).ParamBlock->GetStr(enum_to_value(stage ? W3DMaterialParamID::Stage1MappingArgs : W3DMaterialParamID::Stage0MappingArgs), 0);
		if (str && str[0])
		{
			return new String(str);
		}
	}
	return &undefined;
}

def_visible_primitive(wwSetTexture, "wwSetTexture");
Value *wwSetTexture_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetTexture, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_string(arg_list[1]))
	{
		throw TypeError(L"texture path and texture name", arg_list[1], class_tag(String));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"index number", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	const wchar_t *value = arg_list[1]->to_string();
	int index = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		BitmapTex* tm = static_cast<BitmapTex*>(gm->GetSubTexmap(index));
		if (!tm)
		{
			tm = NewDefaultBitmapTex();
		}
		tm->ActivateTexDisplay(TRUE);
		tm->SetMapName(value);
		gm->SetSubTexmap(index, tm);
		gm->InvalidateDisplayTexture();
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetTexture, "wwGetTexture");
Value *wwGetTexture_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetTexture, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"index number", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int index = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		BitmapTex* tm = static_cast<BitmapTex*>(gm->GetSubTexmap(index));
		if (tm)
		{
			return Name::intern(tm->GetMapName());
		}
	}
	return &undefined;
}

def_visible_primitive(wwGetAmbient, "wwGetAmbient");
Value *wwGetAmbient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetAmbient, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		Color c = gm->GetMaterialPass(pass).ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::AmbientColour), 0);
		return new ColorValue(c);
	}
	return &undefined;
}

def_visible_primitive(wwGetDiffuse, "wwGetDiffuse");
Value *wwGetDiffuse_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetDiffuse, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		Color c = gm->GetMaterialPass(pass).ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::DiffuseColour), 0);
		return new ColorValue(c);
	}
	return &undefined;
}

def_visible_primitive(wwGetSpecular, "wwGetSpecular");
Value *wwGetSpecular_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetSpecular, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		Color c = gm->GetMaterialPass(pass).ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::SpecularColour), 0);
		return new ColorValue(c);
	}
	return &undefined;
}

def_visible_primitive(wwGetEmissive, "wwGetEmissive");
Value *wwGetEmissive_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetEmissive, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		Color c = gm->GetMaterialPass(pass).ParamBlock->GetColor(enum_to_value(W3DMaterialParamID::EmissiveColour), 0);
		return new ColorValue(c);
	}
	return &undefined;
}

def_visible_primitive(wwGetShininess, "wwGetShininess");
Value *wwGetShininess_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetShininess, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		float f = gm->GetMaterialPass(pass).ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Shininess), 0);
		return Float::intern(f);
	}
	return &undefined;
}

def_visible_primitive(wwGetOpacity, "wwGetOpacity");
Value *wwGetOpacity_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetOpacity, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		float f = gm->GetMaterialPass(pass).ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Opacity), 0);
		return Float::intern(f);
	}
	return &undefined;
}

def_visible_primitive(wwGetTranslucency, "wwGetTranslucency");
Value *wwGetTranslucency_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetTranslucency, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		float f = gm->GetMaterialPass(pass).ParamBlock->GetFloat(enum_to_value(W3DMaterialParamID::Translucency), 0);
		return Float::intern(f);
	}
	return &undefined;
}

def_visible_primitive(wwGetDepthCompare, "wwGetDepthCompare");
Value *wwGetDepthCompare_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetDepthCompare, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DepthCmp), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetDestBlend, "wwGetDestBlend");
Value *wwGetDestBlend_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetDestBlend, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomDestMode), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetPriGradient, "wwGetPriGradient");
Value *wwGetPriGradient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetPriGradient, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::PriGradient), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetSecGradient, "wwGetSecGradient");
Value *wwGetSecGradient_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetSecGradient, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SecGradient), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetSrcBlend, "wwGetSrcBlend");
Value *wwGetSrcBlend_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetSrcBlend, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::CustomSrcMode), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetAlphaTestFlag, "wwGetAlphaTestFlag");
Value *wwGetAlphaTestFlag_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetAlphaTestFlag, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"pass number", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::AlphaTest), 0);
		if (i)
		{
			return &true_value;
		}
		return &false_value;
	}
	return &undefined;
}

def_visible_primitive(wwGetDetailColor, "wwGetDetailColor");
Value *wwGetDetailColor_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetDetailColor, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailColour), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetDetailAlpha, "wwGetDetailAlpha");
Value *wwGetDetailAlpha_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetDetailAlpha, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::DetailAlpha), 0);
		return Integer::intern(i);
	}
	return &undefined;
}
def_visible_primitive(wwSetMatFlags, "wwSetMatFlags");
Value *wwSetMatFlags_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetMatFlags, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Value", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int value = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int m1 = (value & W3DVERTMAT_STAGE0_MAPPING_MASK) >> 0x10;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage0Mapping), 0, m1);
		int m2 = (value & W3DVERTMAT_STAGE1_MAPPING_MASK) >> 8;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::Stage1Mapping), 0, m2);
		int f = value & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(W3DMaterialParamID::SpecularToDiffuse), 0, f == 1);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetMatFlags, "wwGetMatFlags");
Value *wwGetMatFlags_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetMatFlags, 2, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int m1 = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage0Mapping), 0);
		int m2 = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::Stage1Mapping), 0);
		int f = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(W3DMaterialParamID::SpecularToDiffuse), 0);
		int flags = m1 | m2 | f;
		return Integer::intern(flags);
	}
	return &undefined;
}
def_visible_primitive(wwSetTexFlags, "wwSetTexFlags");
Value *wwSetTexFlags_cf(Value ** arg_list, int count)
{
	check_arg_count(wwSetTexFlags, 4, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	if (!is_number(arg_list[3]))
	{
		throw TypeError(L"Value", arg_list[3], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	int value = arg_list[3]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int p = (value & W3DTEXTURE_PUBLISH);
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1Publish : W3DMaterialParamID::Stage0Publish), 0, p == 1);
		int n = (value & W3DTEXTURE_NO_LOD);
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1NoLOD : W3DMaterialParamID::Stage0NoLOD), 0, n == 1);
		int u = (value & W3DTEXTURE_CLAMP_U);
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampU : W3DMaterialParamID::Stage0ClampU), 0, u == 1);
		int v = (value & W3DTEXTURE_CLAMP_V);
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampV : W3DMaterialParamID::Stage0ClampV), 0, v == 1);
		int a = (value & W3DTEXTURE_ALPHA_BITMAP);
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1AlphaBitmap : W3DMaterialParamID::Stage0AlphaBitmap), 0, a == 1);
		int h = (value & W3DTEXTURE_HINT_MASK & ~W3DTEXTURE_TYPE_MASK) >> W3DTEXTURE_HINT_SHIFT;
		gm->GetMaterialPass(pass).ParamBlock->SetValue(enum_to_value(stage ? W3DMaterialParamID::Stage1PassHint : W3DMaterialParamID::Stage0PassHint), 0, h);
		gm->MaterialDirty();
	}
	return &ok;
}

def_visible_primitive(wwGetTexFlags, "wwGetTexFlags");
Value *wwGetTexFlags_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetTexFlags, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int p = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1Publish : W3DMaterialParamID::Stage0Publish), 0) == 1 ? W3DTEXTURE_PUBLISH : 0;
		int n = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1NoLOD : W3DMaterialParamID::Stage0NoLOD), 0) == 1 ? W3DTEXTURE_NO_LOD : 0;
		int u = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampU : W3DMaterialParamID::Stage0ClampU), 0) == 1 ? W3DTEXTURE_CLAMP_U : 0;
		int v = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1ClampV : W3DMaterialParamID::Stage0ClampV), 0) == 1 ? W3DTEXTURE_CLAMP_V : 0;
		int a = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1AlphaBitmap : W3DMaterialParamID::Stage0AlphaBitmap), 0) == 1 ? W3DTEXTURE_ALPHA_BITMAP : 0;
		int h = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1PassHint : W3DMaterialParamID::Stage0PassHint), 0) << W3DTEXTURE_HINT_SHIFT;
		int flags = p | n | u | v | a | h;
		return Integer::intern(flags);
	}
	return &undefined;
}

def_visible_primitive(wwGetAnimType, "wwGetAnimType");
Value *wwGetAnimType_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetAnimType, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		int i = gm->GetMaterialPass(pass).ParamBlock->GetInt(enum_to_value(stage ? W3DMaterialParamID::Stage1AnimMode : W3DMaterialParamID::Stage0AnimMode), 0);
		return Integer::intern(i);
	}
	return &undefined;
}

def_visible_primitive(wwGetFrames, "wwGetFrames");
Value *wwGetFrames_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetFrames, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		float f = gm->GetMaterialPass(pass).ParamBlock->GetFloat(enum_to_value(stage ? W3DMaterialParamID::Stage1Frames : W3DMaterialParamID::Stage0Frames), 0);
		return Float::intern(f);
	}
	return &undefined;
}

def_visible_primitive(wwGetFrameRate, "wwGetFrameRate");
Value *wwGetFrameRate_cf(Value ** arg_list, int count)
{
	check_arg_count(wwGetFrameRate, 3, count);
	if (!is_material(arg_list[0]))
	{
		throw TypeError(L"W3D Material", arg_list[0], class_tag(MAXMaterial));
	}
	if (!is_number(arg_list[1]))
	{
		throw TypeError(L"Pass", arg_list[1], class_tag(Integer));
	}
	if (!is_number(arg_list[2]))
	{
		throw TypeError(L"Stage", arg_list[2], class_tag(Integer));
	}
	Mtl *mtl = arg_list[0]->to_mtl();
	int pass = arg_list[1]->to_int();
	int stage = arg_list[2]->to_int();
	if (mtl->ClassID() == W3DMaterialClassDesc::Instance()->ClassID())
	{
		W3DMaterial *gm = (W3DMaterial *)mtl;
		float f = gm->GetMaterialPass(pass).ParamBlock->GetFloat(enum_to_value(stage ? W3DMaterialParamID::Stage1FPS : W3DMaterialParamID::Stage0FPS), 0);
		return Float::intern(f);
	}
	return &undefined;
}
