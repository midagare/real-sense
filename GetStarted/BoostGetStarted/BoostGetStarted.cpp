#include <boost/circular_buffer.hpp>
#include <iostream>
#include <stdlib.h>
#include <time.h>

int main()
{
	// Create a circular buffer
	const int bufsize = 5;
	boost::circular_buffer<int> cb(bufsize);
	const int l = RAND_MAX / 3;
	srand(time(NULL));
	int i = 0;

	// push elements in cb
	while (i < bufsize)
		cb.push_back(++i);
	
	// push or pop elements randomly. Ends when cb is empty
	while (!cb.empty() && i < 1000)
	{
		if (rand() > l)

			cb.push_back(++i);
		else
		{
			std::cout << cb.size() << " : " << cb[0] << std::endl;
			cb.pop_front();
		}
	}
	std::cout << "cb is empty" << std::endl;

	system("pause");
	return 0;
}