#include <check.h>

#include "mocks.h"

static void check_formatted_duration(
        long seconds,
        const char* expected_string)
{
    char *string = format_duration(seconds);

    fail_unless(!strcmp(string, expected_string),
            "\nFailed test for format_duration(\"%ld\"): \"%s\" expected, \"%s\" received",
            seconds,
            expected_string,
            string);
}

START_TEST (test_formatted_duration)
{
    check_formatted_duration(     0,         "0m:00s");
    check_formatted_duration(     1,         "0m:01s");
    check_formatted_duration(    59,         "0m:59s");
    check_formatted_duration(    60,         "1m:00s");
    check_formatted_duration(  3599,        "59m:59s");
    check_formatted_duration(  3600,     "1h:00m:00s");
    check_formatted_duration( 86399,    "23h:59m:59s");
    check_formatted_duration( 86400, "1d:00h:00m:00s");
    check_formatted_duration(172799, "1d:23h:59m:59s");
} END_TEST

Suite *test_suite(void)
{
    Suite *s = suite_create("UnNetHack hacklib");
    TCase *tc_core = tcase_create("functions");

    suite_add_tcase (s, tc_core);

    tcase_add_test(tc_core, test_formatted_duration);

    return s;
}

int main(int argc, char *argv[]) {
    int nf;
    Suite *s = test_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_xml(sr, "test_hacklib.xml");
    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
