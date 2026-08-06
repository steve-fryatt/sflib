/* Copyright 2026, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file: test_msgs_without_instance.c
 *
 * Unit Tests for the msgs.c code.
 *
 * The tests in this file expect no msgs instance to exist.
 */

/* ANSI C Header files. */

/* Acorn C Header files. */

/* SFLib Header files. */

/* Unity Header files. */

#include "unity.h"

/* Locate Application header file. */

#include "msgs.h"

/**
 * Unit Test setup.
 */

void setUp(void)
{ }

/**
 * Unit Test teardown.
 */

void tearDown(void)
{ }

/**
 * The Unit Tests.
 */

/*** Bad initialisation ***/

void lookup_result_on_null_instance_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Token", buffer, 100), "Fails when not initialised");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Token", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_on_null_instance_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Token", buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Token", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_on_null_instance_some_space()
{
	char buffer[5];
	buffer[4] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Token", buffer, 4), "Fails when not initialised");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Tok", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[4], "The guard character hasn't changed");
}

void lookup_on_null_instance_some_space()
{
	char buffer[5];
	buffer[4] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Token", buffer, 4), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Tok", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[4], "The guard character hasn't changed");
}

void lookup_result_on_null_instance_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Token", buffer, 1), "Fails when not initialised");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_on_null_instance_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Token", buffer, 1), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/**
 * The main test runner.
 */

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(lookup_result_on_null_instance_full_space);
	RUN_TEST(lookup_on_null_instance_full_space);
	RUN_TEST(lookup_result_on_null_instance_some_space);
	RUN_TEST(lookup_on_null_instance_some_space);
	RUN_TEST(lookup_result_on_null_instance_no_space);
	RUN_TEST(lookup_on_null_instance_no_space);
	return UNITY_END();
}
