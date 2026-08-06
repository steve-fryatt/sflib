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
 * \file: test_msgs.c
 *
 * Unit Tests for the msgs.c code.
 *
 * The tests in this file all expext a valid messages file to be initialised
 * in setUp() and closed down in tearDown().
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
{
	msgs_initialise("<SFLib$TestDir>.Messages");
}

/**
 * Unit Test teardown.
 */

void tearDown(void)
{
	msgs_terminate();
}

/**
 * The Unit Tests.
 */

/*** Write to a NULL buffer pointer. ***/

void lookup_result_to_null_buffer()
{
	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Token", NULL, 0), "Fails when given null buffer");
}

void lookup_to_null_buffer()
{
	TEST_ASSERT_EQUAL_PTR_MESSAGE(NULL, msgs_lookup("Token", NULL, 0), "Returns a NULL pointer");
}

/*** Write to a zero length buffer. ***/

void lookup_result_to_zero_length_buffer()
{
	char buffer[1] = { '~' };

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Token", buffer, 0), "Fails when given zero length buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[0], "The guard character hasn't changed");
}

void lookup_to_zero_length_buffer()
{
	char buffer[1] = { '~' };

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Token", buffer, 0), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[0], "The guard character hasn't changed");
}

/*** Lookup a NULL token and terminate the buffer. ***/

void lookup_result_with_null_token_full_space()
{
	char buffer[101];
	buffer[0] = '~';
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result(NULL, buffer, 100), "Fails when given NULL token");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', buffer[0], "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_with_null_token_full_space()
{
	char buffer[101];
	buffer[0] = '~';
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup(NULL, buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', buffer[0], "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_with_null_token_no_space()
{
	char buffer[2];
	buffer[0] = '~';
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result(NULL, buffer, 100), "Fails when given NULL token");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', buffer[0], "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_with_null_token_no_space()
{
	char buffer[2];
	buffer[0] = '~';
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup(NULL, buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', buffer[0], "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/*** Lookup an invalid token and copy the token into the buffer. ***/

void lookup_result_with_invalid_token_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Invalid", buffer, 100), "Fails when given invalid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_with_invalid_token_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Invalid", buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_with_invalid_token_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_ERROR, msgs_lookup_result("Invalid", buffer, 1), "Fails when given invalid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_with_invalid_token_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Invalid", buffer, 1), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The buffer should have been terminated");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/*** Lookup an empty message ***/

void lookup_result_with_empty_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_OK, msgs_lookup_result("Empty", buffer, 100), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The message should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_with_empty_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Empty", buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The message should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_with_empty_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_BUFFER_FULL, msgs_lookup_result("Empty", buffer, 1), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The message should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_with_empty_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("Empty", buffer, 1), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The message should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/*** Lookup a simple message. ***/

void lookup_result_with_simple_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_OK, msgs_lookup_result("TenChar", buffer, 100), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1234567890", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_with_simple_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("TenChar", buffer, 100), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1234567890", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_with_simple_message_some_space()
{
	char buffer[6];
	buffer[5] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_BUFFER_FULL, msgs_lookup_result("TenChar", buffer, 5), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1234", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[5], "The guard character hasn't changed");
}

void lookup_with_simple_message_some_space()
{
	char buffer[6];
	buffer[5] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("TenChar", buffer, 5), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1234", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[5], "The guard character hasn't changed");
}

void lookup_result_with_simple_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_BUFFER_FULL, msgs_lookup_result("TenChar", buffer, 1), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_with_simple_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_lookup("TenChar", buffer, 1), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/*** Lookup a message with parameters. ***/

void lookup_result_with_parameterised_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_OK, msgs_param_lookup_result("Parameters", buffer, 100, "AA", "BB", "CC", "DD"), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("12AA34BB56CC78DD90", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_with_parameterised_message_full_space()
{
	char buffer[101];
	buffer[100] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_param_lookup("Parameters", buffer, 100, "AA", "BB", "CC", "DD"), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("12AA34BB56CC78DD90", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[100], "The guard character hasn't changed");
}

void lookup_result_with_parameterised_message_some_space()
{
	char buffer[12];
	buffer[11] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_BUFFER_FULL, msgs_param_lookup_result("Parameters", buffer, 11, "AA", "BB", "CC", "DD"), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("12AA34BB56", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[11], "The guard character hasn't changed");
}

void lookup_with_parameterised_message_some_space()
{
	char buffer[12];
	buffer[11] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_param_lookup("Parameters", buffer, 11, "AA", "BB", "CC", "DD"), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("12AA34BB56", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[11], "The guard character hasn't changed");
}

void lookup_result_with_parameterised_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_INT_MESSAGE(MSGS_STATUS_BUFFER_FULL, msgs_param_lookup_result("Parameters", buffer, 1, "AA", "BB", "CC", "DD"), "Succeeds when given valid token");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

void lookup_with_parameterised_message_no_space()
{
	char buffer[2];
	buffer[1] = '~';

	TEST_ASSERT_EQUAL_PTR_MESSAGE(buffer, msgs_param_lookup("Parameters", buffer, 1, "AA", "BB", "CC", "DD"), "Returns a pointer to the buffer");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", buffer, "The token should have been copied to the buffer");
	TEST_ASSERT_EQUAL_CHAR_MESSAGE('~', buffer[1], "The guard character hasn't changed");
}

/**
 * The main test runner.
 */

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(lookup_result_to_null_buffer);
	RUN_TEST(lookup_to_null_buffer);
	RUN_TEST(lookup_result_to_zero_length_buffer);
	RUN_TEST(lookup_to_zero_length_buffer);
	RUN_TEST(lookup_result_with_null_token_full_space);
	RUN_TEST(lookup_with_null_token_full_space);
	RUN_TEST(lookup_result_with_null_token_no_space);
	RUN_TEST(lookup_with_null_token_no_space);
	RUN_TEST(lookup_result_with_invalid_token_full_space);
	RUN_TEST(lookup_with_invalid_token_full_space);
	RUN_TEST(lookup_result_with_invalid_token_no_space);
	RUN_TEST(lookup_with_invalid_token_no_space);
	RUN_TEST(lookup_result_with_empty_message_full_space);
	RUN_TEST(lookup_with_empty_message_full_space);
	RUN_TEST(lookup_result_with_empty_message_no_space);
	RUN_TEST(lookup_with_empty_message_no_space);
	RUN_TEST(lookup_result_with_simple_message_full_space);
	RUN_TEST(lookup_with_simple_message_full_space);
	RUN_TEST(lookup_result_with_simple_message_some_space);
	RUN_TEST(lookup_with_simple_message_some_space);
	RUN_TEST(lookup_result_with_simple_message_no_space);
	RUN_TEST(lookup_with_simple_message_no_space);
	RUN_TEST(lookup_result_with_parameterised_message_full_space);
	RUN_TEST(lookup_with_parameterised_message_full_space);
	RUN_TEST(lookup_result_with_parameterised_message_some_space);
	RUN_TEST(lookup_with_parameterised_message_some_space);
	RUN_TEST(lookup_result_with_parameterised_message_no_space);
	RUN_TEST(lookup_with_parameterised_message_no_space);
	return UNITY_END();
}
