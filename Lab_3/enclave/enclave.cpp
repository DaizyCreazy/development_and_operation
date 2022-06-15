#include "enclave_t.h"
#include "sgx_trts.h"
#include <string.h>

const char table[6][18] = {
	"0A-00-27-BC-30-15",
	"5F-73-FE-DA-CE-AB",
	"E5-8B-86-7D-A1-06",
	"0D-62-F0-D4-A2-32",
	"2B-A9-FF-A9-42-C1",
	"CF-4C-35-77-5D-8F"
};
void foo(char* buf, size_t len, size_t idx) {
	if (idx < 6) {
		const char* data_ptr = data_ptr = table[idx];
		memcpy(buf, data_ptr, strlen(data_ptr + 1));
	}
	else {
		memset(buf, 0, strlen(table[0]));
	}
	return;
}
