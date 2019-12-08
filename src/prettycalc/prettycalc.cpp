// prettycalc.cpp : Файл представлен исключительно в ознакомительных
// целях и не является самостоятельной единицей.

#include "stdafx.h"

int main()
{
	int array[5][2]; // только пять первых строк или меньше
	std::cout << "Pretty Calc" << std::endl;
	int n = 5;
	for (int i = 0; i < n; i++)
	{
		std::cin >> array[i][0];
		std::cin >> array[i][1];

		if (array[i][0] <= 0 || array[i][1] <= 0)
		{
			if ((array[i][0] < 0 || array[i][1] < 0) && i > 0)
			{
				n = i;
				break;
			}
			std::cout << "Error" << std::endl;
			return 0;
		}
	}
	int res = 0;
	for (int i = 0; i< n; i++)
	{
		res += array[i][0] * array[i][1];

	}
	std::cout << "Result: " << res << std::endl;
}