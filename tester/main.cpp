#include <iostream>
#include <libc.h>

int main(int argc, char **argv)
{
	pid_t process_id;
	process_id = ::fork();
	if (process_id == 0)
	test_chunked_content();
}
