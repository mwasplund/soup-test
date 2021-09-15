import module Sample;

namespace Sample::UnitTests
{
	class MyClassUnitTests
	{
	public:
		void DoWork_Success()
		{
			auto uut = MyClass();
			auto result = uut.DoWork();
		}
	};

	void Main()
	{
		auto test = MyClassUnitTests();

		test.DoWork_Success();
	}
}
