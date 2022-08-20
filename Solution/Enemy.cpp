#include "Enemy.h"

using namespace DirectX;

Enemy::Enemy(Camera *camera,
			 ObjModel *model,
			 ObjModel *bulModel,
			 const DirectX::XMFLOAT3 &pos)
	:GameObj(camera, model, pos),
	bulModel(new ObjModel("Resources/sphere", "sphere", 0U, false)),
	camera(camera),
	phase(std::bind(&Enemy::phase_Approach, this)) {

	obj->rotation.x += 180.f;
}

void Enemy::shot(float vel, float bulScale) {
	// C++17から追加した要素の参照が返ってくるようになった
	std::unique_ptr<EnemyBullet> &i = bul.emplace_front(new EnemyBullet(camera, bulModel.get(), obj->position));

	
	XMFLOAT3 velF3{ 0,0,-2 };

	i->setVel(velF3);
	i->setScale(bulScale);
}

#pragma region phase

void Enemy::phase_Approach() {
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (shotFrame-- == 0U) {
		shot(2.f, 2.5f);
		shotFrame = shotFrameMax;
	}

	if (obj->position.z < 0.f) {
		vel = DirectX::XMFLOAT3(-1, 1, 0);
		phase = std::bind(&Enemy::phase_Leave, this);
	}
}

void Enemy::phase_Leave() {
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (std::abs(obj->position.x) > 50.f &&
		std::abs(obj->position.y) > 50.f) {
		alive = false;
	}
}

#pragma endregion phase

void Enemy::update(Light *light) {
	if (alive) {
		phase();
	}

	bul.remove_if([](std::unique_ptr<EnemyBullet> &i) {return !i->getAlive(); });

	for (auto &i : bul) {
		i->drawWithUpdate(light);
	}
}
