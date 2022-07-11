#include <Windows.h>

#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

#include <d3dx12.h>

#include <vector>
#include <string>
#include <fstream>
#include <DirectXMath.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <DirectXTex.h>
#include <wrl.h>

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

#include "Input.h"

#include "WinAPI.h"

#include "DX12Base.h"

#include "SpriteCommon.h"

#include "Sprite.h"

#include "DebugText.h"

#include <sstream>
#include <fstream>

#include "ObjModel.h"
#include "Object3d.h"
#include <memory>

#include "CollisionShape.h"

#include "Collision.h"
#include <iomanip>

#include "Sound.h"

#include "Time.h"

#include "System.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	System* sys = new System();

	sys->update();

	delete sys;
	sys = nullptr;

	return 0;
}