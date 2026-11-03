/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "rabbitvcsplugin.h"

#include <KFileItem>

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>

#include <sys/stat.h>

using V = KVersionControlPlugin;

namespace
{
KFileItem item(const QString &path, bool dir)
{
    return KFileItem(QUrl::fromLocalFile(path), QString(), dir ? S_IFDIR : S_IFREG);
}

void write(const QString &path, const QByteArray &data = "x")
{
    QFile f(path);
    QVERIFY2(f.open(QIODevice::WriteOnly), qPrintable(path));
    f.write(data);
}

bool git(const QString &dir, const QStringList &args)
{
    QProcess p;
    p.setWorkingDirectory(dir);
    p.start(QStringLiteral("git"),
            QStringList{QStringLiteral("-c"), QStringLiteral("user.name=t"), QStringLiteral("-c"), QStringLiteral("user.email=t@t")} + args);
    return p.waitForFinished(10000) && p.exitCode() == 0;
}
} // namespace

class PluginTest : public QObject
{
    Q_OBJECT

    QTemporaryDir m_tmp;
    QString m_repo;

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_tmp.isValid());
        m_repo = QDir(m_tmp.path()).canonicalPath() + QStringLiteral("/repo");
        QVERIFY(QDir().mkpath(m_repo + QStringLiteral("/sub")));
        QVERIFY(QDir().mkpath(m_repo + QStringLiteral("/other")));
        QVERIFY(git(m_repo, {QStringLiteral("init"), QStringLiteral("-q")}));
        write(m_repo + QStringLiteral("/.gitignore"), "build/\n");
        write(m_repo + QStringLiteral("/tracked.txt"));
        write(m_repo + QStringLiteral("/sub/a.txt"));
        write(m_repo + QStringLiteral("/other/b.txt"));
        QVERIFY(git(m_repo, {QStringLiteral("add"), QStringLiteral(".")}));
        QVERIFY(git(m_repo, {QStringLiteral("commit"), QStringLiteral("-q"), QStringLiteral("-m"), QStringLiteral("init")}));

        write(m_repo + QStringLiteral("/sub/a.txt"), "changed"); // unstaged modification
        write(m_repo + QStringLiteral("/other/b.txt"), "changed");
        QVERIFY(QDir().mkpath(m_repo + QStringLiteral("/newdir")));
        write(m_repo + QStringLiteral("/newdir/n.txt"));
        QVERIFY(QDir().mkpath(m_repo + QStringLiteral("/build")));
        write(m_repo + QStringLiteral("/build/o.bin"));
    }

    void fileNames()
    {
        QCOMPARE(RabbitVCSPlugin(RabbitVCSPlugin::Vcs::Git).fileName(), QStringLiteral(".git"));
        QCOMPARE(RabbitVCSPlugin(RabbitVCSPlugin::Vcs::Svn).fileName(), QStringLiteral(".svn"));
    }

    void statesAtRepoRoot()
    {
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        QVERIFY(plugin.beginRetrieval(m_repo + QLatin1Char('/')));
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/tracked.txt"), false)), V::NormalVersion);
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/sub/a.txt"), false)), V::LocallyModifiedUnstagedVersion);
        // the folder itself and its contents inherit "unversioned"/"ignored"
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/newdir"), true)), V::UnversionedVersion);
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/newdir/n.txt"), false)), V::UnversionedVersion);
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/build"), true)), V::IgnoredVersion);
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/build/o.bin"), false)), V::IgnoredVersion);
        // a folder with a modified file inside shows as modified
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/sub"), true)), V::LocallyModifiedVersion);
        plugin.endRetrieval();
    }

    void deletedFileIsMissing()
    {
        QVERIFY(QFile::remove(m_repo + QStringLiteral("/tracked.txt")));
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        QVERIFY(plugin.beginRetrieval(m_repo + QLatin1Char('/')));
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/tracked.txt"), false)), V::MissingVersion);
        write(m_repo + QStringLiteral("/tracked.txt"));
    }

    void subdirOnlyReportsItsOwnEntries()
    {
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        QVERIFY(plugin.beginRetrieval(m_repo + QStringLiteral("/sub/")));
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/sub/a.txt"), false)), V::LocallyModifiedUnstagedVersion);
        QCOMPARE(plugin.itemVersion(item(m_repo + QStringLiteral("/other/b.txt"), false)), V::NormalVersion);
    }

    void symlinkedPathStillMatches()
    {
        const QString link = m_tmp.path() + QStringLiteral("/link");
        QVERIFY(QFile::link(m_repo, link));
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        QVERIFY(plugin.beginRetrieval(link + QStringLiteral("/sub/")));
        QCOMPARE(plugin.itemVersion(item(link + QStringLiteral("/sub/a.txt"), false)), V::LocallyModifiedUnstagedVersion);
    }

    void outsideRepoFails()
    {
        QTemporaryDir plain;
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        QVERIFY(!plugin.beginRetrieval(plain.path() + QLatin1Char('/')));
        QVERIFY(plugin.localRepositoryRoot(plain.path()).isEmpty());
        QCOMPARE(plugin.localRepositoryRoot(m_repo + QStringLiteral("/sub")), m_repo);
    }

    void menuDependsOnContext()
    {
        RabbitVCSPlugin plugin(RabbitVCSPlugin::Vcs::Git);
        auto visible = [](const QList<QAction *> &top) {
            QStringList names;
            for (QAction *a : top.first()->menu()->actions()) {
                if (a->isVisible()) {
                    names << a->text();
                }
            }
            return names;
        };
        const QStringList dirMenu = visible(plugin.versionControlActions({item(m_repo, true)}));
        const QStringList fileMenu = visible(plugin.versionControlActions({item(m_repo + QStringLiteral("/sub/a.txt"), false)}));
        QVERIFY(dirMenu.contains(QStringLiteral("Push")));
        QVERIFY(!fileMenu.contains(QStringLiteral("Push")));
        QVERIFY(fileMenu.contains(QStringLiteral("Annotate...")));
        QVERIFY(!dirMenu.contains(QStringLiteral("Annotate...")));
    }
};

int main(int argc, char **argv)
{
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication app(argc, argv);
    PluginTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "plugin_test.moc"
