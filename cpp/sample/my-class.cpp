export module Sample;
import Sample.Helper;

namespace Sample
{
	export class MyClass
	{
	public:
		int DoWork()
		{
			auto helper = Helper();
			return helper.DoWork();
		}
	};
}
