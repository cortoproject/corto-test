/* $CORTO_GENERATED
 *
 * test.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <corto/test/test.h>

/* $header() */
corto_threadKey test_suiteKey;
/* $end */

corto_bool _test_assert(
    corto_bool condition,
    corto_string str_condition,
    corto_uint32 __line)
{
/* $begin(corto/test/assert) */
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    if (!condition) {
        char *assertMsg = NULL;
        corto_asprintf(&assertMsg, "%d: assert(%s)", __line, str_condition);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
    }

    return !condition;
/* $end */
}

corto_bool _test_assertEqual(
    corto_any a,
    corto_any b,
    corto_string str_a,
    corto_string str_b,
    corto_uint32 __line)
{
/* $begin(corto/test/assertEqual) */
    corto_equalityKind eq;
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    eq = corto_type_compare(a, b);
    if (eq != CORTO_EQ) {
        corto_asprintf(&assertMsg, "%d: assert(%s == %s)", __line, str_a, str_b);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
    }

    return (eq != CORTO_EQ);
/* $end */
}

corto_bool _test_assertint(
    corto_uint64 i1,
    corto_uint64 i2,
    corto_string str_i1,
    corto_string str_i2,
    corto_uint32 __line)
{
/* $begin(corto/test/assertint) */
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    if (i1 != i2) {
        char *si1, *si2;
        if (isdigit(*str_i1) || (*str_i1 == '-')) {
            si1 = strdup(str_i1);
        } else {
            corto_asprintf(&si1, "%s (%lld)", str_i1, i1);
        }
        if (isdigit(*str_i2) || (*str_i2 == '-')) {
            si2 = strdup(str_i2);
        } else {
            corto_asprintf(&si2, "%s (%lld)", str_i2, i2);
        }
        corto_asprintf(&assertMsg, "%d: %s != %s", __line, si1, si2);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
        corto_dealloc(si1);
        corto_dealloc(si2);
    }

    return i1 == i2;
/* $end */
}

corto_bool _test_assertstr(
    corto_string s1,
    corto_string s2,
    corto_string str_s1,
    corto_string str_s2,
    corto_uint32 __line)
{
/* $begin(corto/test/assertstr) */
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    if (strcmp(s1, s2)) {
        corto_asprintf(&assertMsg, "%d: %s (\"%s\") != %s (\"%s\")", __line, str_s1, s1, str_s2, s2);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
    }

    return strcmp(s1, s2);
/* $end */
}

corto_void _test_fail(
    corto_string err)
{
/* $begin(corto/test/fail) */
    int i;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    if (this->result.success) {
        this->result.errmsg = corto_strdup(err);
        this->result.success = FALSE;
    }

    for (i = 0; i < 255; i++) {
        fprintf(stderr, "\b");
    }

    corto_error("%sFAIL%s: %s%s:%s",
        CORTO_RED,
        CORTO_NORMAL,
        corto_fullpath(NULL, this->testcase),
        CORTO_NORMAL,
        this->result.errmsg ? this->result.errmsg : "");

    corto_string lasterr = corto_lasterr();
    if (lasterr) {
        fprintf(stderr, "   %sdetails%s: %s\n", CORTO_BOLD, CORTO_NORMAL, lasterr);
    }

    /* Run teardown before exit */
    test_SuiteData_teardown(this);

    exit(-1);
/* $end */
}

corto_void _test_setTimeout(
    corto_uint32 sec)
{
/* $begin(corto/test/setTimeout) */
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::setTimeout called but no testsuite is running!");
        abort();
    }

    this->timeout = sec;

/* $end */
}

int testMain(int argc, char* argv[]) {
/* $begin(main) */
    CORTO_UNUSED(argc);
    CORTO_UNUSED(argv);
    if (corto_threadTlsKey(&test_suiteKey, NULL)) {
        return -1;
    }
    return 0;
/* $end */
}
