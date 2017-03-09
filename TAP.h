#pragma once
bool ok( bool result, const char *name, const char *comment );
bool ok( bool result, const char *name );
bool okf( bool result, const char *name_fmt, const char *name_arg );
bool ok( bool result );
bool ok();
bool failed();
void done_testing();
bool all_tests_ok();


