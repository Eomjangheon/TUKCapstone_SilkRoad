#include "pch.h"
#include "OcTree.h"
#include "OcNode.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "OrientedBoxCollider.h"
#include"SceneManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "RigidBody.h"
#include "Terrain.h"
#include "Transform.h"
#include "Timer.h"

OcTree::OcTree(int maxSize, int minSize)
{

	m_rootNode = make_shared<OcNode>(make_shared<BoundingBox>(Vec3(16000, 0, 16000), Vec3(maxSize, maxSize, maxSize)), nullptr);
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

			if (currentNode->GetBB()->Contains(*(sphereCollider->GetBoundingSphere()))== DirectX::CONTAINS) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*sphereCollider->GetBoundingSphere())== DirectX::CONTAINS)
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

			if (currentNode->GetBB()->Contains(*boxCollider->GetBoundingBox())== DirectX::CONTAINS) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*boxCollider->GetBoundingBox())== DirectX::CONTAINS)
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
		else if (bs->GetColliderType() == ColliderType::OrientedBox) {
			shared_ptr<OrientedBoxCollider> boxOrientedCollider = dynamic_pointer_cast<OrientedBoxCollider>(bs);

			if (currentNode->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == DirectX::CONTAINS) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == DirectX::CONTAINS)
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

	
	CollisionTerrain(bs);
	// �θ� ���鿡 ���� �浹 �˻�
	CollisionInspectionToParrent(bs, IncludedNode);
	// �ڽ� ���鿡 ���� �浹 �˻�
	CollisionInspectionToChild(bs, IncludedNode);

	
}



void OcTree::Update()
{
	//UpdateOcnode(m_rootNode);

}

