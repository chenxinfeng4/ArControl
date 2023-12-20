/*
================1===========
 Safe C++, Or How to Avoid Most Common Mistakes in C++ Code
 by Vladimir Kushnir, (O’Reilly).

 Copyright 2012 Vladimir Kushnir, ISBN 9781449320935.

 If you feel your use of code examples falls outside fair use or the
 permission given above, feel free to contact us at permissions@oreilly.com.

 ===============2===========
 for qt, (QString valid), Chen Xinfeng

 ===============3===========
 #define SCPP_ASSERT_THROW_ON //SCPP_ASSERT = SCPP_ASSERT_THROW; defaultly off
 #define SCPP_TEST_ASSERT_ON  //SCPP_TEST_ASSERT = SCPP_ASSERT; defaultly off
 #define SCPP_ASSERT_DIALOG_ON //SCPP_ASSERT = SCPP_ASSERT_DIALOG; defaultly off
 SCPP_ASSERT(a==tr("hello");   // no Message
 SCPP_ASSERT(a==tr("hello"), QString("unicode")); // use QString Message
 SCPP_ASSERT_THROW(...)       // throw, menually
 SCPP_ASSERT_DIALOG(...)      // errordlg, NONE throw NOR exit;
*/
#ifndef SCPP_ASSERT_H
#define SCPP_ASSERT_H

#include <QString>

#define DEFLAUT_MSG "Error happend!"
void SCPP_AssertErrorHandler(const char* file_name,
                             unsigned line_number,
                             const QString message = DEFLAUT_MSG);
void SCPP_AssertErrorHandler_Throw(const char* file_name,
                                   unsigned line_number,
                                   const QString message = DEFLAUT_MSG)
                                   noexcept(false);
void SCPP_AssertErrorHandler_Dialog(const char* file_name,
                                   unsigned line_number,
                                   const QString message = DEFLAUT_MSG);
#define SCPP_ASSERT_THROW(condition, ...)            \
    if(!(condition)) {                               \
        SCPP_AssertErrorHandler_Throw(               \
             __FILE__, __LINE__, ## __VA_ARGS__);    \
    }

#define SCPP_ASSERT_DIALOG(condition, ...)           \
    if(!(condition)) {                               \
        SCPP_AssertErrorHandler_Dialog(              \
            __FILE__, __LINE__, ## __VA_ARGS__);     \
    }

// Permanent sanity check macro.
#if defined(SCPP_ASSERT_THROW_ON)
#   define SCPP_ASSERT  SCPP_ASSERT_THROW
#elif defined(SCPP_ASSERT_DIALOG_ON)
#   define SCPP_ASSERT  SCPP_ASSERT_DIALOG
#else
#   define SCPP_ASSERT(condition, ...)               \
        if(!(condition)) {                           \
            SCPP_AssertErrorHandler(                 \
                __FILE__, __LINE__, ## __VA_ARGS__); \
        }
#endif



// Temporary (for testing only) sanity check macro
#ifdef SCPP_TEST_ASSERT_ON
#	define SCPP_TEST_ASSERT(condition,msg) SCPP_ASSERT(condition, msg)
#else
#	define SCPP_TEST_ASSERT(condition,msg) // do nothing
#endif

#endif // SCPP_ASSERT_H
