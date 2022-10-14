#include "ObjSet.h"

ObjSet::ObjSet(Camera* camera,
			   const std::string& dirPath,
			   const std::string& name,
			   bool smoothing)
{
	model = std::make_unique<ObjModel>(dirPath, name, 0U, smoothing);
	obj.resize(1u, Object3d(camera, model.get(), 0U));
}

void ObjSet::drawWithUpdate(Light* light)
{
	for (auto& i : obj)
	{
		i.drawWithUpdate(DX12Base::getInstance(), light);
	}
}