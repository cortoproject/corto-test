/* This is a managed file. Do not delete this comment. */

#define FIND(p, i) corto(CORTO_LOOKUP, {.parent=p, .id=i})

#include <corto/test/test.h>
int16_t test_SuiteData_construct(
    test_SuiteData this)
{
    CORTO_UNUSED(this);

    if (corto_getenv("CI")) {
        /* Use less strict timings in less predictable CI environments */
        this->timeout.sec = 30;
        this->timeout.nanosec = 0;
    } else {
        this->timeout.sec = 5;
        this->timeout.nanosec = 0;
    }
    return 0;
}


typedef struct test_guard_t {
    test_SuiteData suite;
    corto_object testcase;
    struct corto_mutex_s m;
} test_guard_t;

void* test_guard(void *arg) {
    test_guard_t *data = arg;
    corto_time start;
    corto_bool quit = 0;

    corto_time_get(&start);

    corto_lock(data->suite);
    corto_time timeout = data->suite->timeout;
    corto_unlock(data->suite);

    corto_time timeExpire = corto_time_add(start, timeout);
    struct timespec tsExpire = {timeExpire.sec, timeExpire.nanosec};

    /* Testcase may change timeout, so after each timeout verify whether timeout
     * has changed, and whether a new wait is required */
    do {
        if (corto_mutex_lockTimed(&data->m, tsExpire) == ETIMEDOUT) {
            corto_time newTimeout, now;

            corto_time_get(&now);

            /* Obtain (new) timeout */
            corto_lock(data->suite);
            newTimeout = data->suite->timeout;
            corto_unlock(data->suite);

            /* If changed, wait for the remainder */
            if (newTimeout.sec != timeout.sec || newTimeout.nanosec != timeout.nanosec) {

                /* Calculate new expiry time */
                timeout = newTimeout;
                timeExpire = corto_time_add(start, timeout);

                /* If time has already expired, quit */
                if (corto_time_compare(now, timeExpire) == 1) {
                    quit = 1;
                }
            } else {
                quit = 1;
            }
        } else {
            corto_mutex_unlock(&data->m);
            break; /* Testcase finished */
        }
    } while (!quit);

    if (quit /*&& (!corto_beingTraced()) */) {
        test_erase();
        corto_id timeFmt; sprintf(timeFmt, "%d.%.9u", timeout.sec, timeout.nanosec);
        char ch, *ptr = &timeFmt[strlen(timeFmt) - 1];
        for (; (ch = *ptr) != '.'; ptr --) {
            if (ch == '0')
                ptr[1] = '\0';
            else
                break;
        }
        corto_object testroot = FIND(root_o, "test");
        corto_assert(testroot != NULL, "testroot disappeared?");
        corto_error("test: testcase '%s' timed out after %ss",
          test_id(NULL, data->testcase),
          timeFmt);

        abort();
    }

    return NULL;
}

int16_t test_SuiteData_run(
    test_SuiteData this,
    test_Case testcase)
{
    corto_set_ref(&this->testcase, testcase);

    if (testcase) {
        corto_attr attr;

        this->result.success = TRUE;
        extern corto_tls test_suiteKey;
        corto_thread guard = 0;
        test_guard_t *data = NULL;

        /* Setup test */
        corto_tls_set(test_suiteKey, this);
        attr = corto_set_attr(CORTO_ATTR_DEFAULT);
        test_SuiteData_setup(this);

        /* Setup termination guard */
        if (this->timeout.sec || this->timeout.nanosec) {
            data = corto_alloc(sizeof(test_guard_t));
            data->suite = this;
            data->testcase = testcase;
            corto_mutex_new(&data->m);
            corto_mutex_lock(&data->m);
            guard = corto_thread_new(test_guard, data);
        }

        this->assertCount = 0;
        if (corto_function(testcase)->kind == CORTO_PROCEDURE_CDECL) {
            ((void(*)(corto_object))corto_function(testcase)->fptr)(this);
        } else {
            corto_invoke(corto_function(testcase), NULL, this);
        }
        if (!this->assertCount) {
            test_empty();
        }

        /* Teardown test */
        if (!this->tearingDown) {
            this->tearingDown = TRUE;
            test_SuiteData_teardown(this);
        }
        corto_tls_set(test_suiteKey, NULL);
        corto_set_attr(attr);

        /* Stop termination guard */
        if (guard) {
            corto_mutex_unlock(&data->m);
            corto_thread_join(guard, NULL);
            corto_dealloc(data);
        }
    } else {
        corto_throw("no test provided for suite '%s'", corto_idof(this));
        goto error;
    }

    return 0;
error:
    return -1;
}

void test_SuiteData_setup_v(
    test_SuiteData this)
{

	CORTO_UNUSED(this);

}

void test_SuiteData_teardown_v(
    test_SuiteData this)
{

	CORTO_UNUSED(this);

}
