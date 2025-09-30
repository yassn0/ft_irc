#include <iostream>

int main(int ac, char **av)
{
	
	if (ac != 3)
	{
		std::cerr << "Error: Two arguments needed, <port> <password>" << std::endl;
		return 1;
	}
	
	try
	{
		//check argument
		/* code */
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	
	
}
