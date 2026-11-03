/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "rabbitvcsplugin.h"

#include <KFileItem>
#include <KIO/CommandLauncherJob>
#include <KJob>

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
    if (!process.waitForFinished(60000)) {
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

const RabbitVCSPlugin::ActionDef gitActions[] = {
    {"Update", "rabbitvcs-update", "update", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Commit...", "rabbitvcs-commit", "commit", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Push", "go-up", "push", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Show Changes...", "rabbitvcs-changes", "changes", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Show Log", "rabbitvcs-show_log", "log", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Edit Conflicts", "rabbitvcs-editconflicts", "editconflicts", nullptr, RabbitVCSPlugin::InRepoFile, true},
    {"Rename...", "rabbitvcs-rename", "rename", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Delete", "rabbitvcs-delete", "delete", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Revert...", "rabbitvcs-revert", "revert", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Clean", "rabbitvcs-clear", "clean", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Reset", "rabbitvcs-reset", "reset", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Branches", "rabbitvcs-branch", "branches", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Tags", "rabbitvcs-branch", "tags", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Remotes", "rabbitvcs-checkmods", "remotes", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Export...", "rabbitvcs-export", "export", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Merge...", "rabbitvcs-merge", "merge", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Annotate...", "rabbitvcs-annotate", "annotate", nullptr, RabbitVCSPlugin::InRepoFile, true},
    {"Apply Patch...", "rabbitvcs-applypatch", "applypatch", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Create Patch...", "rabbitvcs-createpatch", "createpatch", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Add...", "rabbitvcs-add", "add", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Add to Ignore List", "rabbitvcs-no", "ignore", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Checkout...", "rabbitvcs-checkout", "checkout", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::OutOfVcs, true},
    {"Create Repository Here", "rabbitvcs-run", "create", "--vcs git", RabbitVCSPlugin::OutOfVcs, true},
    {"Clone...", "rabbitvcs-checkout", "clone", nullptr, RabbitVCSPlugin::OutOfVcs, true},
    {"Settings", "rabbitvcs-settings", "settings", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile | RabbitVCSPlugin::OutOfVcs, false},
    {"About", "rabbitvcs-about", "about", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile | RabbitVCSPlugin::OutOfVcs, false},
};

const RabbitVCSPlugin::ActionDef svnActions[] = {
    {"Update", "rabbitvcs-update", "update", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Commit...", "rabbitvcs-commit", "commit", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Show Changes...", "rabbitvcs-changes", "changes", nullptr, RabbitVCSPlugin::InRepoFile, true},
    {"Show Log", "rabbitvcs-show_log", "log", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Repo-browser", "rabbitvcs-drive", "browser", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Check for Modifications", "rabbitvcs-checkmods", "checkmods", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Edit Conflicts", "rabbitvcs-editconflicts", "editconflicts", nullptr, RabbitVCSPlugin::InRepoFile, true},
    {"Resolve...", "rabbitvcs-resolve", "markresolved", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Update to Revision...", "rabbitvcs-update", "updateto", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Rename...", "rabbitvcs-rename", "rename", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Delete", "rabbitvcs-delete", "delete", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Revert...", "rabbitvcs-revert", "revert", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Clean up...", "rabbitvcs-cleanup", "cleanup", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Get Lock...", "rabbitvcs-lock", "lock", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Release Lock...", "rabbitvcs-unlock", "unlock", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Branch/tag...", "rabbitvcs-branch", "branch", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Switch...", "rabbitvcs-switch", "switch", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Merge...", "rabbitvcs-merge", "merge", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Export...", "rabbitvcs-export", "export", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Relocate...", "rabbitvcs-relocate", "relocate", nullptr, RabbitVCSPlugin::InRepoDir, true},
    {"Annotate...", "rabbitvcs-annotate", "annotate", nullptr, RabbitVCSPlugin::InRepoFile, true},
    {"Create Patch...", "rabbitvcs-createpatch", "createpatch", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Apply Patch...", "rabbitvcs-applypatch", "applypatch", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Properties", "rabbitvcs-properties", "properties", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Add...", "rabbitvcs-add", "add", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Add to Ignore List", "rabbitvcs-no", "ignore", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile, true},
    {"Checkout...", "rabbitvcs-checkout", "checkout", nullptr, RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::OutOfVcs, true},
    {"Import...", "rabbitvcs-import", "import", nullptr, RabbitVCSPlugin::OutOfVcs, true},
    {"Create Repository Here", "rabbitvcs-run", "create", "--vcs svn", RabbitVCSPlugin::OutOfVcs, true},
    {"Settings", "rabbitvcs-settings", "settings", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile | RabbitVCSPlugin::OutOfVcs, false},
    {"About", "rabbitvcs-about", "about", nullptr,
     RabbitVCSPlugin::InRepoDir | RabbitVCSPlugin::InRepoFile | RabbitVCSPlugin::OutOfVcs, false},
};
} // namespace

RabbitVCSPlugin::RabbitVCSPlugin(Vcs vcs, QObject *parent)
    : KVersionControlPlugin(parent)
    , m_vcs(vcs)
{
    m_menu.setTitle(vcs == Vcs::Git ? QStringLiteral("RabbitVCS Git") : QStringLiteral("RabbitVCS SVN"));
    m_menu.setIcon(QIcon::fromTheme(QStringLiteral("rabbitvcs")));

    const ActionDef *defs = vcs == Vcs::Git ? gitActions : svnActions;
    const int count = vcs == Vcs::Git ? int(std::size(gitActions)) : int(std::size(svnActions));

    m_actionDefs.reserve(count);
    m_actions.reserve(count);
    for (int i = 0; i < count; ++i) {
        const ActionDef &def = defs[i];
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
        m_actionDefs.append(def);
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
    const QString root = QString::fromLocal8Bit(
                             runProcess(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")}, directory, &exitCode))
                             .trimmed();
    if (exitCode != 0 || root.isEmpty()) {
        return false;
    }

    const QByteArray output = runProcess(QStringLiteral("git"),
                                         {QStringLiteral("--no-optional-locks"),
                                          QStringLiteral("status"),
                                          QStringLiteral("--porcelain"),
                                          QStringLiteral("-z"),
                                          QStringLiteral("-u"),
                                          QStringLiteral("--ignored")},
                                         directory,
                                         &exitCode);
    if (exitCode != 0) {
        return false;
    }

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
        if (!path.startsWith(directory)) {
            continue;
        }

        ItemVersion state = NormalVersion;
        switch (x) {
        case '!':
            state = IgnoredVersion;
            break;
        case '?':
            state = UnversionedVersion;
            break;
        case 'A':
        case 'C':
            state = AddedVersion;
            break;
        case 'D':
            state = RemovedVersion;
            break;
        case 'M':
        case 'R':
            state = LocallyModifiedVersion;
            break;
        case 'U':
            state = ConflictingVersion;
            break;
        default:
            break;
        }
        if (y == 'D' || y == 'M') {
            state = LocallyModifiedUnstagedVersion;
        }
        if (x == 'U' || y == 'U' || (x == 'A' && y == 'A') || (x == 'D' && y == 'D')) {
            state = ConflictingVersion;
        }
        if (state != NormalVersion) {
            m_status.insert(path, state);
        }
    }
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

        ItemVersion state = NormalVersion;
        switch (code) {
        case '?':
        case '~':
            state = UnversionedVersion;
            break;
        case 'I':
            state = IgnoredVersion;
            break;
        case 'A':
            state = AddedVersion;
            break;
        case 'D':
            state = RemovedVersion;
            break;
        case 'M':
        case 'R':
            state = LocallyModifiedVersion;
            break;
        case 'C':
            state = ConflictingVersion;
            break;
        case '!':
            state = MissingVersion;
            break;
        case ' ':
            if (line.at(1) == 'M') {
                state = LocallyModifiedVersion;
            }
            break;
        default:
            break;
        }
        if (state == NormalVersion) {
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
        m_status.insert(path, state);
    }
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
        m_actions.at(i)->setVisible(m_actionDefs.at(i).applies & context);
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

    auto *job = new KIO::CommandLauncherJob(QStringLiteral("rabbitvcs"), args, this);
    connect(job, &KJob::result, this, [this](KJob *finishedJob) {
        if (finishedJob->error()) {
            Q_EMIT errorMessage(QStringLiteral("Could not run the rabbitvcs command: %1").arg(finishedJob->errorString()));
        } else {
            Q_EMIT itemVersionsChanged();
        }
    });
    job->start();
}
