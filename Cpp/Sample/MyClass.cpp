module;
#include <string>
export module Sample;
import Opal;

using namespace Opal;

namespace Sample
{
	export class MyClass
	{
	public:
		Path DoWork()
		{
			return Path("./Folder/File.txt");
		}
	};
}
