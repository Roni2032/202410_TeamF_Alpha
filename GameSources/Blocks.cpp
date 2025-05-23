/*!
@file FloorBlock.cpp
@brief 地面の実体
*/

#include "stdafx.h"
#include "Project.h"

namespace basecross{

	void InstanceBlock::OnCreate() {
		m_Draw = AddComponent<PNTStaticInstanceDraw>();
		m_Draw->SetMeshResource(L"DEFAULT_CUBE");
		m_Draw->SetTextureResource(m_TexKey);

		m_Maps.reserve(m_SizeY);
		for (int i = 0; i < m_SizeY; i++) {
			m_Maps.push_back({});
		}
	}
	void InstanceBlock::AddBlock(int y, int cell) {
		if (y >= m_SizeY) return;
		m_Maps[y].push_back(cell);
	}
	void InstanceBlock::DrawMap(vector<vector<BlockData>>& map, Vec2 drawSize, Vec3 leftTop) {
		auto stage = GetTypeStage<GameStage>();
		m_Camera = OnGetDrawCamera();
		Vec3 at = stage->GetMapIndex(m_Camera->GetAt());

		float drawIndexMinY = at.y - drawSize.y;
		float drawIndexMaxY = at.y + drawSize.y;

		drawIndexMinY = max(0, drawIndexMinY);
		drawIndexMaxY = min(map.size() - 1, drawIndexMaxY);
		/*if (drawIndexMinY < 0) {
			drawIndexMinY = 0;
		}
		if (drawIndexMaxY >= map.size()) {
			drawIndexMaxY = map.size() - 1;
		}*/
		
		drawSize.x = map[0].size();

		m_Draw->ClearMatrixVec();
		for (int y = drawIndexMinY; y <= drawIndexMaxY; y++) {
			for (int x = 0; x < drawSize.x; x++) {
				if (map[y][x].GetID() != BlockTypes::UNBREAK) continue;

				Mat4x4 matrix;
				matrix.translation(stage->GetWorldPosition(Vec2(x, y)));

				m_Draw->AddMatrix(matrix);

				if (map[y][x].GetIsLoaded()) continue;
				map[y][x].SetIsLoaded(true);

				if (CheckExposedBlock(map, Vec2(x, y))) {
					auto obj = GetStage()->AddGameObject<Block>(L"", stage->GetWorldPosition(Vec2(x,y)), Vec3(1.0f));
					GetTypeStage<GameStage>()->RegisterBlock(Vec2(x, y), obj);
				}
			}
		}

		for (int y = 0; y < map.size(); y++) {
			for (int x = 0; x < map[y].size(); x++) {
				if (map[y][x].GetID() != BlockTypes::UNBREAK) continue;
				if (!map[y][x].GetIsLoaded()) continue;

				if (y < drawIndexMinY || y > drawIndexMaxY) {
					stage->RemoveGameObject<GameObject>(map[y][x].GetBlock());
					map[y][x].SetIsLoaded(false);
				}
			}
		}
	}
	bool InstanceBlock::CheckExposedBlock(vector<vector<BlockData>>& map,Vec2 center) {
		Vec2 aroundMap[] = {
					Vec2(center.x + 1,center.y),
					Vec2(center.x - 1,center.y),
					Vec2(center.x,center.y + 1),
					Vec2(center.x,center.y - 1)
		};
		for (Vec2 around : aroundMap) {
			if (around.x < 0 || around.x >= map[center.y].size()) continue;
			if (around.y < 0 || around.y >= map.size()) continue;

			if (map[static_cast<int>(around.y)][static_cast<int>(around.x)].GetID() != BlockTypes::UNBREAK) {
				return true;
			}
		}
		return false;
	}

	vector<weak_ptr<Transform>> Block::CollisionObjects = {};

	void Block::OnCreate() {
		if (m_TexKey == L"" || m_TexKey == L"null") {
			
		}
		else {
			auto drawComp = AddComponent<BcPNTStaticDraw>();
			drawComp->SetMeshResource(L"DEFAULT_CUBE");
			drawComp->SetTextureResource(m_TexKey);
			drawComp->SetDiffuse(Col4(1, 1, 1, 1));
			//モデルの設定
		}

		auto col = AddComponent<CollisionObb>();
		col->SetFixed(true);
		
		AddTag(L"Stage");

		auto trans = GetComponent<Transform>();
		trans->SetPosition(m_Pos);
		trans->SetScale(m_Scale);
		trans->SetRotation(m_Rot);

		Start();
	}

