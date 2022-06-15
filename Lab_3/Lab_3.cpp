#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "enclave_u.h"
#define ENCLAVE_FILE _T ("enclave.signed.dll")
#define BUF_LEN 100 // Длина буфера обмена между анклавом и небезопасным приложением

/*
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
*/

int main() {

	char buffer[BUF_LEN] = { 0 };

	sgx_enclave_id_t eid;
	sgx_status_t ret = SGX_SUCCESS;
	sgx_launch_token_t token = { 0 };
	int updated = 0;

	ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
	if (ret != SGX_SUCCESS) {
		printf("Application error %#x, failed to create enclave.\n", ret);
		return -1;
	}

	while (true) {
		printf("Enter a number of PC to get MAC-address or enter -1 to exit a program: \t");
		int idx = -1;
		scanf_s("%d", &idx);
		if (idx < 0) {
			return 0;
		}
		foo(eid, buffer, BUF_LEN, idx);
		printf("%s \n******************************************\n\n", buffer);
	}

	if (SGX_SUCCESS != sgx_destroy_enclave(eid))
		return -1;
	return 0;
}