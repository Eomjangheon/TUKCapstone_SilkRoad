#include "pch.h"
#include "OcTree.h"
#include "OcNode.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include"SceneManager.h"
#include "Scene.h"
#include "GameObject.h"

OcTree::OcTree(int maxSize, int minSize)
{

	m_rootNode = make_shared<OcNode>(make_shared<BoundingBox>(Vec3(0, 0, 0), Vec3(maxSize, maxSize, maxSize)), nullptr);
	m_minSize = minSize;
}

OcTree::~OcTree()
{

}

void OcTree::InsertObjectCollider(shared_ptr<BaseCollider> bs)
{
	shared_ptr<OcNode> currentNode = m_rootNode;
	while (true)
	{
		// �� �̻� ���� �� ���� ��� �ݺ��� ����
		if (currentNode->GetBB()->Extents.x / 2 < m_minSize)
			break;


		if (bs->GetColliderType() == ColliderType::Sphere) {
			shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(bs);

			if (currentNode->GetBB()->Contains(*(sphereCollider->GetBoundingSphere()))==2) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*sphereCollider->GetBoundingSphere())==2)
					{
						currentNode = currentNode->GetChildNode(i);
						break;
					}

					notIncludedCount++;
				}

				// ������ ���ԵǴ� �ڽ� ��尡 ���� ��� �ݺ��� ����
				if (notIncludedCount == 8)
					break;
			}
		}
		else if (bs->GetColliderType() == ColliderType::Box) {
			shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(bs);

			if (currentNode->GetBB()->Contains(*boxCollider->GetBoundingBox())==2) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*boxCollider->GetBoundingBox())==2)
					{
						currentNode = currentNode->GetChildNode(i);
						break;
					}

					notIncludedCount++;
				}

				// ������ ���ԵǴ� �ڽ� ��尡 ���� ��� �ݺ��� ����
				if (notIncludedCount == 8)
					break;
			}
		}

	}

	// ������Ʈ�� AABB�� ���ؾ��ϴ� ��ġ�� ����
	currentNode->InsertObjectCollider(bs);
}

void OcTree::CollisionInspection(shared_ptr<BaseCollider> bs)
{
	// ��Ʈ ��忡������ �Ķ���ͷ� �Ѿ�� ������Ʈ AABB�� ���ϴ� ��带 ã��
	shared_ptr<OcNode> IncludedNode = FindColliderIncludedNode(bs, m_rootNode);

	

	// �θ� ���鿡 ���� �浹 �˻�
	CollisionInspectionToParrent(bs, IncludedNode);
	// �ڽ� ���鿡 ���� �浹 �˻�
	CollisionInspectionToChild(bs, IncludedNode);

}

void OcTree::Update()
{
	UpdateOcnode(m_rootNode);

}

void OcTree::UpdateOcnode(shared_ptr<OcNode> currentNode)
{
	currentNode->Update();
	for (int i = 0; i < currentNode->IncludedObjectAABBCount(); ++i)
	{
		if (currentNode->IncludedObjectAABB(i)->updatePos) {
			currentNode->IncludedObjectAABB(i)->updatePos = false;
			shared_ptr<BaseCollider> tempCol = currentNode->IncludedObjectAABB(i);
			int id =tempCol->GetColliderId();
			currentNode->DeleteCol(id);
			InsertObjectCollider(tempCol);
		}
	}

	if (!currentNode->IsHaveChilds())
		return;

	for (int i = 0; i < 8; i++)
	{
		shared_ptr<OcNode> childNode = currentNode->GetChildNode(i);
		UpdateOcnode(childNode);
	}
}



