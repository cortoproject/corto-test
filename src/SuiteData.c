/* $CORTO_GENERATED
 *
 * SuiteData.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <corto/test/test.h>

corto_int16 _test_SuiteData_construct(
    test_SuiteData this)
{
/* $begin(corto/test/SuiteData/construct) */
    CORTO_UNUSED(this);
    this->timeout.sec = 10;
    this->timeout.nanosec = 0;
    return 0;
/* $end */
}

/* $header(corto/test/SuiteData/run) */

typedef struct test_guard_t {
    test_SuiteData suite;
    corto_object testcase;
    struct corto_mutex_s m;
} test_guard_t;

void* test_guard(void *arg) {
    test_guard_t *data = arg;
    corto_time start;
    corto_bool quit = 0;

    corto_timeGet(&start);

    corto_lock(data->suite);
    corto_time timeout = data->suite->timeout;
    corto_unlock(data->suite);

    corto_time timeExpire = corto_timeAdd(start, timeout);

    /* Testcase may change timeout, so after each timeout verify whether timeout
     * has changed, and whether a new wait is required */
    do {
        if (corto_mutexLockTimed(&data->m, timeExpire) == ETIMEDOUT) {
            corto_time newTimeout, now;

            corto_timeGet(&now);

            /* Obtain (new) timeout */
            corto_lock(data->suite);
            newTimeout = data->suite->timeout;
            corto_unlock(data->suite);

            /* If changed, wait for the remainder */
            if (newTimeout.sec != timeout.sec || newTimeout.nanosec != timeout.nanosec) {

                /* Calculate new expiry time */
                timeout = newTimeout;
                timeExpire = corto_timeAdd(start, timeout);

                /* If time has already expired, quit */
                if (corto_time_compare(now, timeExpire) == 1) {
                    quit = 1;
                }
            } else {
                quit = 1;
            }
        } else {
            break; /* Testcase finished */
        }
    } while (!quit);

    if (quit) {
        test_erase();
        corto_id timeFmt; sprintf(timeFmt, "%d.%.9u", timeout.sec, timeout.nanosec);
        char ch, *ptr = &timeFmt[strlen(timeFmt) - 1];
        for (; (ch = *ptr) != '.'; ptr --) {
            if (ch == '0')
                ptr[1] = '\0';
            else
                break;
        }
        corto_object testroot = corto_lookup(root_o, "test");
        corto_assert(testroot != NULL, "testroot disappeared?");
        corto_error("test: testcase '%s' timed out after %ss",
          test_id(NULL, data->testcase),
          timeFmt);
        corto_release(testroot);

        abort();
    }

    return NULL;
}
/* $end */
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
        corto_thread guard = 0;
        test_guard_t *data = NULL;

        /* Setup test */
        corto_threadTlsSet(test_suiteKey, this);
        attr = corto_setAttr(CORTO_ATTR_DEFAULT);
        test_SuiteData_setup(this);

        /* Setup termination guard */
        if (this->timeout.sec || this->timeout.nanosec) {
            data = corto_alloc(sizeof(test_guard_t));
            data->suite = this;
            data->testcase = testcase;
            corto_mutexNew(&data->m);
            corto_mutexLock(&data->m);
            guard = corto_threadNew(test_guard, data);
        }

        this->assertCount = 0;
        corto_call(corto_function(testcase), NULL, this);
        if (!this->assertCount) {
            test_fail(" not implemented");
        }

        /* Teardown test */
        test_SuiteData_teardown(this);
        corto_threadTlsSet(test_suiteKey, NULL);
        corto_setAttr(attr);

        /* Stop termination guard */
        if (guard) {
            corto_mutexUnlock(&data->m);
            corto_threadJoin(guard, NULL);
            corto_dealloc(data);
        }
    } else {
        corto_seterr("no test provided for suite '%s'", corto_idof(this));
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
