#include "Looper.h"

#include <DirectXMath.h>
#include "3D/Obj/Object3d.h"
#include "Sound/SoundBase.h"
#include "Input/Input.h"
#include "SceneManager.h"
using namespace DirectX;

Looper::Looper()
{
	Object3d::staticInit();

	// Soundのクラスより先に消えないようにここで生成しておく
	SoundBase::getInstange();

	pushImGuiCol();
}

void Looper::pushImGuiCol()
{
	constexpr XMFLOAT3 guiCol = XMFLOAT3(0.f, 1.f, 1.f);
	constexpr float guiColDarkVel = 0.25f;
	constexpr XMFLOAT3 guiColTitleBg = XMFLOAT3(guiCol.x * guiColDarkVel,
												guiCol.y * guiColDarkVel,
												guiCol.z * guiColDarkVel);
	constexpr float guiBgVel = 0.5f;
	constexpr XMFLOAT3 guiBg = XMFLOAT3(guiCol.x * guiBgVel,
										guiCol.y * guiBgVel,
										guiCol.z * guiBgVel);

	constexpr float guiAlpha = 0.125f;
	constexpr float guiTitleAlpha = 0.25f;

	// タイトルバーの色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TitleBg, ImVec4(guiColTitleBg.x,
															  guiColTitleBg.y,
															  guiColTitleBg.z,
															  guiTitleAlpha));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TitleBgActive, ImVec4(guiCol.x,
																	guiCol.y,
																	guiCol.z,
																	guiTitleAlpha));

	// 背景の色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, ImVec4(guiBg.x,
															   guiBg.y,
															   guiBg.z,
															   guiAlpha));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ScrollbarBg, ImVec4(guiBg.x,
																  guiBg.y,
																  guiBg.z,
																  guiAlpha));

	// その他の色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(guiCol.x,
														   guiCol.y,
														   guiCol.z,
														   1.f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Border, ImVec4(guiCol.x,
															 guiCol.y,
															 guiCol.z,
															 0.5f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ScrollbarGrab, ImVec4(guiCol.x,
																	guiCol.y,
																	guiCol.z,
																	0.5f));
}

void Looper::popImGuiCol()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

Looper::~Looper()
{
	popImGuiCol();
}

void Looper::loopUpdate()
{
	// 入力情報の更新
	Input::getInstance()->update();

	// --------------------
	// シーンマネージャーの更新
	// --------------------
	SceneManager::getInstange()->update();
}

void Looper::loopDraw()
{
	// --------------------
	// シーンマネージャーの描画
	// --------------------
	constexpr DirectX::XMFLOAT3 clearColor = { 0.f, 0.f, 0.f };	//黒色

	// ポストエフェクト内の描画
	PostEffect::getInstance()->startDrawScene(DX12Base::ins());
	SceneManager::getInstange()->drawObj3d();
	PostEffect::getInstance()->endDrawScene(DX12Base::ins());

	// 全体の描画
	DX12Base::ins()->startDraw();
	DX12Base::ins()->clearBuffer(clearColor);
	DX12Base::ins()->startImGui();

	PostEffect::getInstance()->draw(DX12Base::ins());

	SceneManager::getInstange()->drawFrontSprite();

	DX12Base::ins()->endImGui();
	DX12Base::ins()->endDraw();
}

bool Looper::loop()
{
	loopUpdate();
	loopDraw();

	return exitFlag;
}