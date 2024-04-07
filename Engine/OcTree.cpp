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
		else if (bs->GetColliderType() == ColliderType::OrientedBox) {
			shared_ptr<OrientedBoxCollider> boxOrientedCollider = dynamic_pointer_cast<OrientedBoxCollider>(bs);

			if (currentNode->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == 2) {
				if (!currentNode->IsHaveChilds())
					currentNode->SplitBy8();

				// ������ ���Ե��� �ʴ� �ڽĵ��� ����
				int notIncludedCount = 0;

				for (int i = 0; i < 8; i++)
				{
					// �ڽ� ��忡 ������ ���ԵǸ� ���� ��带 
					// �ڽ� ���� ���� �� ó���ܰ���� �ٽ� �˻�
					if (currentNode->GetChildNode(i)->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == 2)
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
	else if (bs->GetColliderType() == ColliderType::OrientedBox) {
		shared_ptr<OrientedBoxCollider> boxOrientedCollider = dynamic_pointer_cast<OrientedBoxCollider>(bs);

		if (currentNode->GetBB()->Contains(*boxOrientedCollider->GetBoundingOrientedBox()) == 2) {
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
array<int, 3> planeY{ 1,2,2 };
array<int, 3> planeZ{ 2,6,5 };

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

		// baseColliser�� Sphere�� ���
		if (bs->GetColliderType() == ColliderType::Sphere) {
			shared_ptr<BoundingSphere> boundingSphereSrc = dynamic_pointer_cast<SphereCollider>(bs)->GetBoundingSphere();

			//����� Sphere�� ���
			if (bsDst->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

				if (!boundingSphereSrc->Intersects(*boundingSphereDst)) { continue;}

				/*bs->setColor(Vec4(1, 0, 0, 0), true);
				bsDst->setColor(Vec4(1, 0, 0, 0), true);*/

				Vec3 CenterSrc = boundingSphereSrc->Center;
				Vec3 CenterDst = boundingSphereDst->Center;

				float distance = SimpleMath::Vector3::Distance(CenterSrc, CenterDst);
				float radii = boundingSphereSrc->Radius + boundingSphereDst->Radius;


				Vec3 normal = CenterDst - CenterSrc;
				normal.Normalize();
				float depth = radii - distance;


				shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
				shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();

				rb1->Move(-normal * depth / 2);
				rb2->Move(normal * depth / 2);


			}
		}

		// baseColliser�� OBB�� ���
		else if (bs->GetColliderType() == ColliderType::OrientedBox) {
			shared_ptr<BoundingOrientedBox> boundingOrientedBoxSrc = dynamic_pointer_cast<OrientedBoxCollider>(bs)->GetBoundingOrientedBox();

			//����� OBB�� ���
			if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
				shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();

				bool cantCol = false;
				Vec3 normal;
				float depth = FLT_MAX;

				XMFLOAT3 cornersA[8];
				XMFLOAT3 cornersB[8];
				boundingOrientedBoxSrc->GetCorners(cornersA);
				boundingOrientedBoxDst->GetCorners(cornersB);

				for (int k = 0; k < 3; ++k) {
					if (cantCol)
						break;

					Vec3 va = cornersA[planeX[k]];
					Vec3 vb = cornersA[planeY[k]];
					Vec3 vc = cornersA[planeZ[k]];

					Vec3 edgeA = va - vb;
					Vec3 edgeB = va - vc;
					Vec3 norm = XMVector3Cross(edgeA, edgeB);


					float minA = FLT_MAX;
					float maxA = FLT_MIN;

					float minB = FLT_MAX;
					float maxB = FLT_MIN;

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
						cantCol = true;
						break;
					}

					float axisDepth = min(maxB - minA, maxA - minB);
					if (axisDepth < depth) {
						depth = axisDepth;
						normal = norm;
					}
				}

				for (int k = 0; k < 3; ++k) {
					if (cantCol)
						break;

					Vec3 va = cornersB[planeX[k]];
					Vec3 vb = cornersB[planeY[k]];
					Vec3 vc = cornersB[planeZ[k]];

					Vec3 edgeA = va - vb;
					Vec3 edgeB = va - vc;
					Vec3 norm = XMVector3Cross(edgeA, edgeB);


					float minA = FLT_MAX;
					float maxA = FLT_MIN;

					float minB = FLT_MAX;
					float maxB = FLT_MIN;

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
						cantCol = true;
						break;
					}

					float axisDepth = min((maxB - minA), (maxA - minB));
					if (axisDepth < depth) {
						depth = axisDepth;
						normal = norm;
					}
				}

				for (int q = 0; q < 3; ++q) {
					if (cantCol)
						break;

					Vec3 va1 = cornersA[lineX[q]];
					Vec3 va2 = cornersA[lineY[q]];
					Vec3 edgeA = va1 - va2;

					for (int w = 0; w < 3; ++w) {
						Vec3 vb1 = cornersB[lineX[w]];
						Vec3 vb2 = cornersB[lineY[w]];
						Vec3 edgeB = vb1 - vb2;
						Vec3 norm = XMVector3Cross(edgeA, edgeB);

						if (norm.Length() < FLT_EPSILON)
							continue;

						float minA = FLT_MAX;
						float maxA = FLT_MIN;

						float minB = FLT_MAX;
						float maxB = FLT_MIN;

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
							cantCol = true;
							break;
						}

						float axisDepth = min((maxB - minA), (maxA - minB));
						if (axisDepth < depth) {
							depth = axisDepth;
							normal = norm;
						}

					}
				}

				if (!cantCol) {
					depth /= normal.Length();
					normal.Normalize();

					Vec3 centerA = boundingOrientedBoxSrc->Center;
					Vec3 centerB = boundingOrientedBoxDst->Center;
					Vec3 dir = centerB - centerA;

					if (dir.Dot(normal) < 0.f) {
						normal = -normal;
					}

					shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
					shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();
					bs->setColor(Vec4(1, 0, 0, 0), true);
					bsDst->setColor(Vec4(1, 0, 0, 0), true);
					rb1->Move(-normal * depth / 2);
					rb2->Move(normal * depth / 2);;
				}

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

		for (int m = 0; m < childNode->IncludedObjectAABBCount(); m++)
		{
			shared_ptr<BaseCollider> bsDst = childNode->IncludedObjectAABB(m);
			// �ڱ� �ڽſ� ���� �浹�˻�� �������� ����
			if (bs->GetColliderId() >= bsDst->GetColliderId())
				continue;

			// baseColliser�� Sphere�� ���
			if (bs->GetColliderType() == ColliderType::Sphere) {
				shared_ptr<BoundingSphere> boundingSphereSrc = dynamic_pointer_cast<SphereCollider>(bs)->GetBoundingSphere();

				//����� Sphere�� ���
				if (bsDst->GetColliderType() == ColliderType::Sphere) {
					shared_ptr<BoundingSphere> boundingSphereDst = dynamic_pointer_cast<SphereCollider>(bsDst)->GetBoundingSphere();

					if (!boundingSphereSrc->Intersects(*boundingSphereDst)) { continue; }

					/*bs->setColor(Vec4(1, 0, 0, 0), true);
					bsDst->setColor(Vec4(1, 0, 0, 0), true);*/

					Vec3 CenterSrc = boundingSphereSrc->Center;
					Vec3 CenterDst = boundingSphereDst->Center;

					float distance = SimpleMath::Vector3::Distance(CenterSrc, CenterDst);
					float radii = boundingSphereSrc->Radius + boundingSphereDst->Radius;


					Vec3 normal = CenterDst - CenterSrc;
					normal.Normalize();
					float depth = radii - distance;


					shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
					shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();

					rb1->Move(-normal * depth / 2);
					rb2->Move(normal * depth / 2);


				}

			}

			// baseColliser�� OBB�� ���
			else if (bs->GetColliderType() == ColliderType::OrientedBox) {
				shared_ptr<BoundingOrientedBox> boundingOrientedBoxSrc = dynamic_pointer_cast<OrientedBoxCollider>(bs)->GetBoundingOrientedBox();

				//����� OBB�� ���
				if (bsDst->GetColliderType() == ColliderType::OrientedBox) {
					shared_ptr<BoundingOrientedBox> boundingOrientedBoxDst = dynamic_pointer_cast<OrientedBoxCollider>(bsDst)->GetBoundingOrientedBox();

					bool cantCol = false;
					Vec3 normal;
					float depth = FLT_MAX;

					XMFLOAT3 cornersA[8];
					XMFLOAT3 cornersB[8];
					boundingOrientedBoxSrc->GetCorners(cornersA);
					boundingOrientedBoxDst->GetCorners(cornersB);

					for (int k = 0; k < 3; ++k) {
						if (cantCol)
							break;

						Vec3 va = cornersA[planeX[k]];
						Vec3 vb = cornersA[planeY[k]];
						Vec3 vc = cornersA[planeZ[k]];

						Vec3 edgeA = va - vb;
						Vec3 edgeB = va - vc;
						Vec3 norm = XMVector3Cross(edgeA, edgeB);


						float minA = FLT_MAX;
						float maxA = FLT_MIN;

						float minB = FLT_MAX;
						float maxB = FLT_MIN;

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
							cantCol = true;
							break;
						}

						float axisDepth = min(maxB - minA, maxA - minB);
						if (axisDepth < depth) {
							depth = axisDepth;
							normal = norm;
						}
					}

					for (int k = 0; k < 3; ++k) {
						if (cantCol)
							break;

						Vec3 va = cornersB[planeX[k]];
						Vec3 vb = cornersB[planeY[k]];
						Vec3 vc = cornersB[planeZ[k]];

						Vec3 edgeA = va - vb;
						Vec3 edgeB = va - vc;
						Vec3 norm = XMVector3Cross(edgeA, edgeB);


						float minA = FLT_MAX;
						float maxA = FLT_MIN;

						float minB = FLT_MAX;
						float maxB = FLT_MIN;

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
							cantCol = true;
							break;
						}

						float axisDepth = min((maxB - minA), (maxA - minB));
						if (axisDepth < depth) {
							depth = axisDepth;
							normal = norm;
						}
					}

					for (int q = 0; q < 3; ++q) {
						if (cantCol)
							break;

						Vec3 va1 = cornersA[lineX[q]];
						Vec3 va2 = cornersA[lineY[q]];
						Vec3 edgeA = va1 - va2;

						for (int w = 0; w < 3; ++w) {
							Vec3 vb1 = cornersB[lineX[w]];
							Vec3 vb2 = cornersB[lineY[w]];
							Vec3 edgeB = vb1 - vb2;
							Vec3 norm = XMVector3Cross(edgeA, edgeB);

							if (norm.Length() < FLT_EPSILON)
								continue;

							float minA = FLT_MAX;
							float maxA = FLT_MIN;

							float minB = FLT_MAX;
							float maxB = FLT_MIN;

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
								cantCol = true;
								break;
							}

							float axisDepth = min((maxB - minA), (maxA - minB));
							if (axisDepth < depth) {
								depth = axisDepth;
								normal = norm;
							}

						}
					}

					if (!cantCol) {
						depth /= normal.Length();
						normal.Normalize();

						Vec3 centerA = boundingOrientedBoxSrc->Center;
						Vec3 centerB = boundingOrientedBoxDst->Center;
						Vec3 dir = centerB - centerA;

						if (dir.Dot(normal) < 0.f) {
							normal = -normal;
						}


						shared_ptr<RigidBody> rb1 = bs->GetRigidBody();
						shared_ptr<RigidBody> rb2 = bsDst->GetRigidBody();
						bs->setColor(Vec4(1, 0, 0, 0), true);
						bsDst->setColor(Vec4(1, 0, 0, 0), true);
						rb1->Move(-normal * depth / 2);
						rb2->Move(normal * depth / 2);;
					}

				}


			}

			
	
		}

		// �ڽĳ���� �ڽĳ��鿡 ���� �浹 �˻縦 ��������� ����
		CollisionInspectionToChild(bs, childNode);
	}
}
