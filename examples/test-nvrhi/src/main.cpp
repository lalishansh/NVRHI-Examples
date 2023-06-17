#include <fmt/core.h>

#include "version.h"

void say_hello();
int main() {
	fmt::print ("Hello, World! from VERSION {}\n", NVRHI_EXAMPLES_VERSION_STRING);
	say_hello();

	getchar();
	return 0;
}
