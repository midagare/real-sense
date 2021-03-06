// CppGetStarted.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <locale>

int main(int argc, char** argv)
{
	std::time_t t = std::time(nullptr);
	char mbstr[100];
	std::strftime(mbstr, sizeof(mbstr), "%Y%m%d_%H%M%S", std::localtime(&t));
	std::cout << mbstr << std::endl;

	if (argc > 3 && !strcmp(argv[1], "add")) {
		int a = std::atoi(argv[2]);
		int b = std::atoi(argv[3]);
		std::cout << "add " << a << " to " << b << " : " << PCH_H::add(a,b) << std::endl;
	} else {
		std::cout << "Hello world!!!\n\n"
			<< "usage: CppGetStarted add [int a] [int b] "
			<< std::endl;
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu


// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
