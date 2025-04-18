/*!
@file Character.cpp
@brief キャラクターなど実体
*/

#include "stdafx.h"
#include "Project.h"

namespace basecross{
	void BackGround::OnCreate() {
		auto draw = AddComponent<PNTStaticDraw>();
		draw->SetMeshResource(L"DEFAULT_SQUARE");
		draw->SetTextureResource(L"BACKGROUND_TEX");
		auto trans = GetComponent<Transform>();
		trans->SetPosition(m_Pos);
		trans->SetScale(Vec3(15.0f,m_Scale.y,2));

		SetDrawLayer(0);
	}
	void BackGround::OnUpdate() {

	}

	void BackGround::SetActive(bool flag) {
	}

	void BackGroundManager::OnCreate(){
		m_Camera = OnGetDrawCamera();
		auto camera = m_Camera.lock();

		float cameraHight = camera->GetHeight();
		Vec3 at = camera->GetAt() - Vec3(7.0f, 0, 0);
		if (camera != nullptr) {
			for (int j = 0; j < 2; j++) {
				for (int i = -1; i < 3; i++) {
					
					m_BackGrounds.push_back(GetStage()->AddGameObject<BackGround>(at + Vec3(15.0f * j, 0, 10) + Vec3(0, m_DrawScaleY, 0) * static_cast<float>(i), Vec3(m_DrawScaleY)));
				}
			}
		}
	}
	void BackGroundManager::OnUpdate() {
		auto camera = m_Camera.lock();
		if (camera == nullptr) return;
		Vec3 at = camera->GetAt();
		auto stage = GetTypeStage<GameStage>();
		int stageSizeY = stage->GetMapSize().y;
		
		int cutNum = stageSizeY / m_DrawNum;
		for (auto& backGround : m_BackGrounds) {
			auto trans = backGround->GetComponent<Transform>();
			Vec3 pos = trans->GetPosition();
			float diff = pos.y - at.y;
			float updateDistance = m_CameraDrawYBlockNum / 2 + m_DrawScaleY;
			if (diff < -updateDistance) {
				trans->SetPosition(pos.x, pos.y + m_DrawScaleY * m_DrawNum, pos.z);
			}
			else if (diff > updateDistance) {
				trans->SetPosition(pos.x, pos.y - m_DrawScaleY * m_DrawNum, pos.z);
			}
		}
	}
}
//end basecross
