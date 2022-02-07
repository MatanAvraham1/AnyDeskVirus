
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define ANYDESK_CODE_ERROR "no-code__"

int main() {
	char* code = calloc(sizeof(char), 10);
	strncpy_s(code, 10, ANYDESK_CODE_ERROR, 10);
	printf("%d", strcmp(code, ANYDESK_CODE_ERROR));
	return 0;
}