void OcTree::UpdateOcnode(shared_ptr<OcNode> currentNode)
{
	
	currentNode->Update();
	for (int i = 0; i < currentNode->IncludedObjectAABBCount(); ++i)
	{
		if (currentNode->IncludedObjectAABB(i)->updatePos) {
			currentNode->IncludedObjectAABB(i)->updatePos = false;
			shared_ptr<BaseCollider> tempCol = currentNode->IncludedObjectAABB(i);
			int id = tempCol->GetColliderId();
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

		if (currentNode->GetBB()->Contains(*sphereCollider->GetBoundingSphere())==DirectX::CONTAINS) {
			return currentNode;
		}
	}
	else if (bs->GetColliderType() == ColliderType::Box) {
		shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(bs);

		if (currentNode->GetBB()->Contains(*boxCollider->GetBoundingBox())== DirectX::CONTAINS) {
			return currentNode;
		}
	}
	else if (bs->GetColliderType() == ColliderType::OrientedBox) {
		shared_ptr<OrientedBoxCollider> boxOrientedCollider = dynamic_pointer_cast<OrientedBoxCollider>(bs);

		if (currentNode->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == DirectX::CONTAINS) {
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
array<int, 3> planeX{ 0,3,1 };
array<int, 3> planeY{ 1,2,5 };
array<int, 3> planeZ{ 2,6,6 };

array<int, 3> lineX{ 0,1,1 };
array<int, 3> lineY{ 1,2,5 };

void OcTree::CollisionInspectionToParrent(shared_ptr<BaseCollider> bs, shared_ptr<OcNode> currentNode)
{
	// �ֻ��� ������ ���� ��� ����
	if (currentNode == nullptr)
		return;


	for (int i = 0; i < currentNode->IncludedObjectAABBCount(); i++)
	{
		shared_ptr<BaseCollider> bsDst = currentNode->IncludedObjectAABB(i);


		// �ڱ� �ڽſ� ���� �浹�˻�� �������� ����
		if (bs->GetColliderId() >= bsDst->GetColliderId())
			continue;

		shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
		shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();
		if (rb1->GetIsStatic() && rb2->GetIsStatic())
			continue;

		shared_ptr<Vec3> normal = make_shared<Vec3>();
		shared_ptr<float> depth = make_shared<float>();

		*normal = { 0,0,0 };
		*depth = 0.f;

		// �ڱ� �ڽſ� ���� �浹�˻�� �������� ����
		if (bs->GetColliderId() >= bsDst->GetColliderId())
			continue;

		// baseColliser�� Sphere�� ���
		if (bs->GetColliderType() == ColliderType::Sphere) {
			shared_ptr<BoundingSphere> boundingSphereSrc = dynamic_pointer_cast<SphereCollider>(bs)->GetBoundingSphere();

			//����� Sphere�� ���
			if (bsDst->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

				if (!CollisionSphere(boundingSphereSrc, boundingSphereDst, normal, depth))
					continue;
			}

			//����� OBB�� ���
			else if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
				shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();

				if (!CollisionSphereBox(boundingSphereSrc, boundingOrientedBoxDst, normal, depth,false))
					continue;
			}

		}

		// baseColliser�� OBB�� ���
		else if (bs->GetColliderType() == ColliderType::OrientedBox) {
			shared_ptr<BoundingOrientedBox> boundingOrientedBoxSrc = dynamic_pointer_cast<OrientedBoxCollider>(bs)->GetBoundingOrientedBox();

			//����� OBB�� ���
			if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
				shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();

				if (!CollisionBox(boundingOrientedBoxSrc, boundingOrientedBoxDst, normal, depth))
					continue;
			}

			//����� Sphere�� ���
			else if (bsDst->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

				if (!CollisionSphereBox(boundingSphereDst, boundingOrientedBoxSrc, normal, depth,true))
					continue;
			}
		}

		
		bs->setColor(Vec4(1, 0, 0, 0), true);
		bsDst->setColor(Vec4(1, 0, 0, 0), true);

		if (rb1->GetIsStatic()) {
			rb2->Move(*normal * (*depth));
		}

		else if (rb2->GetIsStatic()) {
			rb1->Move(-(*normal) * (*depth));
		}
		else {
			rb1->Move(-(*normal) * (*depth) / 2);
			rb2->Move(*normal * (*depth) / 2);
		}



		Vec3 relativeVelocity = rb2->GetLinearVelocity() - rb1->GetLinearVelocity();
		if (relativeVelocity.Dot(*normal) > 0.f) {
			continue;
		}

		float e = min(rb1->GetRestitution(), rb2->GetRestitution());
		float j = -(1.f + e) * relativeVelocity.Dot(*normal);
		j /= rb1->GetInvMass() + rb2->GetInvMass();
		Vec3 impulse = (*normal) * j;

		rb1->SetLinearVelocity(rb1->GetLinearVelocity() - impulse * rb1->GetInvMass());
		rb2->SetLinearVelocity(rb2->GetLinearVelocity() + impulse * rb2->GetInvMass());

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

		for (int m = 0; m < childNode->IncludedObjectAABBCount(); m++)
		{
			shared_ptr<BaseCollider> bsDst = childNode->IncludedObjectAABB(m);

			shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
			shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();
			if (rb1->GetIsStatic() && rb2->GetIsStatic())
				continue;


			
			shared_ptr<Vec3> normal = make_shared<Vec3>();
			shared_ptr<float> depth = make_shared<float>();

			*normal = { 0,0,0 };
			*depth = 0.f;

			// �ڱ� �ڽſ� ���� �浹�˻�� �������� ����
			if (bs->GetColliderId() >= bsDst->GetColliderId())
				continue;

			// baseColliser�� Sphere�� ���
			if (bs->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<BoundingSphere> boundingSphereSrc = dynamic_pointer_cast<SphereCollider>(bs)->GetBoundingSphere();

				//����� Sphere�� ���
				if (bsDst->GetColliderType() == ColliderType::Sphere) {
					shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

					if (!CollisionSphere(boundingSphereSrc, boundingSphereDst, normal, depth))
						continue;
				}

				//����� OBB�� ���
				else if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
					shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();
					
					if(!CollisionSphereBox(boundingSphereSrc, boundingOrientedBoxDst, normal, depth, false))
						continue;
				}

			}

			// baseColliser�� OBB�� ���
			else if (bs->GetColliderType() == ColliderType::OrientedBox) {
				shared_ptr<BoundingOrientedBox> boundingOrientedBoxSrc = dynamic_pointer_cast<OrientedBoxCollider>(bs)->GetBoundingOrientedBox();

				//����� OBB�� ���
				if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
					shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();

					if (!CollisionBox(boundingOrientedBoxSrc, boundingOrientedBoxDst, normal, depth))
						continue;
				}

				//����� Sphere�� ���
				else if (bsDst->GetColliderType() == ColliderType::Sphere) {
					shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

					if (!CollisionSphereBox(boundingSphereDst, boundingOrientedBoxSrc, normal, depth, true))
						continue;
				}
			}	

			
			bs->setColor(Vec4(1, 0, 0, 0), true);
			bsDst->setColor(Vec4(1, 0, 0, 0), true);

			if (rb1->GetIsStatic()) {
				rb2->Move(*normal * (*depth));
			}
			
			else if (rb2->GetIsStatic()) {
				rb1->Move(-(*normal) * (*depth));
			}
			else {
				rb1->Move(-(*normal) * (*depth) / 2.f);
				rb2->Move(*normal * (*depth) / 2.f);
			}
			


			Vec3 relativeVelocity = rb2->GetLinearVelocity() - rb1->GetLinearVelocity();
			if (relativeVelocity.Dot(*normal) > 0.f) {
				continue;
			}

			float e = min(rb1->GetRestitution(), rb2->GetRestitution());
			float j = -(1.f + e) * relativeVelocity.Dot(*normal);
			j /= rb1->GetInvMass() + rb2->GetInvMass();
			Vec3 impulse = (*normal) * j;

			rb1->SetLinearVelocity(rb1->GetLinearVelocity() - impulse * rb1->GetInvMass());
			rb2->SetLinearVelocity(rb2->GetLinearVelocity() + impulse * rb2->GetInvMass());
		}

		// �ڽĳ���� �ڽĳ��鿡 ���� �浹 �˻縦 ��������� ����
		CollisionInspectionToChild(bs, childNode);
	}
}

