#include "Selector.h"

NODE_RESULT Selector::run()
{
	for (Task& i : child)
	{
		if (i() == NODE_RESULT::SUCCESS)
		{
			return NODE_RESULT::SUCCESS;
		}
	}
	return NODE_RESULT::FAIL;
}