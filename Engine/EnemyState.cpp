#include "pch.h"
#include "EnemyState.h"
#include "Player.h"
#include "Animator.h"
#include "RigidBody.h"
#include "Transform.h"
#include "GameObject.h"
#include "Timer.h"
#include "Input.h"
#include "Enemy.h"


shared_ptr<EnemyState> EnemyState::OnUpdateState()
{
	if (m_enemy->GetHP() <= 0)
		return make_shared<EnemyDieState>(m_enemy);
	return nullptr;
}


shared_ptr<EnemyState> EnemyIdleState::OnUpdateState()
{
	for (int i = 0; i < m_enemy->GetPlayers().size(); ++i)
	{
		shared_ptr<GameObject> player = m_enemy->GetPlayers()[i];
		Vec3 toPlayer = player->GetTransform()->GetLocalPosition() - m_enemy->GetTransform()->GetLocalPosition();
		float dist = toPlayer.Length();
		if (dist < m_enemy->GetChaseRange())
		{
			m_enemy->SetTargetPlayerIndex(i);
			return make_shared<EnemyWalkState>(m_enemy);
		}

	}
	return EnemyState::OnUpdateState();
}

void EnemyIdleState::OnEnter()
{
	m_enemy->GetAnimator()->Play(static_cast<uint32>(ENEMY_STATE::IDLE));
}

shared_ptr<EnemyState> EnemyWalkState::OnUpdateState()
{
	shared_ptr<GameObject> player = m_enemy->GetPlayers()[m_enemy->GetTargetPlayerIndex()];
	Vec3 toPlayer = player->GetTransform()->GetLocalPosition() - m_enemy->GetTransform()->GetLocalPosition();
	float dist = toPlayer.Length();
	if (dist < m_enemy->GetAttackRange())
	{
		if(m_enemy->GetAttackReady())
			return make_shared<EnemyAttackState>(m_enemy);
	}
	else if (dist > m_enemy->GetChaseRange())
	{
		return make_shared<EnemyIdleState>(m_enemy);
	}
	else
	{
		Vec3 dir = toPlayer;
		dir.y = 0;
		shared_ptr<RigidBody> rb = m_enemy->GetRigidBody();
		shared_ptr<Transform> transform = m_enemy->GetTransform();

		dir.Normalize();
		rb->SetMaxSpeed(m_enemy->GetMaxWalkSpeed());
		transform->LookAt(dir);

		Vec3 force = dir * m_enemy->GetWalkForce();
		rb->AddForce(force);
	}
	return EnemyState::OnUpdateState();
}

void EnemyWalkState::OnEnter()
{
	m_enemy->GetAnimator()->Play(static_cast<uint32>(ENEMY_STATE::WALK));
	// �÷��̾� Ÿ��
}

shared_ptr<EnemyState> EnemyAttackState::OnUpdateState()
{
	return EnemyState::OnUpdateState();
}

shared_ptr<EnemyState> EnemyAttackState::OnLateUpdateState()
{
	if (m_enemy->GetAnimator()->IsAnimationEndOnThisFrame())
	{
		return make_shared<EnemyIdleState>(m_enemy);
	}
}

void EnemyAttackState::OnEnter()
{
	m_enemy->GetAnimator()->Play(static_cast<uint32>(ENEMY_STATE::ATTACK));
}

shared_ptr<EnemyState> EnemyDieState::OnUpdateState()
{
	return nullptr;
}

void EnemyDieState::OnEnter()
{
	m_enemy->GetAnimator()->Play(static_cast<uint32>(ENEMY_STATE::DIE));
	m_enemy->GetRigidBody()->SetStatic(true);
	m_enemy->GetRigidBody()->SetOverlap();
	m_enemy->GetRigidBody()->SetLinearVelocity(Vec3(0, 0, 0));
	//m_enemy->GetGameObject()->SetActive(false);
}

shared_ptr<EnemyState> EnemyDieState::OnLateUpdateState()
{
	if (m_enemy->GetAnimator()->IsAnimationEndOnThisFrame())
	{
		m_enemy->GetRigidBody()->MoveTo(Vec3(-1.f, 0, 0));
		return make_shared<EnemyIdleState>(m_enemy);
	}
}