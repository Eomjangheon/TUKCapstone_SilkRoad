#pragma once
#include "BaseCollider.h"

class BaseCollider;


class OcNode:public std::enable_shared_from_this<OcNode>
{
private: 
	shared_ptr<OcNode> m_parentNode;				// �θ� ���
	shared_ptr<OcNode> m_childNodes[8];					// �ڽĵ��� ���
	shared_ptr<BoundingBox> m_bb;							// �� ����� AABB
	vector<shared_ptr<BaseCollider>> m_includedColliders;	// �� ��忡 ���ϴ� ������Ʈ�� AABB
	
	int m_includedCollidersCount;				// �� ��忡 ���ϴ� ������Ʈ�� AABB ����
	bool m_isHaveChilds;						// �ڽ� ��尡 �����ϴ��� ����

public:
	OcNode(shared_ptr<BoundingBox> bb, shared_ptr<OcNode> parentNode);
	~OcNode();

public:
	/* ������Ʈ�� AABB�� ��� */
	void InsertObjectCollider(shared_ptr<BaseCollider> bs);
	/* ���� ����� AABB�� 4���� �Ͽ� �ڽ� ��带 ���� */
	void SplitBy8();
	/* ���� ��忡 �Ķ���ͷ� �Ѿ�� ������Ʈ AABB�� �ִ��� �˻� */
	bool ObjectColliderContains(shared_ptr<BaseCollider> bs);

	void Update();
	void DeleteCol(int id);

	// Get, Set
public:
	shared_ptr<OcNode> GetParentNode() { return m_parentNode; };
	shared_ptr<OcNode> GetChildNode(int index) { return m_childNodes[index]; }
	shared_ptr<BoundingBox> GetBB() { return m_bb; }
	shared_ptr<BaseCollider> IncludedObjectAABB(int index) { return m_includedColliders[index]; }
	int IncludedObjectAABBCount() { return m_includedCollidersCount; }
	bool IsHaveChilds() { return m_isHaveChilds; }


	shared_ptr<GameObject> m_go;
	bool include = false;
};