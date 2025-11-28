#include <string.h>

static int is_delimiter(char c, char *delims)
{
	for (int i = 0; i < strlen(delims); i++)
		if (c == delims[i])
			return 1;
	return 0;
}

int get_option(char **buffer, char *option, char *delims)
{
	int start, end, length;
	char *curr = *buffer;

	for (start = 0;
	     is_delimiter(curr[start], delims) && start < strlen(curr); start++)
		;
	for (end = start;
	     !is_delimiter(curr[end], delims) && end < strlen(curr); end++)
		;
	length = end - start + 1;
	option[length] = '\0';
	strncpy(option, curr + start, end - start);
	(*buffer) += end;
	return 0;
}
