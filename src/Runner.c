/* $CORTO_GENERATED
 *
 * Runner.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <corto/test/test.h>

/* $header() */
void test_updateProgress(test_Runner this) {
    int i;
    char *str;
    if (test_CaseListSize(this->failures)) {
        corto_asprintf(&str, "%s: PASS:%d, %sFAIL%s:%d",
            this->name,
            test_CaseListSize(this->successes),
            CORTO_RED,
            CORTO_NORMAL,
            test_CaseListSize(this->failures));
    } else {
        corto_asprintf(&str, "%s: %sPASS%s:%d, FAIL:%d",
            this->name,
            CORTO_GREEN,
            CORTO_NORMAL,
            test_CaseListSize(this->successes),
            test_CaseListSize(this->failures));
    }

    for (i = 0; i < strlen(str) + 2 * 2; i++) {
        printf("\b");
    }
    printf("%s", str);
    fflush(stdout);
}
/* $end */

corto_int16 _test_Runner_construct(
    test_Runner this)
{
/* $begin(corto/test/Runner/construct) */
    corto_object testroot = corto_lookup(root_o, "test");
    if (!testroot) {
        corto_seterr("test: /test not found");
        goto error;
    }

    /* If a testcase is provided, run it. Otherwise, discover testcases and
     * forward to separate process. */
    if (this->testcase) {
        corto_object testcase = corto_resolve(testroot, this->testcase);
        if (testcase) {
            corto_time start;
            corto_timeGet(&start);

            corto_trace("test:   START %s", this->testcase);
            corto_type testClass = corto_parentof(testcase);
            test_SuiteData suite = test_SuiteData(corto_create(testClass));

            if (test_SuiteData_run(suite, testcase)) {
                corto_error("test: failed to define test suite");
                test_CaseListAppend(this->failures, testcase);
            } else {
                corto_object prev = corto_setOwner(this);
                corto_define(suite);
                corto_setOwner(prev);

                corto_time now;
                corto_timeGet(&now);
                now = corto_timeSub(now, start);
                corto_id timeFmt; sprintf(timeFmt, "%d.%.9u", now.sec, now.nanosec);
                char ch, *ptr = &timeFmt[strlen(timeFmt) - 1];
                for (; (ch = *ptr) != '.'; ptr --) {
                    if (ch == '0')
                        ptr[1] = '\0';
                    else
                        break;
                }
                corto_trace("test:   DONE  %s (%ss)", this->testcase, timeFmt);
            }

            corto_delete(suite);

        } else {
            /* Testcase not found, don't report error */
            corto_trace("test: %s NOT FOUND IN %s", this->name);
        }
    } else {
        corto_listen(this, test_Runner_runTest_o, CORTO_ON_DEFINE | CORTO_ON_TREE, root_o, NULL);
    }

    corto_release(testroot);

    return 0;
error:
    return -1;
/* $end */
}

corto_void _test_Runner_destruct(
    test_Runner this)
{
/* $begin(corto/test/Runner/destruct) */
    if (!this->testcase) {
        test_updateProgress(this);
        printf("\n");
        corto_silence(this, test_Runner_runTest_o, CORTO_ON_DEFINE | CORTO_ON_TREE, root_o);
    }
/* $end */
}

corto_void _test_Runner_runTest(
    test_Runner this,
    corto_object observable)
{
/* $begin(corto/test/Runner/runTest) */

    if (corto_instanceof(corto_type(test_Case_o), observable)) {
        corto_id testcaseId;
        corto_int8 err, ret;

        test_id(testcaseId, observable);

        {
            corto_id cmd;
            corto_trace("\ntest: TESTCASE %s", testcaseId);
            corto_trace("test:   EXEC  %s",
                test_command(cmd, this->lib, observable));
        }

        corto_pid pid = corto_procrun(
            "corto",
            (char*[]){
                "corto",
                "--mute",
                this->lib,
                testcaseId,
                NULL
            }
        );

        if (!pid) {
            corto_error("%sFAIL%s: %s (%s)\n",
                CORTO_RED,
                CORTO_NORMAL,
                corto_lasterr(),
                testcaseId);
        } else if ((err = corto_procwait(pid, &ret)) || ret) {
            if (err > 0) {
                corto_id cmd;
                test_erase();
                fprintf(stderr, "%sFAIL%s: %s: test crashed with signal %d\n",
                    CORTO_RED,
                    CORTO_NORMAL,
                    testcaseId, err);
                fprintf(stderr, "  When debugging, use the following command:\n  %s\n",
                    test_command(cmd, this->lib, observable));
            } else if (err < 0) {
                int i;
                for (i = 0; i < 255; i++) {
                    fprintf(stderr, "\b");
                }
                fprintf(stderr, "%sFAIL%s: %s (%s)\n",
                    CORTO_RED,
                    CORTO_NORMAL,
                    corto_lasterr(),
                    testcaseId);
            } else {
                /* Process exited with a returncode != 0 so
                 * must've printed an error msg itself */
            }
            test_CaseListAppend(this->failures, observable);
        } else {
            test_CaseListAppend(this->successes, observable);
        }
        this->testsRun++;

        /* Don't print statistics when in CI mode */
        corto_string ciEnv = corto_getenv("CI");
        if ((!ciEnv || stricmp(ciEnv, "true")) && (corto_verbosityGet() > CORTO_TRACE)) {
            corto_time start;
            corto_timeGet(&start);
            corto_float64 diff =
              corto_timeToDouble(start) - corto_timeToDouble(this->timer);

            if (diff > 0.05) {
                test_updateProgress(this);
                corto_timeGet(&this->timer);
            }
        } else {
            /* When in CI mode, show each individual testcase. Failures are
             * already reported. */
            if (!err) {
                printf("%s%s%s: %s\n",
                    CORTO_GREEN,
                    "PASS",
                    CORTO_NORMAL,
                    testcaseId);
            }
        }
    }
/* $end */
}