//�ͷ��ΰ��� �浹�� ���� �Լ�
void OcTree::CollisionTerrain(shared_ptr<BaseCollider> bs)
{
	shared_ptr<RigidBody> rb = bs->GetRigidBody();
	if (rb->GetIsStatic())
		return;


	if (!m_terrain) {
		m_terrain = GET_SINGLE(SceneManager)->GetActiveScene()->m_terrain;
	}

	//�ͷ����� ���� ���� ��ġ ��� ���̿� normal�� �ҷ��´�.
	Vec3 pos = bs->GetRigidBody()->GetPosition();
	shared_ptr<Vec3> norm = make_shared<Vec3>();
	shared_ptr<float> h = make_shared < float >();
	m_terrain->getHeight(pos.x, pos.z, h, norm);

	//�����޽ÿ��� �浹�� �����ϱ� ���� �ӽ��� ����ü �浹ü�� �����.
	shared_ptr<BoundingOrientedBox> OBB = make_shared<BoundingOrientedBox>();
	OBB->Center = Vec3(pos.x, *h-100, pos.z);
	OBB->Extents = Vec3 (100, 100, 100);


	shared_ptr<Vec3> normal = make_shared<Vec3>();
	shared_ptr<float> depth = make_shared<float>();

	*normal = { 0,0,0 };
	*depth = 0.f;

	//�浹 �˻縦 ���� ������ �ּ� �Ÿ��� ������ ��´�
	if (bs->GetColliderType() == ColliderType::Sphere) {
		shared_ptr<BoundingSphere> boundingSphereSrc = dynamic_pointer_cast<SphereCollider>(bs)->GetBoundingSphere();


		if (!CollisionSphereBox(boundingSphereSrc, OBB, normal, depth, false)){
			return;
		}

	}
	// baseColliser�� OBB�� ���


	else if (bs->GetColliderType() == ColliderType::OrientedBox) {
		shared_ptr<BoundingOrientedBox> boundingOrientedBoxSrc = dynamic_pointer_cast<OrientedBoxCollider>(bs)->GetBoundingOrientedBox();

		//����� OBB�� ���

		if (!CollisionBox(boundingOrientedBoxSrc, OBB, normal, depth)){
			return;
		}
	}

	//������ ������ �ݴ�������� �����δ�.
	bs->setColor(Vec4(1, 0, 0, 0), true);
	rb->Move(-(*normal) * (*depth));

	//���� �ͷ����� �������� �������� �������Ѵ�.
	Vec3 relativeVelocity = - rb->GetLinearVelocity();
	if (relativeVelocity.Dot(*normal) > 0.f) {
		return;
	}

	float e = min(rb->GetRestitution(),0.5);
	float j = -(1.f + e) * relativeVelocity.Dot(*norm);
	j /= rb->GetInvMass() + 1.f/1000.f;
	Vec3 impulse = (*norm) * j;

	rb->SetLinearVelocity(rb->GetLinearVelocity() - impulse * rb->GetInvMass());
	

}

