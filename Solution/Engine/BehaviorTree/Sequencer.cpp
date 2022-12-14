#include "Sequencer.h"

NODE_RESULT Sequencer::run()
{
	for (Task& i : child)
	{
		if (i() == NODE_RESULT::FAIL)
		{
			return NODE_RESULT::FAIL;
		}
	}
	return NODE_RESULT::SUCCESS;
}
