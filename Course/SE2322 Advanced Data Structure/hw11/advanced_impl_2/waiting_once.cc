#include "waiting_once_api.hpp"

void waiting_once::call_once_waiting(init_function f)
{
	// TODO: implement this
	if (!call_once)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!call_once)
		{
			f();
			call_once = true;
		}
	}
}
