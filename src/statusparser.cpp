/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "statusparser.h"

using V = KVersionControlPlugin;

StatusMap parseGitStatus(const QByteArray &output, const QString &root)
{
    StatusMap result;
    const QList<QByteArray> records = output.split('\0');
    for (int i = 0; i < records.size(); ++i) {
        const QByteArray &record = records.at(i);
        if (record.size() < 4) {
            continue;
        }
        const char x = record.at(0);
        const char y = record.at(1);
        if (x == 'R' || x == 'C' || y == 'R' || y == 'C') {
            ++i; // the original path follows as its own NUL-separated record
        }

        QString relative = QString::fromLocal8Bit(record.mid(3));
        while (relative.endsWith(QLatin1Char('/'))) {
            relative.chop(1);
        }
        if (relative.isEmpty()) {
            continue;
        }
        const QString path = root + QLatin1Char('/') + relative;

        V::ItemVersion state = V::NormalVersion;
        switch (x) {
        case '!':
            state = V::IgnoredVersion;
            break;
        case '?':
            state = V::UnversionedVersion;
            break;
        case 'A':
        case 'C':
            state = V::AddedVersion;
            break;
        case 'D':
            state = V::RemovedVersion;
            break;
        case 'M':
        case 'R':
            state = V::LocallyModifiedVersion;
            break;
        case 'U':
            state = V::ConflictingVersion;
            break;
        default:
            break;
        }
        if (y == 'D') {
            state = V::MissingVersion;
        } else if (y == 'M') {
            state = V::LocallyModifiedUnstagedVersion;
        }
        if (x == 'U' || y == 'U' || (x == 'A' && y == 'A') || (x == 'D' && y == 'D')) {
            state = V::ConflictingVersion;
        }
        if (state != V::NormalVersion) {
            result.insert(path, state);
        }
    }
    return result;
}

StatusMap parseSvnStatus(const QByteArray &output, const QString &directory)
{
    StatusMap result;
    const QList<QByteArray> lines = output.split('\n');
    for (const QByteArray &line : lines) {
        if (line.size() < 9) {
            continue;
        }
        const char code = line.at(0);
        QString relative = QString::fromLocal8Bit(line.mid(8));
        if (relative.isEmpty()) {
            continue;
        }

        V::ItemVersion state = V::NormalVersion;
        switch (code) {
        case '?':
        case '~':
            state = V::UnversionedVersion;
            break;
        case 'I':
            state = V::IgnoredVersion;
            break;
        case 'A':
            state = V::AddedVersion;
            break;
        case 'D':
            state = V::RemovedVersion;
            break;
        case 'M':
        case 'R':
            state = V::LocallyModifiedVersion;
            break;
        case 'C':
            state = V::ConflictingVersion;
            break;
        case '!':
            state = V::MissingVersion;
            break;
        case ' ':
            if (line.at(1) == 'M') {
                state = V::LocallyModifiedVersion;
            }
            break;
        default:
            break;
        }
        if (state == V::NormalVersion) {
            continue;
        }

        QString path;
        if (relative == QLatin1String(".")) {
            path = directory;
            while (path.endsWith(QLatin1Char('/'))) {
                path.chop(1);
            }
        } else {
            while (relative.endsWith(QLatin1Char('/'))) {
                relative.chop(1);
            }
            path = directory + relative;
        }
        result.insert(path, state);
    }
    return result;
}
