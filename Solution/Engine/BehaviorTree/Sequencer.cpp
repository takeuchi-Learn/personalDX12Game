#include "Sequencer.h"

NODE_RESULT Sequencer::mainProc()
{
	for (Task& i : child)
	{
		if (i.run() == NODE_RESULT::FAIL)
		{
			return NODE_RESULT::FAIL;
		}
	}
	return NODE_RESULT::SUCCESS;
}