bool OcTree::CollisionSphere(shared_ptr<BoundingSphere> mainSphere, shared_ptr<BoundingSphere> subSphere, shared_ptr<Vec3> normal, shared_ptr<float> depth)
{

	Vec3 CenterSrc = mainSphere->Center;
	Vec3 CenterDst = subSphere->Center;

	float distance = SimpleMath::Vector3::Distance(CenterSrc, CenterDst);
	float radii = mainSphere->Radius + subSphere->Radius;

	if (distance >= radii)
		return false;

	*normal = CenterDst - CenterSrc;
	(*normal).Normalize();
	*depth = radii - distance;

	return true;
}

bool OcTree::CollisionSphereBox(shared_ptr<BoundingSphere> mainSphere, shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<Vec3> normal, shared_ptr<float> depth, bool isBoxMain)
{
	bool cantCol = false;

	*depth = FLT_MAX;

	cantCol = ProjectileFromCubePlaneWithSphere(mainCube, mainSphere, normal, depth);
	if (!cantCol) {


		Vec3 sphereCenter = mainSphere->Center;
		Vec3 centerB = mainCube->Center;
		Vec3 dir = sphereCenter - centerB;
		if (!isBoxMain)
			dir = centerB - sphereCenter;
		if (dir.Dot(*normal) < 0.f) {
			*normal = -(*normal);
		}
	}
	else {
		return false;
	}
	return true;

	//*depth = FLT_MAX;
	//Vec3 axis;
	//float axisDepth = 0.f;
	//std::shared_ptr<float> minA = std::make_shared<float>();
	//std::shared_ptr<float> maxA = std::make_shared<float>();
	//std::shared_ptr<float> minB = std::make_shared<float>();
	//std::shared_ptr<float> maxB = std::make_shared<float>();

	//XMFLOAT3 cornersA[8];
	//shared_ptr<array<Vec3, 8>> cornersArrayA = make_shared<array<Vec3, 8>>();
	//mainCube->GetCorners(cornersA);

	//for (int i = 0; i < 8; ++i) {
	//	(*cornersArrayA)[i] = cornersA[i];
	//}

	//for (int k = 0; k < 3; ++k) {
	//	Vec3 va = (*cornersArrayA)[planeX[k]];
	//	Vec3 vb = (*cornersArrayA)[planeY[k]];
	//	Vec3 vc = (*cornersArrayA)[planeZ[k]];

	//	Vec3 edgeA = va - vb;
	//	Vec3 edgeB = va - vc;
	//	Vec3 axis = XMVector3Cross(edgeA, edgeB);
	//	axis.Normalize();

	//	ProjectCube(cornersArrayA, axis, minA, maxA);
	//	ProjectSphere(mainSphere->Center, mainSphere->Radius, axis, minB, maxB);

	//	if (*minA >= *maxB || *minB >= *maxA) {
	//		return false;
	//	}

	//	axisDepth = min(*maxB - *minA, *maxA - *minB);
	//	if (axisDepth < *depth) {
	//		*depth = axisDepth;
	//		*normal = axis;
	//	}
	//}

	//int result = -1;
	//float minDistance = FLT_MAX;
	//Vec3 sphereCenter = mainSphere->Center;
	//Vec3 boxCenter = mainCube->Center;

	//for (int j = 0; j < 8; ++j) {
	//	Vec3 v = cornersA[j];
	//	float distance = (v - sphereCenter).Length();
	//	if (distance < minDistance) {
	//		minDistance = distance;
	//		result = j;
	//	}
	//}
	//Vec3 cp = cornersA[result];
	//axis = cp - sphereCenter;
	//axis.Normalize();
	//
	//ProjectCube(cornersArrayA, axis, minA, maxA);
	//ProjectSphere(mainSphere->Center, mainSphere->Radius, axis, minB, maxB);
	//if (*minA >= *maxB || *minB >= *maxA)
	//{
	//	return false;
	//}
	//axisDepth = min(*maxB - *minA, *maxA - *minB);
	//if (axisDepth < *depth) {
	//	*depth = axisDepth;
	//	*normal = axis;
	//}
	//Vec3 dir = boxCenter - sphereCenter;
	///*if (!isBoxMain)
	//	dir = sphereCenter - mainCube->Center;*/


	//if (dir.Dot(*normal) < 0.f) {
	//	*normal = -(*normal);
	//}
	//return true;
}

