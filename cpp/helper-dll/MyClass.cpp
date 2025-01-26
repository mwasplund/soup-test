export module Sample.Helper;

namespace Sample
{
	export class Helper
	{
	public:
		__declspec(dllexport) int DoWork()
		{
			return 123;
		}
	};
}
