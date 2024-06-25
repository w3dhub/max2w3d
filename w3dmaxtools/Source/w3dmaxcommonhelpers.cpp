#include "general.h"
#include "w3dmaxcommonhelpers.h"

namespace W3D::MaxTools
{
	void SetSelectedSceneNodes(INodeTab & selection)
	{
		Interface* ci = GetCOREInterface();
		theHold.Begin();
		ci->ClearNodeSelection(FALSE);
		ci->SelectNodeTab(selection, TRUE);
		theHold.End();
	}

	void GetCurrentFilename(TSTR& out)
	{		
		SplitFilename(GetCOREInterface()->GetCurFileName(), nullptr, &out, nullptr);
	}
}