bool OcTree::CollisionBox(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube, shared_ptr<Vec3> normal, shared_ptr<float> depth)
{
	bool cantCol = false;
	*depth = FLT_MAX;

	cantCol = ProjectileFromCubePlane(mainCube, subCube, normal, depth);
	if (!cantCol)	cantCol = ProjectileFromCubePlane(subCube, mainCube, normal, depth);
	else return false;
	if (!cantCol)	cantCol = ProjectileFromCubeEdges(mainCube, subCube, normal, depth);
	else return false;
	if (!cantCol) {
		*depth /= (*normal).Length();
		(*normal).Normalize();

		Vec3 centerA = mainCube->Center;
		Vec3 centerB = subCube->Center;
		Vec3 dir = centerB - centerA;

		if (dir.Dot(*normal) < 0.f) {
			*normal = -(*normal);
		}

		
	}
	return true;
	/**depth = FLT_MAX;

	XMFLOAT3 cornersA[8];
	XMFLOAT3 cornersB[8];
	mainCube->GetCorners(cornersA);
	subCube->GetCorners(cornersB);
	shared_ptr<array<Vec3, 8>> cornersArrayA = make_shared<array<Vec3, 8>>();
	shared_ptr<array<Vec3, 8>> cornersArrayB = make_shared<array<Vec3, 8>>();
	shared_ptr<float> minA = std::make_shared<float>();
	shared_ptr<float> maxA = std::make_shared<float>();
	shared_ptr<float> minB = std::make_shared<float>();
	shared_ptr<float> maxB = std::make_shared<float>();

	for (int i = 0; i < 8; ++i) {
		(*cornersArrayA)[i] = cornersA[i];
		(*cornersArrayB)[i] = cornersB[i];
	}

	for (int k = 0; k < 3; ++k) {
		Vec3 va = (*cornersArrayA)[planeX[k]];
		Vec3 vb = (*cornersArrayA)[planeY[k]];
		Vec3 vc = (*cornersArrayA)[planeZ[k]];

		Vec3 edgeA = va - vb;
		Vec3 edgeB = va - vc;
		Vec3 axis = XMVector3Cross(edgeA, edgeB);

		ProjectCube(cornersArrayA, axis, minA, maxA);
		ProjectCube(cornersArrayB, axis, minB, maxB);
		

		if (*minA >= *maxB || *minB >= *maxA) {
			return false;
		}

		float axisDepth = min((*maxB - *minA), (*maxA - *minB));
		if (axisDepth < *depth) {
			*depth = axisDepth;
			*normal = axis;
		}
	}

	for (int k = 0; k < 3; ++k) {
		Vec3 va = (*cornersArrayB)[planeX[k]];
		Vec3 vb = (*cornersArrayB)[planeY[k]];
		Vec3 vc = (*cornersArrayB)[planeZ[k]];

		Vec3 edgeA = va - vb;
		Vec3 edgeB = va - vc;
		Vec3 axis = XMVector3Cross(edgeA, edgeB);

		ProjectCube(cornersArrayA, axis, minA, maxA);
		ProjectCube(cornersArrayB, axis, minB, maxB);


		if (*minA >= *maxB || *minB >= *maxA) {
			return false;
		}

		float axisDepth = min((*maxB - *minA), (*maxA - *minB));
		if (axisDepth < *depth) {
			*depth = axisDepth;
			*normal = axis;
		}
	}

	for (int q = 0; q < 3; ++q) {
		Vec3 va1 = cornersA[lineX[q]];
		Vec3 va2 = cornersA[lineY[q]];
		Vec3 edgeA = va1 - va2;

		for (int w = 0; w < 3; ++w) {
			Vec3 vb1 = cornersB[lineX[w]];
			Vec3 vb2 = cornersB[lineY[w]];
			Vec3 edgeB = vb1 - vb2;
			Vec3 axis = XMVector3Cross(edgeA, edgeB);
			

			if (axis.Length() < FLT_EPSILON)
				continue;

			*minA = FLT_MAX;
			*maxA = -FLT_MAX;

			*minB = FLT_MAX;
			*maxB = -FLT_MAX;

			for (int j = 0; j < 8; ++j) {
				Vec3 VA = cornersA[j];
				float projA = VA.Dot(axis);
				*minA = min(*minA, projA);
				*maxA = max(*maxA, projA);
			}

			for (int j = 0; j < 8; ++j) {
				Vec3 VB = cornersB[j];
				float projB = VB.Dot(axis);
				*minB = min(*minB, projB);
				*maxB = max(*maxB, projB);
			}

			if (minA >= maxB || minB >= maxA) {
				return false;
			}

			float axisDepth = min((*maxB - *minA), (*maxA - *minB));
			if (axisDepth < *depth) {
				*depth = axisDepth;
				*normal = axis;
			}

		}
	}
	*depth /= (*normal).Length();
	(*normal).Normalize();

	Vec3 centerA = mainCube->Center;
	Vec3 centerB = subCube->Center;
	Vec3 dir = centerB - centerA;

	if (dir.Dot(*normal) < 0.f) {
		*normal = -(*normal);
	}

	return true;*/

	

}

