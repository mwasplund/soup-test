#include <string>
#include <iostream>

import Sample;

namespace Sample::UnitTests
{
	class MyClassUnitTests
	{
	public:
		void DoWork_Success()
		{
			auto uut = MyClass();
			auto result = uut.DoWork();
			if (result != 123)
				throw std::runtime_error("Does not match expected.");
		}
	};
}

int main()
{
	auto test = ::Sample::UnitTests::MyClassUnitTests();

	try
	{
		test.DoWork_Success();
		std::cout << "All Pass!" << std::endl;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return -1;
}