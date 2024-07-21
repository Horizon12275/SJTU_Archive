#include "waiting_once_api.hpp"

void waiting_once::call_once_waiting(init_function f)
{
	// TODO: implement this
	std::call_once(m_flag, f);
}