void OcTree::ProjectCube(shared_ptr<array<Vec3, 8>> corners, Vec3 axis, shared_ptr<float> minimum, shared_ptr<float> maximun)
{
	*minimum = FLT_MAX;
	*maximun = -FLT_MAX;

	for (int j = 0; j < 8; ++j) {
		Vec3 VA = (*corners)[j];
		float projA = VA.Dot(axis);

		*minimum = min(*minimum, projA);
		*maximun = max(*maximun, projA);
	}
}

void OcTree::ProjectSphere(Vec3 center, float radius, Vec3 axis, shared_ptr<float> min, shared_ptr<float> max)
{
	Vec3 dirAndRadius = axis * radius;
	Vec3 p1 = center + dirAndRadius;
	Vec3 p2 = center - dirAndRadius;

	*min = p1.Dot(axis);
	*max = p2.Dot(axis);
	if (*min > *max) {
		float temp = *min;
		*min = *max;
		*max = temp;
	}
}

bool OcTree::ProjectileFromCubePlane(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube, shared_ptr<Vec3> normal, shared_ptr<float> depth)
{
	XMFLOAT3 cornersA[8];
	XMFLOAT3 cornersB[8];
	mainCube->GetCorners(cornersA);
	subCube->GetCorners(cornersB);

	for (int k = 0; k < 3; ++k) {
		Vec3 va = cornersA[planeX[k]];
		Vec3 vb = cornersA[planeY[k]];
		Vec3 vc = cornersA[planeZ[k]];

		Vec3 edgeA = va - vb;
		Vec3 edgeB = va - vc;
		Vec3 norm = XMVector3Cross(edgeA, edgeB);
		norm.Normalize();

		float minA = FLT_MAX;
		float maxA = -FLT_MAX;

		float minB = FLT_MAX;
		float maxB = -FLT_MAX;

		for (int j = 0; j < 8; ++j) {
			Vec3 VA = cornersA[j];
			float projA = VA.Dot(norm);
			minA = min(minA, projA);
			maxA = max(maxA, projA);
		}

		for (int j = 0; j < 8; ++j) {
			Vec3 VB = cornersB[j];
			float projB = VB.Dot(norm);
			minB = min(minB, projB);
			maxB = max(maxB, projB);
		}

		if (minA >= maxB || minB >= maxA) {
			return true;
		}

		float axisDepth = min(maxB - minA, maxA - minB);
		if (axisDepth < *depth) {
			*depth = axisDepth;
			*normal = norm;
		}
	}
	return false;
}

