#pragma once

namespace SoupTest
{
	export class Assert
	{
	public:
		static void Fail(std::string_view message)
		{
			auto errorMessage = std::stringstream();
			errorMessage << "Assert Failed: " << message;
			throw std::logic_error(std::move(errorMessage.str()));
		}

		static void IsTrue(bool value, std::string_view message)
		{
			if (!value)
			{
				Fail(message);
			}
		}

		static void IsFalse(bool value, std::string_view message)
		{
			if (value)
			{
				Fail(message);
			}
		}

		template<typename T>
		static void ThrowsRuntimeError(T test)
		{
			try
			{
				test();
				Fail("Test did not throw when expected.");
			}
			catch (std::runtime_error& exception)
			{
				// Saw the expected error
			}
		}

		template<typename T> struct is_shared_ptr : std::false_type {};
		template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

		template<typename T>
		static typename std::enable_if<std::is_pointer<T>::value || is_shared_ptr<T>::value>::type NotNull(T value, const std::string& message)
		{
			if (value == nullptr)
			{
				Fail(message);
			}
		}

		template<typename T>
		static typename std::enable_if<std::is_pointer<T>::value || is_shared_ptr<T>::value>::type AreEqual(
			T expected,
			T actual,
			const std::string& message)
		{
			if (expected == nullptr)
			{
				Fail("Expected was null, use IsNull instead.");
			}
			else if (actual == nullptr)
			{
				Fail("Actual was null, use IsNull if this is expected.");
			}
			else if (*expected != *actual)
			{
				Fail(message);
			}
		}

		template<typename T>
		static typename std::enable_if<!std::is_pointer<T>::value && !is_shared_ptr<T>::value>::type AreEqual(
			const T& expected,
			const T& actual,
			std::string_view message)
		{
			if (expected != actual)
			{
				auto errorExpected = std::stringstream();
				errorExpected << message;
				Fail(errorExpected.str());
			}
		}

		static void AreEqual(
			std::string_view expected,
			std::string_view actual,
			std::string_view message)
		{
			if (expected != actual)
			{
				auto errorExpected = std::stringstream();
				errorExpected << message <<
					" Expected<" << expected <<
					"> Actual<" << actual << ">";
				Fail(errorExpected.str());
			}
		}

		static void AreEqual(
			const std::string& expected,
			const std::string& actual,
			std::string_view message)
		{
			if (expected != actual)
			{
				auto errorExpected = std::stringstream();
				errorExpected << message <<
					" Expected<" << expected <<
					"> Actual<" << actual << ">";
				Fail(errorExpected.str());
			}
		}

		template<typename T>
		static void AreEqual(
			const std::vector<T>& expected,
			const std::vector<T>& actual,
			std::string_view message)
		{
			if (expected.size() != actual.size())
			{
				auto errorExpected = std::stringstream();
				errorExpected << message <<
					" Size does not match [" <<
					expected.size() << ", " <<
					actual.size() << "]";
				Fail(errorExpected.str());
			}
			else
			{
				for (size_t i = 0; i < expected.size(); i++)
				{
					Assert::AreEqual(expected[i], actual[i], message);
				}
			}
		}

		template<typename T>
		static typename std::enable_if<std::is_pointer<T>::value || is_shared_ptr<T>::value>::type AreNotEqual(
			T expected,
			T actual,
			std::string_view message)
		{
			if (expected == nullptr)
			{
				Fail("Expected was null, use IsNull instead.");
			}
			else if (actual == nullptr)
			{
				Fail("Actual was null, use IsNull if this is expected.");
			}
			else if (*expected == *actual)
			{
				Fail(message);
			}
		}

		template<typename T>
		static typename std::enable_if<!std::is_pointer<T>::value && !is_shared_ptr<T>::value>::type AreNotEqual(
			const T& expected,
			const T& actual,
			std::string_view message)
		{
			if (expected == actual)
			{
				Fail(message);
			}
		}
	};
}