#pragma once
#ifndef W3D_MAX_TOOLS_INCLUDE_W3D_MAX_COMMON_HELPERS_H
#define W3D_MAX_TOOLS_INCLUDE_W3D_MAX_COMMON_HELPERS_H

#include <max.h>

namespace W3D::MaxTools
{
	template <typename FUNC>
	void VisitSceneNodes(FUNC&& f, INode& current = *GetCOREInterface()->GetRootNode());

	void SetSelectedSceneNodes(INodeTab& selection);
	void GetCurrentFilename(TSTR& out);
}

#include "w3dmaxcommonhelpers.inl"
#endif //W3D_MAX_TOOLS_INCLUDE_W3D_MAX_COMMON_HELPERS_H
