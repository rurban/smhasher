#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Types.h"
#include "VERSION.h"
extern HashInfo *g_hashUnderTest;
int test_count = 0;
int failed_count = 0;
clock_t timeBegin = clock();
clock_t timeEnd;
void start_testing() {
  timeBegin = clock();
}
bool okf( bool result, const char *name_fmt, const char *arg ) {
    test_count++;
    if (!result)
        failed_count++;
    printf("%sok %d - ", result ? "" : "not ", test_count);
    printf(name_fmt,arg);
    printf("\n"); // nl ok
    return result;
}


bool ok( bool result, const char *name, const char *comment ) {
    test_count++;
    if (!result)
        failed_count++;
    if (name && comment) {
        printf("%sok %d - %s # %s\n", result ? "" : "not ", test_count, name, comment);
    } else if (name) {
        printf("%sok %d - %s\n", result ? "" : "not ", test_count, name);
    } else {
        printf("%sok %d\n", result ? "" : "not ", test_count);
    }
    return result;
}

bool ok( bool result, const char *name ) {
    return ok(result, name, NULL);
}
bool ok( bool result ) {
    return ok(result, NULL, NULL);
}
bool ok() {
    return ok(true, NULL, NULL);
}
bool failed() {
    return ok(false, NULL, NULL);
}

void done_testing() {
    timeEnd = clock();
    printf("1..%d\n",test_count);
    int exitcode= failed_count ? 1 : 0;
    const char *name= g_hashUnderTest ? g_hashUnderTest->name : "SelfTest";

    if (failed_count) {
        printf("# Tests Failed. %s failed", name);
    } else {
        printf("# All Tests Passed. %s passed", name);
    }
    if (!failed_count || failed_count == test_count) {
        if (test_count > 1) {
            printf(" all %d tests run.\n", test_count);
        } else {
            printf(".\n");
        }
    } else {
        printf(" %d of %d tests.\n", failed_count, test_count);
    }
    printf( "# Testing took %f seconds\n", double(timeEnd - timeBegin) / double(CLOCKS_PER_SEC) );
    printf( "# ");
    print_version();
    exit(exitcode);
}

bool all_tests_ok() { return failed_count == 0; }

