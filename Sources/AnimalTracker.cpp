//Animal Tracker copyright notice:
//All rights reserved to Anna Zamansky, Dept. of Information System, University of Haifa.
//Any use of this software is only allowed with permission from Anna Zamansky.
//----------------------------------------------------------------------------------------------------------------------------------------------------
//Based on A. Kaifi and H. Althobaiti's modification of code written by Kyle Hounslow in 2013 and copyrighted as follows:
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
//to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "AnimalBodyReader.h"

int main(int argc, char* argv[])
{
	AnimalBodyReader animalBodyReader;
	printf("Animal Tracker (dept. of IS, University of Haifa), ver. 1.1 (2017) is running\n");

	if (animalBodyReader.ReadConfig("AnimalBodyReaderConfig.xml") != NO_ERROR) {
		printf(_FUNC_ "Failed to read configuration, exiting");
		getchar();
		return 1;
	}
	printf("Configuration was successfully read\n");

	if (animalBodyReader.StartVideo() != NO_ERROR) {
		printf(_FUNC_ "Failed to obtain video\n");
		getchar();
		return 2;
	}
	printf("Video capturing was successfully started\n");

	waitKey(500);
	while (true) {
		int ret = animalBodyReader.HandleVideoFrame();
		if (ret == 12345) {
			printf("Terminated by user, exiting\n");
			return 0;
		}
		if (ret != NO_ERROR) {
			if (ret != 2) {
				printf(_FUNC_ "Video frame handling failed, exiting\n");
			}
			getchar();
			return 3;
		}
	}

	return 0;
}
