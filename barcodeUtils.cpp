
#include "barcodeUtils.h"
#include "code128decoder.h"
#include "code128encoder.h"
#include <QPainter>
#include <QRegularExpression>
#include <vector>
using namespace std;

optional<QString> decodeBarcode(const QImage &src)
{
    if (src.isNull())
        return nullopt;

    const QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    auto r = code128::decodeGray(gray.constBits(), gray.width(), gray.height(), static_cast<int>(gray.bytesPerLine()));
    if (!r)
        return nullopt;

    return QString::fromLatin1(r->c_str());
}

QString toSixDigits(const QString &text)
{
    const QString t = text.trimmed();
    static const QRegularExpression sixDigits("^[1-9]\\d{5}$"); //first digit not zero 
    if (sixDigits.match(t).hasMatch())
        return t;

    const QByteArray bytes = t.toUtf8();
    unsigned long long h = 1469598103934665603ULL;
    for (unsigned char c : bytes)
    {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return QString("%1").arg(static_cast<qulonglong>(h % 1000000ULL), 6, 10, QChar('0'));
}

QImage makeBarcodeImage(const QString &sixDigits)
{
    const vector<bool> modules = code128::encodeDigits(sixDigits.toStdString());
    if (modules.empty())
        return {};

    const int scale = 4;
    const int barHeight = 120;
    const int textHeight = 30;
    QImage img(static_cast<int>(modules.size()) * scale, barHeight + textHeight, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    for (size_t i = 0; i < modules.size();)
    {
        if (!modules[i])
        {
            ++i;
            continue;
        }
        size_t j = i;
        while (j < modules.size() && modules[j])
        {
            ++j;
        }
        p.drawRect(static_cast<int>(i) * scale, 0, static_cast<int>(j - i) * scale, barHeight);
        {
            i = j;
        }
    }
    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(14);
    p.setFont(f);

    p.drawText(QRect(0, barHeight, img.width(), textHeight), Qt::AlignCenter, sixDigits);
    return img;
}
