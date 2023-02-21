#include "Selector.h"

NODE_RESULT Selector::mainProc()
{
	for (Task& i : child)
	{
		if (i.run() == NODE_RESULT::SUCCESS)
		{
			return NODE_RESULT::SUCCESS;
		}
	}
	return NODE_RESULT::FAIL;
}
