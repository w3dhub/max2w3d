#ifndef TT_INCLUDE_AABTREEBUILDER_H
#define TT_INCLUDE_AABTREEBUILDER_H
#include <vector>

#include "AAPlaneClass.h"
#include "vector3i.h"
#include "w3d.h"

typedef Vector3i16 TriIndex;
class AABTreeClass;
class ChunkSaveClass;
class XMLWriter;
class AABTreeBuilderClass
{
public:
	AABTreeBuilderClass();

	void				Build_AABTree(int poly_count, TriIndex * polys, int vertcount, Vector3 * verts, bool new_format);
	void				Build_AABTree(std::vector<TriIndex>&& polys, std::vector<Vector3>&& vers);
#ifndef W3X
	void				Export(ChunkSaveClass & csave);
#else
	void				Export(XMLWriter& csave);
#endif
	int					Node_Count();
	int					Poly_Count();
	enum 
	{ 
		MIN_POLYS_PER_NODE =		4,
		SMALL_VERTEX =				-100000,
		BIG_VERTEX =				100000
	};
private:
	struct CullNodeStruct 
	{
		CullNodeStruct()
			: PolyIndices()
			, Front(nullptr)
			, Back(nullptr)
			, Min(0,0,0)
			, Max(0,0,0)
			, Index(0)
		{}

		std::vector<uint32>             PolyIndices;
		std::unique_ptr<CullNodeStruct> Front;
		std::unique_ptr<CullNodeStruct> Back;
		Vector3                         Min;
		Vector3                         Max;
		uint32                          Index;
	};
	struct SplitChoiceStruct
	{
		SplitChoiceStruct(void) : 
			Cost(FLT_MAX),
			FrontCount(0),
			BackCount(0),
			BMin(BIG_VERTEX,BIG_VERTEX,BIG_VERTEX),
			BMax(SMALL_VERTEX,SMALL_VERTEX,SMALL_VERTEX),
			FMin(BIG_VERTEX,BIG_VERTEX,BIG_VERTEX),
			FMax(SMALL_VERTEX,SMALL_VERTEX,SMALL_VERTEX),
			Plane(AAPlaneClass::XNORMAL,0) 
		{
		}

		//CFE: We do the FRONT/BACK splitting logic twice currently. It's may be cheaper to stash the vertices here than do the tests again later 

		float        Cost;
		uint32       FrontCount;
		uint32       BackCount;
		Vector3      BMin;
		Vector3      BMax;
		Vector3      FMin;
		Vector3      FMax;
		AAPlaneClass Plane;
	};

	struct SplitArraysStruct
	{
		SplitArraysStruct()
			: FrontPolys()
			, BackPolys()
		{
		}

		std::vector<uint32> FrontPolys;
		std::vector<uint32> BackPolys;
	};
	enum OverlapType
	{
		 NONE       = 0x00
		,POS        = 0x01
		,NEG        = 0x02
		,ON         = 0x04
		,BOTH       = 0x08
		,OUTSIDE    = POS
		,INSIDE     = NEG
		,OVERLAPPED = BOTH
		,FRONT      = POS
		,BACK       = NEG
	};
	void              Reset();
	void              Build_AABTree();
	void              Build_Tree(CullNodeStruct& node, std::vector<uint32>&& poly_indices);
	SplitChoiceStruct Select_Splitting_Plane(const std::vector<uint32>& poly_indices) const;
	SplitChoiceStruct Compute_Plane_Score(const std::vector<uint32>& poly_indices, const AAPlaneClass & plane) const;
	void              Split_Polys(std::vector<uint32>&& poly_indices, const SplitChoiceStruct& sc, SplitArraysStruct& arrays) const;
	OverlapType       Which_Side(const AAPlaneClass & plane,int poly_index) const;
	void              Compute_Bounding_Box(CullNodeStruct& node);
	int               Assign_Index(CullNodeStruct& node,int index);
	int               Node_Count_Recursive(CullNodeStruct& node,int curcount);
	void              Update_Min(const int poly_index, Vector3& set_min) const;
	void              Update_Max(const int poly_index, Vector3& set_max) const;
	void              Update_Min_Max(int poly_index, Vector3 & set_min, Vector3 & set_max) const;
	void              Build_W3D_AABTree_Recursive(CullNodeStruct& node, std::vector<W3dMeshAABTreeNode>& w3d_nodes, std::vector<uint32>& poly_indices);

	std::unique_ptr<CullNodeStruct> m_root;
	std::vector<TriIndex>           m_polys;
	std::vector<Vector3>            m_verts;
	bool m_newFormat;

	friend class AABTreeClass;
};

#endif