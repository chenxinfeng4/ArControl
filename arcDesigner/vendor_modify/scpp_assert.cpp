#include "scpp_assert.h"

#include <stdlib.h>	// exit()
#include <QMessageBox> // show dialog
#include <QTextStream>

static const QString errorstrconst(const char* file_name,
                                  unsigned line_number,
                                  const QString message)
{
    QString *s = new QString("ASSERT failure: \"%1\", file %2, line %3\n");
    *s = (*s).arg(message).arg(file_name).arg(line_number);
    QTextStream err(stderr);
    err << *s;
    return *s;
}

void SCPP_AssertErrorHandler(const char* file_name,
                             unsigned line_number,
                             const QString message)
{
    // Terminate application
    qt_assert_x("assert", qUtf8Printable(message),file_name,line_number);
    Q_UNREACHABLE();
    exit(1);
}

void SCPP_AssertErrorHandler_Throw(const char* file_name,
                                   unsigned line_number,
                                   const QString message)
                                   noexcept(false)
{
    // Throw an error
    throw errorstrconst(file_name, line_number, message);
}

void SCPP_AssertErrorHandler_Dialog(const char* file_name,
                                    unsigned line_number,
                                    const QString message)
{
    QMessageBox::critical(0, QObject::tr("Error"),message);
    errorstrconst(file_name, line_number, message);
    // Don't Terminate application
    qt_noop();
}
