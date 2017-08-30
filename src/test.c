/* This is a managed file. Do not delete this comment. */

#include <corto/test/test.h>

corto_threadKey test_suiteKey;

void test_erase(void) {
    int i;
    for (i = 0; i < 255; i++) {
        fprintf(stderr, "\b");
    }
}

corto_string test_id(corto_id buffer, corto_object testcase) {
    corto_string result = NULL;
    corto_object testroot = corto_lookup(root_o, "test");
    corto_assert(testroot != NULL, "testroot disappeared?");
    result = corto_path(buffer, testroot, testcase, "/");
    *(char*)strchr(result, '(') = '\0';
    corto_release(testroot);
    return result;
}

corto_string test_command(corto_id buffer, corto_string lib, corto_object testcase) {
    corto_id testcaseId, library;
    test_id(testcaseId, testcase);
    sprintf(library, "%s/%s", corto_cwd(), lib);
    corto_cleanpath(library, library);
    sprintf(buffer, "corto -l %s %s", library, testcaseId);
    return buffer;
}


bool test_assert(
    bool condition,
    corto_string str_condition,
    uint32_t __line)
{
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
}

bool test_assertEqual(
    corto_any a,
    corto_any b,
    corto_string str_a,
    corto_string str_b,
    uint32_t __line)
{
    corto_equalityKind eq;
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    eq = corto_ptr_compare(a.value, a.type, b.value);
    if (eq != CORTO_EQ) {
        corto_asprintf(&assertMsg, "%d: assert(%s == %s)", __line, str_a, str_b);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
    }

    return (eq != CORTO_EQ);
}

bool test_assertflt(
    double f1,
    double f2,
    corto_string str_f1,
    corto_string str_f2,
    uint32_t __line)
{
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    if (f1 != f2) {
        char *si1, *si2;
        if (isdigit(*str_f1) || (*str_f1 == '-')) {
            si1 = strdup(str_f1);
        } else {
            corto_asprintf(&si1, "%s (%f)", str_f1, f1);
        }
        if (isdigit(*str_f2) || (*str_f2 == '-')) {
            si2 = strdup(str_f2);
        } else {
            corto_asprintf(&si2, "%s (%f)", str_f2, f2);
        }
        corto_asprintf(&assertMsg, "%d: %s != %s", __line, si1, si2);
        test_fail(assertMsg);
        corto_dealloc(assertMsg);
        corto_dealloc(si1);
        corto_dealloc(si2);
    }

    return f1 == f2;
}

bool test_assertint(
    uint64_t i1,
    uint64_t i2,
    corto_string str_i1,
    corto_string str_i2,
    uint32_t __line)
{
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
}

bool test_assertstr(
    corto_string s1,
    corto_string s2,
    corto_string str_s1,
    corto_string str_s2,
    uint32_t __line)
{
    char *assertMsg = NULL;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }
    this->assertCount++;

    if (s1 || s2) {
        if ((!s1 && s2) || (s1 && !s2) || strcmp(s1, s2)) {
            if ((s1 && strchr(s1, '\n')) || (s2 && strchr(s2, '\n'))) {
                corto_asprintf(&assertMsg, "%d: \n%s:\n%s\n%s:\n%s\n", __line, str_s1, s1, str_s2, s2);
            } else {
                corto_asprintf(&assertMsg, "%d: %s (\"%s\") != %s (\"%s\")", __line, str_s1, s1, str_s2, s2);
            }
            test_fail(assertMsg);
            corto_dealloc(assertMsg);
            return FALSE;
        }
    }

    return TRUE;
}

void test_empty(void)
{
    int i;
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::fail called but no testsuite is running!");
        abort();
    }

    for (i = 0; i < 255; i++) {
        fprintf(stderr, "\b");
    }

    printf("%sEMPTY%s: %s%s%s: missing implementation              \n",
        CORTO_YELLOW,
        CORTO_NORMAL,
        this->tearingDown ? corto_idof(corto_parentof(this->testcase)) : "",
        this->tearingDown ? "/teardown" : test_id(NULL, this->testcase),
        CORTO_NORMAL);

    /* Run teardown before exit, prevent infinite recursion if assert is called
     * in teardown. */
    if (!this->tearingDown) {
        this->tearingDown = TRUE;
        test_SuiteData_teardown(this);
    }

    exit(1);
}

void test_fail(
    corto_string err)
{
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

    for (i = 0; i < 80; i++) {
        fprintf(stderr, " ");
    }

    for (i = 0; i < 80; i++) {
        fprintf(stderr, "\b");
    }

    corto_string lasterr = corto_lasterr() ? corto_strdup(corto_lasterr()) : NULL;

    fprintf(stderr, "%sFAIL%s: %s%s%s:%s\n",
        CORTO_RED,
        CORTO_NORMAL,
        this->tearingDown ? corto_idof(corto_parentof(this->testcase)) : "",
        this->tearingDown ? "/teardown" : test_id(NULL, this->testcase),
        CORTO_NORMAL,
        err ? err : "");

    if (lasterr) {
        fprintf(stderr, "   %sdetails%s: %s\n", CORTO_BOLD, CORTO_NORMAL, lasterr);
        corto_dealloc(corto_lasterr());
    }

    /* Run teardown before exit, prevent infinite recursion if assert is called
     * in teardown. */
    if (!this->tearingDown) {
        this->tearingDown = TRUE;
        test_SuiteData_teardown(this);
    }

    exit(-1);
}

bool test_runslow(void)
{
    char *runslow = getenv("CORTO_TEST_RUNSLOW");
    if (runslow && !strcmp("CORTO_TEST_RUNSLOW", "TRUE")) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void test_setTimeout(
    corto_time *t)
{
    test_SuiteData this = corto_threadTlsGet(test_suiteKey);
    if (!this) {
        corto_error("test: test::setTimeout called but no testsuite is running!");
        abort();
    }

    corto_lock(this);
    this->timeout = *t;

    /* When running in CI, give tests extra time */
    if (corto_getenv("CI")) {
        t->sec *= 6;
        t->nanosec *= 6;
    }

    corto_unlock(this);

}

int testMain(int argc, char *argv[]) {
    CORTO_UNUSED(argc);
    CORTO_UNUSED(argv);
    if (corto_threadTlsKey(&test_suiteKey, NULL)) {
        return -1;
    }
    return 0;
}

