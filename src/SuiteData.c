/* $CORTO_GENERATED
 *
 * SuiteData.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "corto/test/test.h"

corto_int16 _test_SuiteData_construct(
    test_SuiteData this)
{
/* $begin(corto/test/SuiteData/construct) */
    CORTO_UNUSED(this);
    return 0;
/* $end */
}

corto_int16 _test_SuiteData_run(
    test_SuiteData this,
    test_Case testcase)
{
/* $begin(corto/test/SuiteData/run) */
    corto_setref(&this->testcase, testcase);

    if (testcase) {
        corto_attr attr;

        this->result.success = TRUE;
        extern corto_threadKey test_suiteKey;
        corto_threadTlsSet(test_suiteKey, this);

        /* Setup default attributes */
        attr = corto_setAttr(CORTO_ATTR_DEFAULT);

        test_SuiteData_setup(this);
        this->assertCount = 0;
        corto_call(corto_function(testcase), NULL, this);
        if (!this->assertCount) {
            test_fail(" not implemented");
        }
        test_SuiteData_teardown(this);
        corto_threadTlsSet(test_suiteKey, NULL);

        corto_setAttr(attr);
    } else {
        corto_seterr("no test provided for suite '%s'", corto_nameof(this));
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_void _test_SuiteData_setup_v(
    test_SuiteData this)
{
/* $begin(corto/test/SuiteData/setup) */

	CORTO_UNUSED(this);

/* $end */
}

corto_void _test_SuiteData_teardown_v(
    test_SuiteData this)
{
/* $begin(corto/test/SuiteData/teardown) */

	CORTO_UNUSED(this);

/* $end */
}
