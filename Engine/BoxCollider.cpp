#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"
#include "Mesh.h"
#include "Resources.h"
#include "MeshRenderer.h"
#include "SceneManager.h"
#include "Scene.h"

BoxCollider::BoxCollider() : BaseCollider(ColliderType::Box)
{
	m_boundingBox = make_shared<BoundingBox>();
	m_go = make_shared<GameObject>();
	m_go->AddComponent(make_shared<Transform>());
	//m_go->GetTransform()->SetLocalPosition(this->GetTransform()->GetLocalPosition());
	Vec3 extents = m_boundingBox->Extents;
	m_go->GetTransform()->SetLocalScale(extents * 2);//100,200,100


	//m_boundingBox->Center = GetGameObject()->GetTransform()->GetWorldPosition();
	//Vec3 scale = m_go->GetTransform()->GetLocalScale();
	//m_boundingBox->Extents = m_extent/2;

	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	{
		shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadCubeMesh();
		meshRenderer->SetMesh(sphereMesh);
	}
	{
		shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"WireFrame");
		//shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Wood", L"..\\Resources\\Texture\\Wood.jpg");
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(shader);
		//material->SetTexture(0, texture);
		meshRenderer->SetMaterial(material);
	}
	m_go->AddComponent(meshRenderer);
	
}

BoxCollider::~BoxCollider()
{

}

void BoxCollider::Awake()
{
	
	
	

}

void BoxCollider::FinalUpdate()
{


	m_go->GetTransform()->SetLocalPosition(this->GetTransform()->GetLocalPosition());
	m_boundingBox->Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 extents = m_boundingBox->Extents;
	m_go->GetTransform()->SetLocalScale(extents * 2);//100,200,100
}

bool BoxCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return m_boundingBox->Intersects(rayOrigin, rayDir, OUT distance);
}

bool BoxCollider::Intersects(shared_ptr<BoundingSphere> boundingSphere)
{
	return m_boundingBox->Intersects(*boundingSphere);
}

bool BoxCollider::Intersects(shared_ptr<BoundingBox> boundingBox)
{
	return m_boundingBox->Intersects(*boundingBox);
}

void BoxCollider::SetRadius(float radius)
{
}

void BoxCollider::SetCenter(Vec3 center)
{
	m_boundingBox->Center = center;
}

void BoxCollider::SetExtent(Vec3 extent)
{
	m_boundingBox->Extents = extent;
}

void BoxCollider::draw()
{

}