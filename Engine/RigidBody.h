#pragma once
#include "Component.h"

class Terrain;
class BaseCollider;

class RigidBody : public Component
{
public:
	RigidBody();
	virtual ~RigidBody();

	virtual void Awake() override;
	virtual void FinalUpdate() override;
	

public:
	//���� ��ġ�� ���� ������� ��ǥ �̵�
	void	Move(Vec3 amount);

	//���� ��ġ�� ������� �������� ��ǥ �̵�
	void	MoveTo(Vec3 position);

	//���� �������� ���� ���Ѵ�. ������ Ŭ���� ���� ������ ����
	void	AddForce(Vec3 amount) { m_force = amount; }

	//������ ��ũ���� ����ϴ� �Լ��̹Ƿ� ���ǻ��X
	void	MovementStep(int iterations);


	//getter, setter
public:
	Vec3	GetPosition() { return m_position; }
	Vec3	GetLinearVelocity() { return m_linearVelocity; }
	float	GetMass() { return m_mass; }
	float	GetRestitution() { return m_restitution; }
	bool	GetStatic() { return m_isStatic; }
	float	GetInvMass() {
		if (m_isStatic)return 0;
		else return m_invMass; }
	float	GetMaxSpeed() { return m_maxSpeed; }

	//mass, restitution, static, maxSpeed�� ���������� ������ ����Ʈ ������ �����
	void	SetLinearVelocity(Vec3 linearVelocity) { m_linearVelocity = linearVelocity; }
	void	SetMass(float mass) { m_mass = mass; m_invMass = 1 / mass; }
	void	SetRestitution(float restitution) { m_restitution = restitution; }
	void	SetStatic(bool isStatic) { m_isStatic = isStatic; }
	void	SetMaxSpeed(float maxSpeed) { m_maxSpeed = maxSpeed; }

	
private:
	

private:
	Vec3 m_position = {};					
	Vec3 m_linearVelocity = {};				
	Vec3 m_rotation = {};
	Vec3 m_rotationVelocity = {};
	Vec3 m_force = {};
	float m_mass = 50.f;			//kg����				
	float m_restitution = 0.5f;		//ź�� 0~1 0�� �������� ź���� �پ����
	float m_invMass= 1/50.f;
	bool m_isStatic = false;

	float m_maxSpeed = 1000.f;		//�ְ�ӵ� cm/s���� 
	

	Vec3 m_gravity = { 0.f,-980.f,0.f };
	

	shared_ptr< Terrain > m_gameTerrain;
	shared_ptr<BaseCollider> m_baseCollider;

	
};

