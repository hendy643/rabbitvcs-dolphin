/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "rabbitvcsplugin.h"
#include "statusparser.h"

#include <KFileItem>

#include <QAction>
#include <QIcon>
#include <QProcess>
#include <QVariant>

#include <iterator>

namespace
{
QByteArray runProcess(const QString &program, const QStringList &args, const QString &workingDirectory, int *exitCode)
{
    QProcess process;
    if (!workingDirectory.isEmpty()) {
        process.setWorkingDirectory(workingDirectory);
    }
    process.start(program, args);
    if (!process.waitForStarted()) {
        if (exitCode) {
            *exitCode = -1;
        }
        return {};
    }
    if (!process.waitForFinished(10000)) {
        process.kill();
        process.waitForFinished();
        if (exitCode) {
            *exitCode = -1;
        }
        return {};
    }
    if (exitCode) {
        *exitCode = process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
    }
    return process.readAllStandardOutput();
}

using P = RabbitVCSPlugin;

const RabbitVCSPlugin::ActionDef gitActions[] = {
    {"Update", "rabbitvcs-update", "update", nullptr, P::InRepoDir, true},
    {"Commit...", "rabbitvcs-commit", "commit", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Push", "go-up", "push", nullptr, P::InRepoDir, true},
    {"Show Changes...", "rabbitvcs-changes", "changes", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Show Log", "rabbitvcs-show_log", "log", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Edit Conflicts", "rabbitvcs-editconflicts", "editconflicts", nullptr, P::InRepoFile, true},
    {"Rename...", "rabbitvcs-rename", "rename", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Delete", "rabbitvcs-delete", "delete", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Revert...", "rabbitvcs-revert", "revert", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Clean", "rabbitvcs-clear", "clean", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Reset", "rabbitvcs-reset", "reset", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Branches", "rabbitvcs-branch", "branches", nullptr, P::InRepoDir, true},
    {"Tags", "rabbitvcs-branch", "tags", nullptr, P::InRepoDir, true},
    {"Remotes", "rabbitvcs-checkmods", "remotes", nullptr, P::InRepoDir, true},
    {"Export...", "rabbitvcs-export", "export", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Merge...", "rabbitvcs-merge", "merge", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Annotate...", "rabbitvcs-annotate", "annotate", nullptr, P::InRepoFile, true},
    {"Apply Patch...", "rabbitvcs-applypatch", "applypatch", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Create Patch...", "rabbitvcs-createpatch", "createpatch", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Add...", "rabbitvcs-add", "add", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Add to Ignore List", "rabbitvcs-no", "ignore", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Checkout...", "rabbitvcs-checkout", "checkout", nullptr, P::InRepoDir | P::OutOfVcs, true},
    {"Create Repository Here", "rabbitvcs-run", "create", "--vcs git", P::OutOfVcs, true},
    {"Clone...", "rabbitvcs-checkout", "clone", nullptr, P::OutOfVcs, true},
    {"Settings", "rabbitvcs-settings", "settings", nullptr,
     P::InRepoDir | P::InRepoFile | P::OutOfVcs, false},
    {"About", "rabbitvcs-about", "about", nullptr,
     P::InRepoDir | P::InRepoFile | P::OutOfVcs, false},
};

const RabbitVCSPlugin::ActionDef svnActions[] = {
    {"Update", "rabbitvcs-update", "update", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Commit...", "rabbitvcs-commit", "commit", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Show Changes...", "rabbitvcs-changes", "changes", nullptr, P::InRepoFile, true},
    {"Show Log", "rabbitvcs-show_log", "log", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Repo-browser", "rabbitvcs-drive", "browser", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Check for Modifications", "rabbitvcs-checkmods", "checkmods", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Edit Conflicts", "rabbitvcs-editconflicts", "editconflicts", nullptr, P::InRepoFile, true},
    {"Resolve...", "rabbitvcs-resolve", "markresolved", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Update to Revision...", "rabbitvcs-update", "updateto", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Rename...", "rabbitvcs-rename", "rename", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Delete", "rabbitvcs-delete", "delete", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Revert...", "rabbitvcs-revert", "revert", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Clean up...", "rabbitvcs-cleanup", "cleanup", nullptr, P::InRepoDir, true},
    {"Get Lock...", "rabbitvcs-lock", "lock", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Release Lock...", "rabbitvcs-unlock", "unlock", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Branch/tag...", "rabbitvcs-branch", "branch", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Switch...", "rabbitvcs-switch", "switch", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Merge...", "rabbitvcs-merge", "merge", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Export...", "rabbitvcs-export", "export", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Relocate...", "rabbitvcs-relocate", "relocate", nullptr, P::InRepoDir, true},
    {"Annotate...", "rabbitvcs-annotate", "annotate", nullptr, P::InRepoFile, true},
    {"Create Patch...", "rabbitvcs-createpatch", "createpatch", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Apply Patch...", "rabbitvcs-applypatch", "applypatch", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Properties", "rabbitvcs-properties", "properties", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Add...", "rabbitvcs-add", "add", nullptr, P::InRepoDir | P::InRepoFile, true},
    {"Add to Ignore List", "rabbitvcs-no", "ignore", nullptr,
     P::InRepoDir | P::InRepoFile, true},
    {"Checkout...", "rabbitvcs-checkout", "checkout", nullptr, P::InRepoDir | P::OutOfVcs, true},
    {"Import...", "rabbitvcs-import", "import", nullptr, P::OutOfVcs, true},
    {"Create Repository Here", "rabbitvcs-run", "create", "--vcs svn", P::OutOfVcs, true},
    {"Settings", "rabbitvcs-settings", "settings", nullptr,
     P::InRepoDir | P::InRepoFile | P::OutOfVcs, false},
    {"About", "rabbitvcs-about", "about", nullptr,
     P::InRepoDir | P::InRepoFile | P::OutOfVcs, false},
};
} // namespace

RabbitVCSPlugin::RabbitVCSPlugin(Vcs vcs, QObject *parent)
    : KVersionControlPlugin(parent)
    , m_vcs(vcs)
{
    m_menu.setTitle(vcs == Vcs::Git ? QStringLiteral("RabbitVCS Git") : QStringLiteral("RabbitVCS SVN"));
    m_menu.setIcon(QIcon::fromTheme(QStringLiteral("rabbitvcs")));

    m_actionDefs = vcs == Vcs::Git ? gitActions : svnActions;
    const int count = vcs == Vcs::Git ? int(std::size(gitActions)) : int(std::size(svnActions));

    m_actions.reserve(count);
    for (int i = 0; i < count; ++i) {
        const ActionDef &def = m_actionDefs[i];
        auto *action = new QAction(QIcon::fromTheme(QString::fromLatin1(def.icon)), QString::fromUtf8(def.text), &m_menu);
        const QString module = QString::fromLatin1(def.module);
        const QStringList extraArgs =
            def.extraArgs ? QString::fromLatin1(def.extraArgs).split(QLatin1Char(' '), Qt::SkipEmptyParts) : QStringList();
        const bool takesPaths = def.takesPaths;
        connect(action, &QAction::triggered, this, [this, module, extraArgs, takesPaths]() {
            runAction(module, extraArgs, takesPaths);
        });
        m_menu.addAction(action);
        m_actions.append(action);
    }
}

QString RabbitVCSPlugin::fileName() const
{
    return m_vcs == Vcs::Git ? QStringLiteral(".git") : QStringLiteral(".svn");
}

QString RabbitVCSPlugin::localRepositoryRoot(const QString &directory) const
{
    int exitCode = -1;
    QByteArray output;
    if (m_vcs == Vcs::Git) {
        output = runProcess(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")}, directory, &exitCode);
    } else {
        output = runProcess(QStringLiteral("svn"),
                            {QStringLiteral("info"), QStringLiteral("--show-item"), QStringLiteral("wc-root"), directory},
                            QString(),
                            &exitCode);
    }
    if (exitCode != 0) {
        return {};
    }
    return QString::fromLocal8Bit(output).trimmed();
}

bool RabbitVCSPlugin::beginRetrieval(const QString &directory)
{
    m_status.clear();
    return m_vcs == Vcs::Git ? retrieveGit(directory) : retrieveSvn(directory);
}

void RabbitVCSPlugin::endRetrieval()
{
}

bool RabbitVCSPlugin::retrieveGit(const QString &directory)
{
    int exitCode = -1;
    // Derive the root from Dolphin's own spelling of the path (not git's resolved one),
    // so entries still match when the repository is reached through a symlink.
    QString prefix = QString::fromLocal8Bit(
        runProcess(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--show-prefix")}, directory, &exitCode));
    if (exitCode != 0) {
        return false;
    }
    while (prefix.endsWith(QLatin1Char('\n')) || prefix.endsWith(QLatin1Char('/'))) {
        prefix.chop(1);
    }
    QString root = directory;
    while (root.endsWith(QLatin1Char('/'))) {
        root.chop(1);
    }
    if (!prefix.isEmpty()) {
        if (!root.endsWith(QLatin1Char('/') + prefix)) {
            return false;
        }
        root.chop(prefix.size() + 1);
    }

    const QByteArray output = runProcess(QStringLiteral("git"),
                                         {QStringLiteral("--no-optional-locks"),
                                          QStringLiteral("status"),
                                          QStringLiteral("--porcelain"),
                                          QStringLiteral("-z"),
                                          QStringLiteral("-unormal"),
                                          QStringLiteral("--ignored"),
                                          QStringLiteral("--"),
                                          QStringLiteral(".")},
                                         directory,
                                         &exitCode);
    if (exitCode != 0) {
        return false;
    }
    m_status = parseGitStatus(output, root);
    return true;
}

bool RabbitVCSPlugin::retrieveSvn(const QString &directory)
{
    int exitCode = -1;
    const QByteArray output = runProcess(QStringLiteral("svn"),
                                         {QStringLiteral("status"), QStringLiteral("--no-ignore")},
                                         directory,
                                         &exitCode);
    if (exitCode != 0) {
        return false;
    }
    m_status = parseSvnStatus(output, directory);
    return true;
}
KVersionControlPlugin::ItemVersion RabbitVCSPlugin::itemVersion(const KFileItem &item) const
{
    const QString path = item.localPath();
    const auto exact = m_status.constFind(path);
    if (exact != m_status.cend()) {
        return exact.value();
    }

    // Everything below an unversioned or ignored directory shares that state.
    QString parent = path;
    int slash = -1;
    while ((slash = parent.lastIndexOf(QLatin1Char('/'))) > 0) {
        parent.truncate(slash);
        const auto it = m_status.constFind(parent);
        if (it != m_status.cend() && (it.value() == UnversionedVersion || it.value() == IgnoredVersion)) {
            return it.value();
        }
    }

    if (item.isDir()) {
        const QString prefix = path + QLatin1Char('/');
        for (auto it = m_status.cbegin(); it != m_status.cend(); ++it) {
            if (!it.key().startsWith(prefix)) {
                continue;
            }
            switch (it.value()) {
            case ConflictingVersion:
                return ConflictingVersion;
            case LocallyModifiedVersion:
            case LocallyModifiedUnstagedVersion:
            case AddedVersion:
            case RemovedVersion:
            case MissingVersion:
                return LocallyModifiedVersion;
            default:
                break;
            }
        }
    }

    return NormalVersion;
}

QList<QAction *> RabbitVCSPlugin::versionControlActions(const KFileItemList &items) const
{
    m_contextItems = items;
    const bool directory = items.isEmpty() || (items.count() == 1 && items.first().isDir());
    showActions(directory ? InRepoDir : InRepoFile);
    return {m_menu.menuAction()};
}

QList<QAction *> RabbitVCSPlugin::outOfVersionControlActions(const KFileItemList &items) const
{
    if (items.count() != 1 || !items.first().isDir() || !items.first().isWritable() || !items.first().isLocalFile()) {
        return {};
    }
    m_contextItems = items;
    showActions(OutOfVcs);
    return {m_menu.menuAction()};
}

void RabbitVCSPlugin::showActions(quint8 context) const
{
    for (int i = 0; i < m_actions.size(); ++i) {
        m_actions.at(i)->setVisible(m_actionDefs[i].applies & context);
    }
}

void RabbitVCSPlugin::runAction(const QString &module, const QStringList &extraArgs, bool takesPaths)
{
    QStringList args{module};
    args << extraArgs;
    if (takesPaths) {
        for (const KFileItem &item : std::as_const(m_contextItems)) {
            args << item.localPath();
        }
    }

    // Refresh only when the command exits, so the new status is visible.
    auto *process = new QProcess(this);
    connect(process, &QProcess::finished, this, [this, process]() {
        Q_EMIT itemVersionsChanged();
        process->deleteLater();
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            Q_EMIT errorMessage(QStringLiteral("Could not run the rabbitvcs command: %1").arg(process->errorString()));
            process->deleteLater();
        }
    });
    process->start(QStringLiteral("rabbitvcs"), args);
}
