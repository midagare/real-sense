#include <iostream>
#include <thread>
#include <sstream>
#include <time.h>
#include <chrono>

void thread_function(int n)
{
	srand(time(NULL) + n);
	int r;
	std::stringstream ss;

	for (int i = 0; i < 10; i++) {
		r = rand() % 1000;
		ss.str("");
		ss << "Thread " << n << " function Executing:" << i << "; time:" << r << std::endl << std::endl;
		std::cout << ss.str();
		std::this_thread::sleep_for(std::chrono::milliseconds(r));
	}
	ss.str("");
	ss << "function Ending: " << n << std::endl << std::endl;
	std::cout << ss.str();
}

int main()
{
	std::thread t[3];
	t[0] = std::thread(thread_function, 1);
	t[1] = std::thread(thread_function, 2);
	t[2] = std::thread(thread_function, 3);

	for (int i = 0; i < 10; i++) {
		std::stringstream ss;
		ss << "Display From MainThread:" << i << std::endl;
		std::cout << ss.str();
	}
	t[0].join();
	t[1].join();
	t[2].join();
	std::cout << "Exit of Main function" << std::endl;
	system("pause");
	return 0;
}