	void Block::OnUpdate() {
		Update();
		GetComponent<Transform>()->SetScale(1, 1, 1);
	}
	void Block::OnCollisionEnter(shared_ptr<GameObject>& Other) {
		auto col = Other->GetComponent<Collision>(false);
		if (col != nullptr) {
			if (col->GetAfterCollision() != AfterCollision::Auto) return;

		}
	}
	void Block::OnCollisionExcute(shared_ptr<GameObject>& Other) {
		auto col = Other->GetComponent<Collision>(false);
		if (col != nullptr) {
			if (col->GetAfterCollision() != AfterCollision::Auto) return;

		}
	}
		
	void FloorBlock::Start() {
		AddTag(L"Floor");
		CheckDurability();
	}
	void FloorBlock::Update() {

	}
	void FloorBlock::HitExplode(int damage) {
		m_Durability -= damage;

		CheckDurability();
		if (m_Durability <= 0) {
			GetTypeStage<GameStage>()->DestroyBlock(GetComponent<Transform>()->GetPosition(), GetThis<GameObject>());
		}
	}

	void FloorBlock::CheckDurability() {
		auto drawComp = GetComponent<BcPNTStaticDraw>();
		if (m_Durability <= 25) {
			drawComp->SetTextureResource(L"TEST25_TEX");
		}
		else if (m_Durability <= 50) {
			drawComp->SetTextureResource(L"TEST50_TEX");
		}
		else if(m_Durability <= 75){
			drawComp->SetTextureResource(L"TEST75_TEX");
		}
		else {
			drawComp->SetTextureResource(L"TEST100_TEX");
		}
	}

	void MoveBlock::Start() {
		Block::Start();

		m_Trans = GetComponent<Transform>();

		m_TargetPos = m_MoveStartPos;
	}

	void MoveBlock::Update() {
		Block::Update();

		float elapsed = App::GetApp()->GetElapsedTime();
		Vec3 pos = m_Trans->GetPosition();
		Vec3 velocity = m_TargetPos - pos;
		velocity = velocity.normalize();

		pos += m_MoveSpeed * velocity * elapsed;

		if ((m_TargetPos - pos).length() < 0.05f) {
			pos = m_TargetPos;

			m_TargetPos = m_TargetPos == m_MoveStartPos ? m_MoveEndPos : m_MoveStartPos;
		}
		m_Trans->SetPosition(pos);
	}

	void MoveBlock::OnCollisionEnter(shared_ptr<GameObject>& Other) {
		Block::OnCollisionEnter(Other);
		if (Other->FindTag(L"Player")) {
			Other->GetComponent<Transform>()->SetParent(GetThis<GameObject>());
		}
	}
	void MoveBlock::OnCollisionExit(shared_ptr<GameObject>& Other) {
		if (Other->FindTag(L"Player")) {
			Other->GetComponent<Transform>()->SetParent(nullptr);
		}
	}

	void ExplodeBlock::OnCollisionEnter(shared_ptr<GameObject>& Other) {
		if (Other->FindTag(L"Player")) {
			auto stage = GetTypeStage<GameStage>();
			stage->AddGameObject<ExplodeCollider>(GetComponent<Transform>()->GetWorldPosition() - Vec3(0,0.5f,0), Explosion(m_Power, m_Range), Other);

			auto otherCol = Other->GetComponent<BCCollisionObb>();
			Vec3 diff = Vec3();
			if (otherCol != nullptr) {
				auto data = otherCol->GetCollisionData(GetThis<GameObject>());
				switch (data.hitDir) {
				case HitDir::Up:
					diff = Vec3(0, 0.5f, 0);
					break;
				case HitDir::Down:
					diff = Vec3(0, -0.5f, 0);
					break;
				case HitDir::Right:
					diff = Vec3(0.5f, 0, 0);
					break;
				case HitDir::Left:
					diff = Vec3(-0.5f, 0, 0);
					break;

				}
			}
			stage->PlayParticle<ExplodeParticle>(L"EXPLODE_PCL", GetComponent<Transform>()->GetWorldPosition() + diff);

			SoundManager::Instance().PlaySE(L"BOMB_SD", 0.1f);
			//プレイヤーをスタン状態にする
			auto player = static_pointer_cast<Player>(Other);
			player->PlayerStun(1.0f);
		}
	}

	void ConditionalMoveBlock::Start() {
		MoveBlock::Start();
	}

	void ConditionalMoveBlock::Update() {
		bool isFunc = m_Func(GetTypeStage<GameStage>());
		if (isFunc) {
			MoveBlock::Update();
		}

		float elapsed = App::GetApp()->GetElapsedTime();
		
	}
}
//end basecross
