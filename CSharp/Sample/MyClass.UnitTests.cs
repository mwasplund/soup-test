using System;

namespace Sample.UnitTests
{
	public static class Program
	{
		public static void Main()
		{
			var test = new MyClassUnitTests();

			test.DoWork_Success();
		}
	}

	public class MyClassUnitTests
	{
		public void DoWork_Success()
		{
			var uut = new MyClass();
			var result = uut.DoWork();
		}
	}
}
