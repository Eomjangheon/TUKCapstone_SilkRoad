#pragma once

class OcNode;
class GameObject;
class BaseCollider;

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

	void Update();
	void UpdateOcnode(shared_ptr<OcNode> currentNode);

private:
	/* ������Ʈ�� AABB�� ���ϴ� ��带 ã�� ��ȯ */
	shared_ptr<OcNode> FindColliderIncludedNode(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);
	/* ���� ��忡 �θ� ���� �浹 �˻縦 ���� */
	void CollisionInspectionToParrent(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);
	/* ���� ��忡 �ڽĿ� ���� �浹 �˻縦 ���� */
	void CollisionInspectionToChild(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode);

	bool ProjectileFromCubePlane(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	bool ProjectileFromCubeEdges(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);

	bool ProjectileFromCubePlaneWithSphere(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingSphere> mainSphere,
		shared_ptr<Vec3> normal, shared_ptr<float>depth);
};