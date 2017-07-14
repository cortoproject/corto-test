/* This is a managed file. Do not delete this comment. */

#include <corto/test/test.h>

void test_updateProgress(test_Runner this) {
    int i;
    char *str;
    if (test_CaseListSize(this->failures)) {
        if (test_CaseListSize(this->empty)) {
            corto_asprintf(&str, "%s: PASS:%d, %sFAIL%s:%d, %sEMPTY%s:%d ",
                this->name,
                test_CaseListSize(this->successes),
                CORTO_RED,
                CORTO_NORMAL,
                test_CaseListSize(this->failures),
                CORTO_YELLOW,
                CORTO_NORMAL,
                test_CaseListSize(this->empty));
            } else {
                corto_asprintf(&str, "%s: PASS:%d, %sFAIL%s:%d, EMPTY:%d ",
                    this->name,
                    test_CaseListSize(this->successes),
                    CORTO_RED,
                    CORTO_NORMAL,
                    test_CaseListSize(this->failures),
                    test_CaseListSize(this->empty));
            }
    } else {
        if (test_CaseListSize(this->empty)) {
            corto_asprintf(&str, "%s: %sPASS%s:%d, FAIL:%d, %sEMPTY%s:%d ",
                this->name,
                CORTO_GREEN,
                CORTO_NORMAL,
                test_CaseListSize(this->successes),
                test_CaseListSize(this->failures),
                CORTO_YELLOW,
                CORTO_NORMAL,
                test_CaseListSize(this->empty));
        } else {
            corto_asprintf(&str, "%s: %sPASS%s:%d, FAIL:%d, EMPTY:%d ",
                this->name,
                CORTO_GREEN,
                CORTO_NORMAL,
                test_CaseListSize(this->successes),
                test_CaseListSize(this->failures),
                test_CaseListSize(this->empty));
        }
    }

    for (i = 0; i < strlen(str) + 2 * 2; i++) {
        printf("\b");
    }
    printf("%s", str);
    fflush(stdout);
}

int16_t test_Runner_construct(
    test_Runner this)
{
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

            /* Set TARGET in case this testcase is called directly */
            char *oldenv = corto_getenv("CORTO_TARGET");
            corto_setenv("CORTO_TARGET", "$HOME/.corto_tmp");

            corto_trace("test:   START %s", this->testcase);
            corto_type testClass = corto_parentof(testcase);
            test_SuiteData suite = test_SuiteData(corto_create(testClass));

            if (corto_getenv("CORTO_TEST_BY_ID") && !stricmp(corto_getenv("CORTO_TEST_BY_ID"), "true")) {
                corto_id cmd;
                fprintf(stderr, "RUN %s\n",
                    test_command(cmd, this->lib, testcase));
            }

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

            corto_setenv("CORTO_TARGET", oldenv);

            corto_delete(suite);
        } else {
            /* Testcase not found, don't report error */
            corto_trace("test: %s NOT FOUND", this->testcase);
        }
    } else {
        if (corto_observer_observe(test_Runner_runTest_o, this, root_o)) {
            goto error;
        }
    }

    corto_release(testroot);

    return 0;
error:
    return -1;
}

void test_Runner_destruct(
    test_Runner this)
{
    if (!this->testcase) {
        test_updateProgress(this);
        printf("\n");
        corto_observer_unobserve(test_Runner_runTest_o, this, root_o);
    }
}

void test_Runner_runTest(
    corto_observerEvent *e)
{
    test_Runner this = e->instance;
    corto_object object = e->data;

    if (corto_instanceof(corto_type(test_Case_o), object)) {
        corto_id testcaseId;
        corto_int8 err = 0, ret = 0;
        corto_string ciEnv = corto_getenv("CI");

        /* Set TARGET to temporary environment so tests can't contaminate the
         * current environment */
        char *oldenv = corto_getenv("CORTO_TARGET");

        corto_setenv("CORTO_TARGET", "$HOME/.corto_tmp");

        test_id(testcaseId, object);

        {
            corto_id cmd; (void)cmd;
            corto_trace("\ntest: TESTCASE %s", testcaseId);
            corto_trace("test:   EXEC  %s",
                test_command(cmd, this->lib, object));
        }

        char *tool = getenv("CORTO_TEST_TOOL");
        corto_pid pid = 0;

        if (!tool || !strcmp(tool, "")) {
            pid = corto_procrun(
                "corto",
                (char*[]){
                    "corto",
                    "--mute",
                    "-l",
                    this->lib,
                    testcaseId,
                    NULL
                }
            );
        } else if (!strcmp(tool, "check-memory")) {
            setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
            if (!strcmp(CORTO_OS_STRING, "linux")) {
                pid = corto_procrun(
                    "valgrind",
                    (char*[]){
                        "valgrind",
                        "-q",
                        "corto",
                        "-l",
                        this->lib,
                        testcaseId,
                        NULL
                    }
                );
            } else {
                corto_seterr("check-memory is only supported on Linux (uses valgrind)");
            }
        } else if (!strcmp(tool, "check-thread")) {
            setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
            if (!strcmp(CORTO_OS_STRING, "linux")) {
                pid = corto_procrun(
                    "valgrind",
                    (char*[]){
                        "valgrind",
                        "-q",
                        "--tool=helgrind",
                        "corto",
                        "-l",
                        this->lib,
                        testcaseId,
                        NULL
                    }
                );
            } else {
                corto_seterr("check-thread is only supported on Linux (uses valgrind)");
            }
        }

        if (!pid) {
            fprintf(stderr, "%sFAIL%s: %s (%s)\n",
                CORTO_RED,
                CORTO_NORMAL,
                corto_lasterr(),
                testcaseId);
        } else if ((err = corto_procwait(pid, &ret)) || ret) {
            if (err > 0) {
                test_erase();
                fprintf(stderr, "%sFAIL%s: %s: test crashed with signal %d\n",
                    CORTO_RED,
                    CORTO_NORMAL,
                    testcaseId, err);
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
            if (!ciEnv || stricmp(ciEnv, "true")) {
                corto_id cmd;
                fprintf(stderr, " Use this command to debug the testcase:\n  %s\n\n",
                    test_command(cmd, this->lib, object));
            }

            if (ret == 1) {
                test_CaseListAppend(this->empty, object);
            } else {
                test_CaseListAppend(this->failures, object);
            }
        } else {
            test_CaseListAppend(this->successes, object);
        }
        this->testsRun++;

        /* Don't print statistics when in CI mode */
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
            if (!err && !ret) {
                printf("%s%s%s: %s\n",
                    CORTO_GREEN,
                    "PASS",
                    CORTO_NORMAL,
                    testcaseId);
            }
        }

        if (corto_rm(corto_getenv("CORTO_TARGET"))) {
            corto_lasterr(); /* Catch error */
            corto_seterr(NULL);
        }
        corto_setenv("CORTO_TARGET", oldenv);
    }
}

