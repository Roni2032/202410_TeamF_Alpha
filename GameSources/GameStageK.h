/*!
@file GameStage.h
@brief ゲームステージ
*/

#pragma once
#include "stdafx.h"

namespace basecross {

	//--------------------------------------------------------------------------------------
	//	ゲームステージクラス
	//--------------------------------------------------------------------------------------
	class GameStageK : public GameStage {
		float timer;
		//ビューの作成
		void CreateResource();
		void CreateViewLight();
	public:
		//構築と破棄
		GameStageK() :GameStage(L"StageK3.csv"), timer(0) {}
		virtual ~GameStageK() {}
		//初期化
		virtual void OnCreate()override;
		virtual void OnUpdate()override;
	};


}
//end basecross

