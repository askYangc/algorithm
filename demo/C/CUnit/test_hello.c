#include <stdio.h>
#include "CUnit/CUnit.h"
#include "CUnit/Automated.h"
#include <CUnit/Basic.h>
#include <CUnit/Console.h>


#include "hello.h"

void utcase_add_case()
{
	int ret = 0;
	ret = add(10, 1);
	printf("ret:%d\n", ret);
	CU_ASSERT_EQUAL_FATAL(ret, 10);
	printf("utcase_add_case\n");
}


void utcase_del_case()
{
	int ret = 0;
	ret = del(10, 1);
	CU_ASSERT_EQUAL(ret, 9);
	printf("utcase_del_case\n");
}


static CU_TestInfo ut_cases[] =
{
    {"case:first_case", utcase_add_case},
    {"case:second_case", utcase_del_case},
    // add more cases here
    CU_TEST_INFO_NULL,
};

int suite_init(void)
{
	printf("suite_init\n");
    return 0;
}

int suite_clean(void)
{
	printf("suite_clean\n");
    return 0;
}

static CU_SuiteInfo ut_suites[] = 
{
    {"my_first_suite", suite_init, suite_clean, ut_cases},
    CU_SUITE_INFO_NULL,
};


void Automated_Mode()
{
	int err = CUE_SUCCESS;
	CU_set_output_filename("cunit_sample");

    err = CU_list_tests_to_file();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to list tests to file, error %d, %s", err, CU_get_error_msg());
        return ;
    }

    CU_automated_run_tests();
}

void Basic_Mode()
{
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
}

void Console_Mode()
{
	CU_console_run_tests();
}

int main() {
    int rc = 0;
    CU_ErrorCode err = CUE_SUCCESS;

    err = CU_initialize_registry();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to initialize registry, error %d", err);
        return 1;
    }

    err = CU_register_suites(ut_suites);
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to register suites, error %d, %s", err, CU_get_error_msg());
        rc = 1;
        goto l_clean_register;
    }

	//**** Automated Mode *****************
    Automated_Mode();
	//***** Basic Mode ********************
    //Basic_Mode();
	//*****Console Mode *******************
	//Console_Mode();

l_clean_register:
    CU_cleanup_registry();
	return rc;
}







