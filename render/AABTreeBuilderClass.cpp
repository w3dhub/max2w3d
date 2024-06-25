#include "General.h"
#include <numeric>
#include "AABTreeBuilderClass.h"
#include "vector3i.h"
#ifndef W3X
#include "chunkclass.h"
#else
#include "xmlwriter.h"
#endif

#define CHECK_NODES 0

const float COINCIDENCE_EPSILON = 0.001f;
AABTreeBuilderClass::AABTreeBuilderClass(void)
	: m_root(nullptr)
	, m_polys()
	, m_verts()
	, m_newFormat(false)
{
}

void AABTreeBuilderClass::Reset(void)
{
	m_root = nullptr;
	m_verts.clear();
	m_polys.clear();
}

void AABTreeBuilderClass::Build_AABTree()
{
	TT_PROFILER_SCOPE(__FUNCTION__)
	std::vector<uint32> poly_indices(m_polys.size());
	std::iota(poly_indices.begin(), poly_indices.end(), 0);

	Build_Tree(*m_root, std::move(poly_indices));


	Compute_Bounding_Box(*m_root);
	Assign_Index(*m_root, 0);
}

void AABTreeBuilderClass::Build_AABTree(int poly_count,TriIndex * polys, int vertcount, Vector3* verts, bool new_format)
{
	m_newFormat = new_format;
	m_verts.assign(verts, verts + vertcount);
	m_polys.assign(polys, polys + poly_count);
	
	m_root = std::make_unique<CullNodeStruct>();

	Build_AABTree();
}

void AABTreeBuilderClass::Build_AABTree(std::vector<TriIndex>&& polys, std::vector<Vector3>&& verts)
{
	m_verts = std::move(verts);
	m_polys = std::move(polys);

	m_root = std::make_unique<CullNodeStruct>();

	Build_AABTree();
}