shared_ptr<OcNode> OcTree::FindColliderIncludedNode(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode)
{
	// ���� ��忡 �Ķ���ͷ� �Ѿ�� ������Ʈ AABB�� ���� ���
	// ���� ��带 ��ȯ
	if (bs->GetColliderType() == ColliderType::Sphere) {
		shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(bs);

		if (currentNode->GetBB()->Contains(*sphereCollider->GetBoundingSphere())==2) {
			return currentNode;
		}
	}
	else if (bs->GetColliderType() == ColliderType::Box) {
		shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(bs);

		if (currentNode->GetBB()->Contains(*boxCollider->GetBoundingBox())==2) {
			return currentNode;
		}
	}
	


	shared_ptr<OcNode> IncludedNode = nullptr;

	// ���� ��忡 �ڽ��� ���� ��� ���� ù ��° �ڽĺ��� ��ͷ� �˻�
	if (currentNode->IsHaveChilds())
	{
		for (int i = 0; i < 8 && IncludedNode == nullptr; i++)
			IncludedNode = FindColliderIncludedNode(bs, currentNode->GetChildNode(i));
	}

	return IncludedNode;
}

void OcTree::CollisionInspectionToParrent(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode)
{
	// �ֻ��� ������ ���� ��� ����
	if (currentNode == nullptr)
		return;

	

	for (int i = 0; i < currentNode->IncludedObjectAABBCount(); i++)
	{

		// �ڱ� �ڽſ� ���� �浹�˻�� �������� ����
		if (bs->GetColliderId() == currentNode->IncludedObjectAABB(i)->GetColliderId())
			continue;


		if (currentNode->IncludedObjectAABB(i)->GetColliderType() == ColliderType::Sphere) {
			shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(currentNode->IncludedObjectAABB(i));
			if (bs->Intersects(sphereCollider->GetBoundingSphere())) {
				bs->setColor(Vec4(1, 0, 0, 0), true);
				sphereCollider->setColor(Vec4(1, 0, 0, 0), true);
			}
		}
		else if (currentNode->IncludedObjectAABB(i)->GetColliderType() == ColliderType::Box) {
			shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(currentNode->IncludedObjectAABB(i));
			if (bs->Intersects(boxCollider->GetBoundingBox())) {
				bs->setColor(Vec4(1, 0, 0, 0), true);
				boxCollider->setColor(Vec4(1, 0, 0, 0), true);
			}	
		}
		
	}

	// ���� ����� �θ� ���� �浹 �˻縦 ��������� ����
	CollisionInspectionToParrent(bs, currentNode->GetParentNode());
}

void OcTree::CollisionInspectionToChild(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode)
{
	/*if (!currentNode->include) {
		currentNode->include = true;
		GET_SINGLE(SceneManager)->GetActiveScene()->AddGameObject(currentNode->m_go);
	}*/
	// �ڽ��� ������ ����
	if (!currentNode->IsHaveChilds())
		return;

	

	for (int i = 0; i < 8; i++)
	{
		shared_ptr<OcNode> childNode = currentNode->GetChildNode(i);

		for (int i = 0; i < childNode->IncludedObjectAABBCount(); i++)
		{
			if (bs->GetColliderId() == childNode->IncludedObjectAABB(i)->GetColliderId())
				continue;
			// �Ķ���ͷ� �Ѿ�� aabb�� �ڽ� ��忡 ���ϴ� ������Ʈ AABB�� �浹�ϴ��� �˻�
			if (childNode->IncludedObjectAABB(i)->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(childNode->IncludedObjectAABB(i));
				if (bs->Intersects(sphereCollider->GetBoundingSphere())) {
					bs->setColor(Vec4(1, 0, 0, 0), true);
					sphereCollider->setColor(Vec4(1, 0, 0, 0), true);
				}
			}
			else if (childNode->IncludedObjectAABB(i)->GetColliderType() == ColliderType::Box) {
				shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(childNode->IncludedObjectAABB(i));
				if (bs->Intersects(boxCollider->GetBoundingBox())) {
					bs->setColor(Vec4(1, 0, 0, 0), true);
					boxCollider->setColor(Vec4(1, 0, 0, 0), true);
				}
			}
	
		}

		// �ڽĳ���� �ڽĳ��鿡 ���� �浹 �˻縦 ��������� ����
		CollisionInspectionToChild(bs, childNode);
	}
}
