#include <string>

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
			if (result != Opal::Path("./Folder/File.txt"))
				throw "Does not match expected.";
		}
	};
}

int main()
{
	auto test = ::Sample::UnitTests::MyClassUnitTests();

	test.DoWork_Success();

	return 0;
}