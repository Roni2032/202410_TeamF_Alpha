/*!
@file Bomb.cpp
@brief ���e����
*/

#include "stdafx.h"
#include "Project.h"

namespace basecross{
	void Bomb::OnCreate() {
		auto drawComp = AddComponent<PNTStaticDraw>();
		drawComp->SetMeshResource(L"BOMB_MD");
		drawComp->SetTextureResource(L"BOMB_MD_TEX");
		Mat4x4 matrix;
		matrix.affineTransformation(
			Vec3(1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, -0.5f, 0.0f)
		);
		drawComp->SetMeshToTransformMatrix(matrix);

		auto shadow = AddComponent<Shadowmap>();
		shadow->SetMeshResource(L"BOMB_MD");
		shadow->SetMeshToTransformMatrix(matrix);

		auto col = AddComponent<CollisionSphere>();
		col->AddExcludeCollisionTag(L"Player");

		auto gravity = AddComponent<BCGravity>();
		gravity->Jump(m_ThrowVelocity);

		GetComponent<Transform>()->SetPosition(m_Pos);
		GetComponent<Transform>()->SetScale(Vec3(0.5f));

		m_GameStage = GetTypeStage<GameStage>();

		Block::CollisionObjects.push_back(GetComponent<Transform>());

		auto parent = GetTypeStage<GameStage>()->m_Player->GetComponent<Transform>()->GetParent();
		if (parent != nullptr) {
			GetComponent<Transform>()->SetParent(parent);
		}

		m_Player = m_GameStage->GetSharedGameObject<GameObject>(L"Player", false);

	}

	void Bomb::OnUpdate() {
		float elapsedTime = App::GetApp()->GetElapsedTime();
		m_ExplodeTimer += elapsedTime;

		if (m_ExplodeTimer > m_ExplodeTime) {
			Explode();
		}

		Vec3 rot = GetComponent<Transform>()->GetRotation();
		rot += rotateSpeed * elapsedTime;
		GetComponent<Transform>()->SetRotation(rot);
	}

	void Bomb::Explode() {
		m_GameStage->AddGameObject<ExplodeCollider>(GetComponent<Transform>()->GetWorldPosition(),m_ExplodeStatus,m_Player.lock());
		
		m_GameStage->PlayParticle<ExplodeParticle>(L"EXPLODE_PCL", GetComponent<Transform>()->GetWorldPosition());

		m_GameStage->RemoveGameObject<Bomb>(GetThis<Bomb>());

		SoundManager::Instance().PlaySE(L"BOMB_SD",1.0f);

		if (m_GameStage->GetGameMode() == GameMode::NotBomb) {
			m_GameStage->ChangeMode(GameMode::InGame);
		}

	}
	void Bomb::OnCollisionEnter(shared_ptr<GameObject>& Other) {
		Explode();
		auto otherTrans = Other->GetComponent<Transform>();

		if (otherTrans->GetPosition().y - otherTrans->GetScale().y >= GetComponent<Transform>()->GetPosition().y) {
			auto gravity = GetComponent<BCGravity>();
			Vec3 velo = gravity->GetVelocity();
			gravity->SetVelocity(Vec3(velo.x, 0, velo.z));
		}
	}


	void ExplodeCollider::OnCreate() {
		auto col = AddComponent<CollisionSphere>();
		col->SetAfterCollision(AfterCollision::None);
		auto trans = GetComponent<Transform>();

		trans->SetPosition(m_Pos);
		trans->SetScale(Vec3(m_Explosion.m_Range));
	}
	void ExplodeCollider::OnUpdate() {
		auto player = m_Player.lock();
		if (player != nullptr) {
			float distance = GetDistance(player);
			Vec3 direction = GetDirection(player);
			if (distance < m_Explosion.m_Range) {
				float rate = distance / m_Explosion.m_Range;

				rate = max(m_MinRate, rate);
				rate = min(m_MaxRate, rate);

				float attenuation = (1.0f - rate);
				Vec3 reflectPower = direction.normalize() * attenuation * m_Explosion.m_Power;

				auto gravity = player->GetComponent<BCGravity>(false);
				if (gravity != nullptr) {

					gravity->Jump(reflectPower);
				}
			}
		}
		GetStage()->RemoveGameObject<ExplodeCollider>(GetThis<ExplodeCollider>());
	}
	
	void ExplodeCollider::OnCollisionEnter(shared_ptr<GameObject>& Other) {
		float distance = GetDistance(Other);
		Vec3 direction = GetDirection(Other);

		float rate = distance / m_Explosion.m_Range;
		rate = max(m_MinRate, rate);
		rate = min(m_MaxRate, rate);
		float attenuation = (1.0f - rate);

 		Vec3 reflectPower = direction.normalize() * attenuation * m_Explosion.m_Power;
		if (Other->FindTag(L"Floor")) {
			auto block = static_pointer_cast<FloorBlock>(Other);
			if (block != nullptr) {
				block->HitExplode(reflectPower.length() * 5.0f);
			}
		}
		auto gravity = Other->GetComponent<BCGravity>(false);
		if (gravity != nullptr) {
			gravity->Jump(reflectPower);
		}
	}

	float ExplodeCollider::GetDistance(shared_ptr<GameObject>& other) {

		Vec3 diff = GetDirection(other);

		return sqrtf(static_cast<float>(pow(diff.x, 2) + pow(diff.y, 2)));
	}
	Vec3 ExplodeCollider::GetDirection(shared_ptr<GameObject>& other) {
		Vec3 otherPos = other->GetComponent<Transform>()->GetWorldPosition() + Vec3(0,0.5f,0);
		Vec3 ExplodeCorePos = GetComponent<Transform>()->GetWorldPosition();

		return otherPos - ExplodeCorePos;
	}
}
//end basecross
