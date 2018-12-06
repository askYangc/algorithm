#include <stdio.h>
#include "CUnit/CUnit.h"
#include "CUnit/Automated.h"

#include "hello.h"

void utcase_add_case()
{
	int ret = 0;
	ret = add(10, 1);
	CU_ASSERT_EQUAL(ret, 11)
}

static CU_TestInfo ut_cases[] =
{
    {"case:first_case", utcase_add_case},
    // add more cases here
    CU_TEST_INFO_NULL,
};

int suite_init(void)
{
    return 0;
}

int suite_clean(void)
{
    return 0;
}

static CU_SuiteInfo ut_suites[] = 
{
    {"my_first_suite", suite_init, suite_clean, ut_cases},
    CU_SUITE_INFO_NULL,
};

int main() {
    int rc = 0;
    CU_ErrorCode err = CUE_SUCCESS;

    err = CU_initialize_registry();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to initialize registry, error %d", err);
        rc = 1;
        goto l_out;
    }

    err = CU_register_suites(ut_suites);
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to register suites, error %d, %s", err, CU_get_error_msg());
        rc = 1;
        goto l_clean_register;
    }

    CU_set_output_filename("cunit_sample");

    err = CU_list_tests_to_file();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to list tests to file, error %d, %s", err, CU_get_error_msg());
        rc = 1;
        goto l_clean_register;
    }

    CU_automated_run_tests();

l_clean_register:
    CU_cleanup_registry();

l_out:
    return rc;
}


