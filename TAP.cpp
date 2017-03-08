#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Types.h"
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
    int exitcode;
    if (failed_count) {
        printf("# Tests failed. %s failed %d of %d tests.\n",
            g_hashUnderTest->name, failed_count, test_count);
        exitcode= 1;
    } else {
        printf("# All tests passed. %s passed %d tests.\n",
            g_hashUnderTest->name, test_count);
        exitcode= 0;
    }
    printf( "# Testing took %f seconds\n", double(timeEnd - timeBegin) / double(CLOCKS_PER_SEC) );
    exit(exitcode);
}


