#pragma once

#include <QList>
#include <QRect>
#include <QString>

namespace AptNative {

struct OverlayOutput {
    QString name;
    QRect geometry;
};

inline qsizetype overlayOutputIndex(const QList<OverlayOutput> &outputs,
                                   const QString &preferred,
                                   const QPoint &cursor,
                                   qsizetype primary)
{
    if (!preferred.isEmpty()) {
        for (qsizetype i = 0; i < outputs.size(); ++i) {
            if (outputs[i].name == preferred) return i;
        }
    }
    for (qsizetype i = 0; i < outputs.size(); ++i) {
        if (outputs[i].geometry.contains(cursor)) return i;
    }
    return outputs.isEmpty() ? -1 : qBound(qsizetype(0), primary, outputs.size() - 1);
}

} // namespace AptNative