void AABTreeBuilderClass::Build_Tree(CullNodeStruct& node, std::vector<uint32>&& poly_indices)
{
	const size_t poly_count = poly_indices.size();

	if (poly_count <= MIN_POLYS_PER_NODE)
	{
		node.PolyIndices = std::move(poly_indices);
		return;
	}

	SplitChoiceStruct sc = Select_Splitting_Plane(poly_indices);
	if (sc.FrontCount + sc.BackCount != poly_count)
	{
		node.PolyIndices = std::move(poly_indices);
		return;
	}
	SplitArraysStruct arrays;
	Split_Polys(std::move(poly_indices), sc, arrays);

	if (arrays.FrontPolys.empty() == false)
	{
		node.Front = std::make_unique<CullNodeStruct>();
		Build_Tree(*node.Front, std::move(arrays.FrontPolys));
	}
	if (arrays.BackPolys.empty() == false)
	{
		node.Back = std::make_unique<CullNodeStruct>();
		Build_Tree(*node.Back, std::move(arrays.BackPolys));
	}
}
AABTreeBuilderClass::SplitChoiceStruct AABTreeBuilderClass::Select_Splitting_Plane(const std::vector<uint32>& poly_indices) const
{
	constexpr int MAX_NUM_TRYS = 50;
	const size_t poly_count = poly_indices.size();
	const int num_trys = min(MAX_NUM_TRYS, (int)poly_count);

	SplitChoiceStruct best_plane_stats;
	for (int trys = 0; trys < num_trys; ++trys)
	{
		AAPlaneClass plane;
		int poly_index = poly_indices[rand() % poly_count];
		const TriIndex polyverts = m_polys[poly_index];
		const Vector3 vert = m_verts[polyverts[rand() % 3]];
		switch(rand() % 3)
		{
			case 0:	plane.Set(AAPlaneClass::XNORMAL, vert.X);	break;
			case 1:	plane.Set(AAPlaneClass::YNORMAL, vert.Y);	break;
			case 2:	plane.Set(AAPlaneClass::ZNORMAL, vert.Z);	break;
		};
		SplitChoiceStruct considered_plane_stats = Compute_Plane_Score(poly_indices, plane);
		if (considered_plane_stats.Cost < best_plane_stats.Cost)
		{
			best_plane_stats = considered_plane_stats;
		}
	}

	return best_plane_stats;
}
AABTreeBuilderClass::SplitChoiceStruct AABTreeBuilderClass::Compute_Plane_Score(const std::vector<uint32>& poly_indices, const AAPlaneClass & plane) const
{
	SplitChoiceStruct sc;
	sc.Plane = plane;

	for (int poly_index : poly_indices)
	{
		switch(Which_Side(plane, poly_index))
		{
			case FRONT:
			case ON:
			case BOTH: //CFE: Is this correct? If the flag is set to both it'll only add it to the front node
				{
					++sc.FrontCount;
					Update_Min_Max(poly_index, sc.FMin, sc.FMax);
					break;
				}
			case BACK:		
				{
					++sc.BackCount;
					Update_Min_Max(poly_index, sc.BMin,sc.BMax );
					break;
				}
		}
	}

	sc.BMin -= Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);
	sc.BMax += Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);

	if ((sc.FrontCount == 0) || (sc.BackCount == 0))
	{
		sc.Cost = FLT_MAX;
	}
	else
	{
		const float back_cost = (sc.BMax.X - sc.BMin.X) * (sc.BMax.Y - sc.BMin.Y) * (sc.BMax.Z - sc.BMin.Z) * sc.BackCount;
		const float front_cost = (sc.FMax.X - sc.FMin.X) * (sc.FMax.Y - sc.FMin.Y) * (sc.FMax.Z - sc.FMin.Z) * sc.FrontCount;
		sc.Cost = front_cost + back_cost;
	}

	return sc;
}
AABTreeBuilderClass::OverlapType AABTreeBuilderClass::Which_Side(const AAPlaneClass& plane, const int poly_index) const
{
	int mask = NONE;

	const TriIndex poly = m_polys[poly_index];
	for (unsigned short vert_index : poly)
	{
		const Vector3 point = m_verts[vert_index];
		const float delta = point[plane.Normal] - plane.Dist;
		if (delta > COINCIDENCE_EPSILON)
		{
			mask |= POS;
		} 
		if (delta < -COINCIDENCE_EPSILON)
		{
			mask |= NEG;
		}
		mask |= ON;
	}
	if (mask == ON)
	{
		return ON;
	}
	if ((mask & ~(POS | ON)) == 0)
	{
		return POS;
	}
	if ((mask & ~(NEG | ON)) == 0)
	{
		return NEG;
	}
	return BOTH;
}
void AABTreeBuilderClass::Split_Polys(std::vector<uint32>&& poly_indices, const SplitChoiceStruct& sc, SplitArraysStruct& arrays) const
{
	arrays.FrontPolys.reserve(sc.FrontCount);
	arrays.BackPolys.reserve(sc.BackCount);

	TT_ASSERT(sc.FrontCount + sc.BackCount == poly_indices.size());
	for (int poly_index : poly_indices)
	{
		switch(Which_Side(sc.Plane, poly_index))
		{
			case FRONT: 
			case ON:
			case BOTH:
				arrays.FrontPolys.emplace_back(poly_index);
				break;
			case BACK:
				arrays.BackPolys.emplace_back(poly_index);
				break;
		}
	}
}
void AABTreeBuilderClass::Compute_Bounding_Box(CullNodeStruct& node)
{
	if (node.Front)
	{
		Compute_Bounding_Box(*node.Front);
	}
	if (node.Back)
	{
		Compute_Bounding_Box(*node.Back);
	}
	node.Min.Set(100000.0f,100000.0f,100000.0f);
	node.Max.Set(-100000.0f,-100000.0f,-100000.0f);
	for (int poly_index : node.PolyIndices)
	{
		Update_Min_Max(poly_index, node.Min,node.Max );
	}
	if (node.Front)
	{
		if (node.Front->Min.X < node.Min.X) node.Min.X = node.Front->Min.X;
		if (node.Front->Max.X > node.Max.X) node.Max.X = node.Front->Max.X;
		if (node.Front->Min.Y < node.Min.Y) node.Min.Y = node.Front->Min.Y;
		if (node.Front->Max.Y > node.Max.Y) node.Max.Y = node.Front->Max.Y;
		if (node.Front->Min.Z < node.Min.Z) node.Min.Z = node.Front->Min.Z;
		if (node.Front->Max.Z > node.Max.Z) node.Max.Z = node.Front->Max.Z;
	}
	if (node.Back)
	{
		if (node.Back->Min.X < node.Min.X) node.Min.X = node.Back->Min.X;
		if (node.Back->Max.X > node.Max.X) node.Max.X = node.Back->Max.X;
		if (node.Back->Min.Y < node.Min.Y) node.Min.Y = node.Back->Min.Y;
		if (node.Back->Max.Y > node.Max.Y) node.Max.Y = node.Back->Max.Y;
		if (node.Back->Min.Z < node.Min.Z) node.Min.Z = node.Back->Min.Z;
		if (node.Back->Max.Z > node.Max.Z) node.Max.Z = node.Back->Max.Z;
	}
#if CHECK_NODES
	TT_RELEASE_ASSERT(isfinite(node.Min.X));
	TT_RELEASE_ASSERT(isfinite(node.Min.Y));
	TT_RELEASE_ASSERT(isfinite(node.Min.Z));
	TT_RELEASE_ASSERT(isfinite(node.Max.X));
	TT_RELEASE_ASSERT(isfinite(node.Max.Y));
	TT_RELEASE_ASSERT(isfinite(node.Max.Z));
#endif
}
int AABTreeBuilderClass::Assign_Index(CullNodeStruct& node,int index)
{
	node.Index = index;
	index++;
	if (node.Front)
	{
		index = Assign_Index(*node.Front,index);
	}
	if (node.Back)
	{
		index = Assign_Index(*node.Back,index);
	}
	return index;
}
int AABTreeBuilderClass::Node_Count(void)
{	
	if (m_root)
	{
		return Node_Count_Recursive(*m_root,0);
	}
	else
	{
		return 0;
	}
}

