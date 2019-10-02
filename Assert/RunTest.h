#pragma once

namespace SoupTest
{
	struct TestState
	{
		int FailCount;
		int PassCount;

		TestState& operator+=(const TestState& rhs)
		{
			FailCount += rhs.FailCount;
			PassCount += rhs.PassCount;
			return *this;
		}
	};

	template<typename T>
	TestState RunTest(
		std::string className,
		std::string testName,
		T test)
	{
		try
		{
			// std::cout << "Running: " << className << "::" << testName << std::endl;
			test();
			// std::cout << "PASS: " << className << "::" << testName << std::endl;
			return TestState{ 0, 1 };
		}
		catch (std::exception& ex)
		{
			std::cout << "FAIL: " << className << "::" << testName << std::endl;
			std::cout << typeid(ex).name() << std::endl;

			if (!std::string(ex.what()).empty())
			{
				std::cout << ex.what() << std::endl;
			}
		}
		catch (...)
		{
			std::cout << "FAIL: " << className << "::" << testName << std::endl;
			std::cout << "Unknown error..." << std::endl;
		}

		return TestState{ 1, 0 };
	}
}