/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "statusparser.h"

#include <QtTest>

using V = KVersionControlPlugin;

class StatusParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void git()
    {
        // rename (new path first, then old), untracked dir, ignored dir, conflict, deleted in worktree
        const QByteArray raw = QByteArray("R  new.txt\0old.txt\0?? newdir/\0!! build/\0UU c.txt\0 D gone.txt\0 M m.txt\0", 70);
        const StatusMap m = parseGitStatus(raw, QStringLiteral("/r"));
        QCOMPARE(m.value(QStringLiteral("/r/new.txt")), V::LocallyModifiedVersion);
        QVERIFY(!m.contains(QStringLiteral("/r/old.txt")));
        QCOMPARE(m.value(QStringLiteral("/r/newdir")), V::UnversionedVersion);
        QCOMPARE(m.value(QStringLiteral("/r/build")), V::IgnoredVersion);
        QCOMPARE(m.value(QStringLiteral("/r/c.txt")), V::ConflictingVersion);
        QCOMPARE(m.value(QStringLiteral("/r/gone.txt")), V::MissingVersion);
        QCOMPARE(m.value(QStringLiteral("/r/m.txt")), V::LocallyModifiedUnstagedVersion);
    }

    void gitEdgeCases()
    {
        // both-added conflict, copy (source record skipped), staged add, short junk record ignored
        const QByteArray raw("AA c.txt\0C  copy.txt\0src.txt\0A  new.txt\0x\0", 42);
        const StatusMap m = parseGitStatus(raw, QStringLiteral("/r"));
        QCOMPARE(m.value(QStringLiteral("/r/c.txt")), V::ConflictingVersion);
        QCOMPARE(m.value(QStringLiteral("/r/copy.txt")), V::AddedVersion);
        QVERIFY(!m.contains(QStringLiteral("/r/src.txt")));
        QCOMPARE(m.value(QStringLiteral("/r/new.txt")), V::AddedVersion);
        QVERIFY(parseGitStatus(QByteArray(), QStringLiteral("/r")).isEmpty());
    }

    void svnEdgeCases()
    {
        // "." is the directory itself; trailing slash is dropped; property-only change; short line ignored
        const QByteArray out("M       .\n?       d/\n M      props\n\nA  x\n");
        const StatusMap m = parseSvnStatus(out, QStringLiteral("/w/"));
        QCOMPARE(m.value(QStringLiteral("/w")), V::LocallyModifiedVersion);
        QCOMPARE(m.value(QStringLiteral("/w/d")), V::UnversionedVersion);
        QCOMPARE(m.value(QStringLiteral("/w/props")), V::LocallyModifiedVersion);
        QCOMPARE(m.size(), 3);
    }

    void svn()
    {
        const QByteArray out("?       new\nM       a.c\n!       gone\nC       c\n");
        const StatusMap m = parseSvnStatus(out, QStringLiteral("/w/"));
        QCOMPARE(m.value(QStringLiteral("/w/new")), V::UnversionedVersion);
        QCOMPARE(m.value(QStringLiteral("/w/a.c")), V::LocallyModifiedVersion);
        QCOMPARE(m.value(QStringLiteral("/w/gone")), V::MissingVersion);
        QCOMPARE(m.value(QStringLiteral("/w/c")), V::ConflictingVersion);
    }
};

QTEST_APPLESS_MAIN(StatusParserTest)
#include "statusparser_test.moc"