int AABTreeBuilderClass::Poly_Count() 
{ 
	return (int)m_polys.size();
}

int AABTreeBuilderClass::Node_Count_Recursive(CullNodeStruct& node, int curcount)
{
	++curcount;
	if (node.Front)
	{
		curcount = Node_Count_Recursive(*node.Front,curcount);
	}
	if (node.Back)
	{
		curcount = Node_Count_Recursive(*node.Back,curcount);
	}
	return curcount;
}
void AABTreeBuilderClass::Update_Min(const int poly_index, Vector3& min) const
{
	const TriIndex polyverts = m_polys[poly_index];

	for (unsigned short vert_index : polyverts)
	{
		Vector3 point = m_verts[vert_index];
		if (point.X < min.X) min.X = point.X;
		if (point.Y < min.Y) min.Y = point.Y;
		if (point.Z < min.Z) min.Z = point.Z;
	}
}
void AABTreeBuilderClass::Update_Max(int poly_index, Vector3& max) const
{
	const TriIndex polyverts = m_polys[poly_index];
	for (unsigned short vert_index : polyverts)
	{
		Vector3 point = m_verts[vert_index];
		if (point.X > max.X) max.X = point.X;
		if (point.Y > max.Y) max.Y = point.Y;
		if (point.Z > max.Z) max.Z = point.Z;
	}
}
void	AABTreeBuilderClass::Update_Min_Max(int poly_index, Vector3 & min, Vector3 & max) const
{
	const TriIndex polyverts = m_polys[poly_index];
	for (unsigned short vert_index : polyverts)
	{
		Vector3 point = m_verts[vert_index];
		if (point.X < min.X) min.X = point.X;
		if (point.Y < min.Y) min.Y = point.Y;
		if (point.Z < min.Z) min.Z = point.Z;
		if (point.X > max.X) max.X = point.X;
		if (point.Y > max.Y) max.Y = point.Y;
		if (point.Z > max.Z) max.Z = point.Z;
	}
}
#ifndef W3X
void AABTreeBuilderClass::Export(ChunkSaveClass & csave)
{
	//CFE: I don't expect the allocations to be a bottleneck here. If they are then I've got a safe static vector wrapper we can use
	csave.Begin_Chunk(W3DChunkType::AABBTREE);
	std::vector<W3dMeshAABTreeNode> nodes;
	nodes.reserve(Node_Count());

	std::vector<uint32> poly_indices;
	nodes.reserve(Poly_Count());

	Build_W3D_AABTree_Recursive(*m_root, nodes, poly_indices);

	csave.Begin_Chunk(W3DChunkType::AABBTREE_HEADER); //This could do with being RAII'd, but one problem at a time
	W3dMeshAABTreeHeader header;
	memset(&header,0,sizeof(header));
	header.NodeCount = (uint32)nodes.size();
	header.PolyCount = (uint32)poly_indices.size();
	csave.Write(&header,sizeof(header));
	csave.End_Chunk();

	csave.Begin_Chunk(W3DChunkType::AABBTREE_POLYINDICES);
	csave.Write(poly_indices.data(), (unsigned long)(poly_indices.size() * sizeof(uint32)));
	csave.End_Chunk();
	csave.Begin_Chunk(W3DChunkType::AABBTREE_NODES);
	
	csave.Write(nodes.data(), (unsigned long)(nodes.size() * sizeof(W3dMeshAABTreeNode)));
	csave.End_Chunk();
	csave.End_Chunk();
}
#else
void AABTreeBuilderClass::Export(XMLWriter& csave)
{
	//CFE: I don't expect the allocations to be a bottleneck here. If they are then I've got a safe static vector wrapper we can use
	std::vector<W3dMeshAABTreeNode> nodes;
	nodes.reserve(Node_Count());

	std::vector<uint32> poly_indices;
	nodes.reserve(Poly_Count());

	Build_W3D_AABTree_Recursive(*m_root, nodes, poly_indices);

	csave.StartTag("AABTree", 1);
	csave.EndTag();
	csave.StartTag("PolyIndices", 1);
	csave.EndTag();
	for (int i = 0; i < Poly_Count(); i++)
	{
		csave.WriteUnsignedInt("P", poly_indices[i]);
	}
	csave.WriteClosingTag();
	for (int i = 0; i < Node_Count(); i++)
	{
		csave.StartTag("Node", 1);
		csave.EndTag();
		csave.WriteVector("Min", nodes[i].Min);
		csave.WriteVector("Max", nodes[i].Max);
		if ((nodes[i].FrontOrPoly0 & 0x80000000) == 0)
		{
			csave.StartTag("Children", 0);
			csave.SetUnsignedIntAttribute("Front", nodes[i].FrontOrPoly0 & 0x7FFFFFFF);
			csave.SetUnsignedIntAttribute("Back", nodes[i].BackOrPolyCount);
		}
		else
		{
			csave.StartTag("Polys", 0);
			csave.SetUnsignedIntAttribute("Begin", nodes[i].FrontOrPoly0 & 0x7FFFFFFF);
			csave.SetUnsignedIntAttribute("Count", nodes[i].BackOrPolyCount);
		}
		csave.EndTag();
		csave.WriteClosingTag();
	}
	csave.WriteClosingTag();
}
#endif
void AABTreeBuilderClass::Build_W3D_AABTree_Recursive(CullNodeStruct& node, std::vector<W3dMeshAABTreeNode>& w3d_nodes, std::vector<uint32>& poly_indices)
{
	w3d_nodes.emplace_back(
		W3dMeshAABTreeNode{
			W3dVectorStruct{ node.Min.X, node.Min.Y , node.Min.Z}, //Min Bounds
			W3dVectorStruct{ node.Max.X, node.Max.Y , node.Max.Z}, //Max Bounds
			0,                                        //FrontPoly(or 0)
			0                                         //BackPoly(or count)
		});
	W3dMeshAABTreeNode& new_w3d_node = w3d_nodes.back();

	if (node.Front != nullptr)
	{
		new_w3d_node.FrontOrPoly0 = node.Front->Index;
		new_w3d_node.BackOrPolyCount = node.Back->Index;
	}
	else
	{
		new_w3d_node.FrontOrPoly0 = (uint32)poly_indices.size() | 0x80000000; //The field in the tree is FrontPolyOr0. Doesn't this break all of the assumptions that name implies?
		new_w3d_node.BackOrPolyCount = (uint32)node.PolyIndices.size();

	}

	poly_indices.insert(poly_indices.end(), node.PolyIndices.begin(), node.PolyIndices.end()); //CFE: This entire system is mixing signed and unsigned ints

	if (node.Front != nullptr)
	{
		Build_W3D_AABTree_Recursive(*node.Front, w3d_nodes, poly_indices);
	}
	if (node.Back != nullptr)
	{
		Build_W3D_AABTree_Recursive(*node.Back, w3d_nodes, poly_indices);
	}
}