bool OcTree::ProjectileFromCubeEdges(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingOrientedBox> subCube, shared_ptr<Vec3> normal, shared_ptr<float> depth)
{
	XMFLOAT3 cornersA[8];
	XMFLOAT3 cornersB[8];
	mainCube->GetCorners(cornersA);
	subCube->GetCorners(cornersB);

	for (int q = 0; q < 3; ++q) {
		Vec3 va1 = cornersA[lineX[q]];
		Vec3 va2 = cornersA[lineY[q]];
		Vec3 edgeA = va1 - va2;

		for (int w = 0; w < 3; ++w) {
			Vec3 vb1 = cornersB[lineX[w]];
			Vec3 vb2 = cornersB[lineY[w]];
			Vec3 edgeB = vb1 - vb2;
			Vec3 norm = XMVector3Cross(edgeA, edgeB);
			norm.Normalize();
			if (norm.Length() < FLT_EPSILON)
				continue;

			float minA = FLT_MAX;
			float maxA = -FLT_MAX;

			float minB = FLT_MAX;
			float maxB = -FLT_MAX;

			for (int j = 0; j < 8; ++j) {
				Vec3 VA = cornersA[j];
				float projA = VA.Dot(norm);
				minA = min(minA, projA);
				maxA = max(maxA, projA);
			}

			for (int j = 0; j < 8; ++j) {
				Vec3 VB = cornersB[j];
				float projB = VB.Dot(norm);
				minB = min(minB, projB);
				maxB = max(maxB, projB);
			}

			if (minA >= maxB || minB >= maxA) {
				return true;
			}

			float axisDepth = min((maxB - minA), (maxA - minB));
			if (axisDepth < *depth) {
				*depth = axisDepth;
				*normal = norm;
			}

		}
	}
	return false;
}

bool OcTree::ProjectileFromCubePlaneWithSphere(shared_ptr<BoundingOrientedBox> mainCube, shared_ptr<BoundingSphere> mainSphere, shared_ptr<Vec3> normal, shared_ptr<float> depth)
{
	XMFLOAT3 cornersA[8];
	mainCube->GetCorners(cornersA);

	for (int k = 0; k < 3; ++k) {
		Vec3 va = cornersA[planeX[k]];
		Vec3 vb = cornersA[planeY[k]];
		Vec3 vc = cornersA[planeZ[k]];

		Vec3 edgeA = va - vb;
		Vec3 edgeB = va - vc;
		Vec3 norm = XMVector3Cross(edgeA, edgeB);
		norm.Normalize();

		float radius = mainSphere->Radius;
		Vec3 dirAndRadius = norm * radius;
		Vec3 p1 = mainSphere->Center + dirAndRadius;
		Vec3 p2 = mainSphere->Center - dirAndRadius;


		float minA = FLT_MAX;
		float maxA = -FLT_MAX;

		float minB = FLT_MAX;
		float maxB = -FLT_MAX;

		for (int j = 0; j < 8; ++j) {
			Vec3 VA = cornersA[j];
			float projA = VA.Dot(norm);

			minA = min(minA, projA);
			maxA = max(maxA, projA);
		}

		minB = p1.Dot(norm);
		maxB = p2.Dot(norm);
		if (minB > maxB) {
			float temp = minB;
			minB = maxB;
			maxB = temp;
		}



		if (minA >= maxB || minB >= maxA) {
			return true;
		}

		float axisDepth = min(maxB - minA, maxA - minB);
		if (axisDepth < *depth) {
			*depth = axisDepth;
			*normal = norm;
		}
	}
	return false;
}
