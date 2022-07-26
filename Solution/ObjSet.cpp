#include "ObjSet.h"

ObjSet::ObjSet(Camera *camera,
			   const std::string &dirPath,
			   const std::string &name,
			   bool smoothing) {
	model = std::make_unique<ObjModel>(dirPath, name, 0U, smoothing);
	obj = std::make_unique<Object3d>(DX12Base::getInstance()->getDev(), camera, model.get(), 0U);
}

void ObjSet::drawWithUpdate(Light *light) {
	obj->drawWithUpdate(DX12Base::getInstance(), light);
}
