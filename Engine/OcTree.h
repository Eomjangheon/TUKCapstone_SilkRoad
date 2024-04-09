#pragma once

class OcNode;
class GameObject;
class BaseCollider;

class Terrain;

class OcTree
{
private:
	shared_ptr<OcNode> m_rootNode;	// �ֻ��� ���
	int m_minSize;			// �ּ� ������

public:
	OcTree(int maxSize, int minSize);
	~OcTree();

	/* ������Ʈ�� AABB�� Ʈ���� ���� */
	void InsertObjectCollider(shared_ptr<BaseCollider> bs);
	/* �浹 �˻� */
	void CollisionInspection(shared_ptr<BaseCollider> bs);

	void CollisionTerrain(shared_ptr<BaseCollider> bs);

	void Update();
	void UpdateOcnode(shared_ptr<OcNode> currentNode);

private:
	/* ������Ʈ�� AABB�� ���ϴ� ��带 ã�� ��ȯ */
	shared_ptr<OcNode> FindColliderIncludedNode(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);
	/* ���� ��忡 �θ� ���� �浹 �˻縦 ���� */
	void CollisionInspectionToParrent(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);
	/* ���� ��忡 �ڽĿ� ���� �浹 �˻縦 ���� */
	void CollisionInspectionToChild(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);

	bool CollisionSphere(shared_ptr<BoundingSphere> mainSphere, shared_ptr<BoundingSphere> subSphere,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	bool CollisionSphereBox(shared_ptr<BoundingSphere> mainSphere, shared_ptr<BoundingOrientedBox> mainCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth , bool isBoxMain);

	bool CollisionBox(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	void ProjectCube(shared_ptr < array<Vec3, 8>>, Vec3 axis, shared_ptr<float> min, shared_ptr<float> max);

	void ProjectSphere(Vec3 center, float radius, Vec3 axis, shared_ptr<float> min, shared_ptr<float> max);
	
	bool ProjectileFromCubePlane(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	bool ProjectileFromCubeEdges(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	bool ProjectileFromCubePlaneWithSphere(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingSphere> mainSphere,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	shared_ptr<Terrain> m_terrain;
};