#pragma once
#include "w3dmaxcommonhelpers.h"

namespace W3D::MaxTools
{
	template <typename FUNC>
	void VisitSceneNodes(FUNC&& f, INode& current)
	{
		const int numChildren = current.NumberOfChildren();
		for (int i = 0; i < numChildren; ++i)
		{
			INode* child = current.GetChildNode(i);
			if (child)
			{
				f(*child);
				VisitSceneNodes(f, *child);
			}
		}
	}
}
