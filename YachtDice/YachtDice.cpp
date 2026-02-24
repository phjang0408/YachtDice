#include <iostream>
#include <array>
int main()
{
	std::array<int, 5> myArray = { 0 };
	for (
		std::array<int, 5>::iterator it = myArray.begin();
		it != myArray.end(); it++) {

		std::cout << *it << std::endl;
	}
}