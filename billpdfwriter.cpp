#include "billpdfwriter.h"
#include "billformatter.h"
#include <QFontDatabase>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QStringList>

bool BillPdfWriter::savePdf(const Bill& bill, const QString& filePath, QString& problemDescription)
{
    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    pdfWriter.setResolution(100);
    pdfWriter.setTitle("Bill " + bill.billId);
    QPainter painter;
    if (!painter.begin(&pdfWriter)) {
        problemDescription = "Could not create the PDF file " + filePath;
        return false;
    }

    QFont billFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    billFont.setPointSize(9);
    painter.setFont(billFont);
    const int lineHeight = painter.fontMetrics().lineSpacing();
    const int firstLineBaselineOffset = painter.fontMetrics().ascent();
    const int pageHeight = pdfWriter.height();
    const QStringList billLines = BillFormatter::screenText(bill).split(QChar('\n'));
    int currentTop = 0;
    for (const QString& billLine : billLines)
     {
        if (currentTop + lineHeight > pageHeight) {
            pdfWriter.newPage();
            currentTop = 0;
        }
        painter.drawText(0, currentTop + firstLineBaselineOffset, billLine);
        currentTop += lineHeight;
    }

    painter.end();
    return true;
}
