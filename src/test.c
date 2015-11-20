/* $CORTO_GENERATED
 *
 * test.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

/* $header() */
corto_threadKey test_suiteKey;
/* $end */

corto_bool _test_assert(corto_bool condition, corto_string str_condition, corto_uint32 __line) {
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

corto_bool _test_assertEqual(corto_any a, corto_any b, corto_string str_a, corto_string str_b, corto_uint32 __line) {
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

corto_void _test_fail(corto_string err) {
/* $begin(corto/test/fail) */
    corto_id id;
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

    corto_error("FAIL: %s:%s",
        corto_fullname(this->test, id),
        this->result.errmsg ? this->result.errmsg : "");

    corto_string lasterr = corto_lasterr();
    if (lasterr) {
        corto_error("   details: %s", lasterr);
    }

    /* Run teardown before exit */
    test_SuiteData_teardown(this);

    exit(-1);
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
