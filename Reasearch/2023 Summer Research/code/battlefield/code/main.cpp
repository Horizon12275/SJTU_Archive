#include "engine.h"
#include "field.h"
#include "units.h"

int main()
{
	int M = 0, N = 0, NT = 0, NU = 0;
	std::cin >> M >> N >> NT >> NU;

	field field_input(M, N);

	for (int i = 0; i < NT; i++) {
		int x = 0, y = 0;
		char t = '\0';

		std::cin >> x >> y >> t;
		x++, y++;

		field_input.set_terrains(x , y , t);
	}

	std::vector<units> units_input;
	for (int i = 0; i < NU; i++) {
		int x = 0, y = 0;
		char p;
		std::string t;

		std::cin >> x >> y >> p >> t;
		x++, y++;

		//if (p == 'B') transform(t.begin(), t.end(), t.begin(), tolower);

		units utmp(x, y, t, p);
		units_input.push_back(utmp);
		if (x >= 1 && x <= M && y >= 1 && y <= N && field_input.terrain(x, y) != 'M' && field_input.terrain(x, y) != 'W' && field_input.terrain(x, y) != 'A')
			field_input.set_units(x, y, &units_input[i]);
		else units_input[i].set_dead();

		//std::cout << std::endl;
		//std::cout << units_input[i].type() << " " << units_input[i].player() << " " << units_input[i].mob() << std::endl;
		//std::cout << field_input.unit(x, y) << " " << field_input.unit(x, y)->type() << std::endl;

	}

	Battlefield BF(field_input, units_input);
	if (BF.Construct(block_size * (N + 2) , block_size * (M + 2) , 2, 2, false, false))
		BF.Start();
	return 0;
}

