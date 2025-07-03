#include "engine.h"

int main()
{
	DataStructure DS;
	if (DS.Construct(Screen_x ,Screen_y , 1, 1, false, true))
		DS.Start();
	return 0;
}

