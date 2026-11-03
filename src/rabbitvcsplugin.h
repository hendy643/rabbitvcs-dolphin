/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RABBITVCSPLUGIN_H
#define RABBITVCSPLUGIN_H

#include <KFileItem>
#include <KVersionControlPlugin>

#include <QHash>
#include <QMenu>

/**
 * Dolphin VCS plugin shared by the Git and Subversion variants.
 *
 * The status is read with the native git/svn tools; the menu actions are
 * handed to the rabbitvcs command line client, which opens the usual UI.
 */
class RabbitVCSPlugin : public KVersionControlPlugin
{
public:
    enum class Vcs {
        Git,
        Svn,
    };

    explicit RabbitVCSPlugin(Vcs vcs, QObject *parent = nullptr);

    QString fileName() const override;
    QString localRepositoryRoot(const QString &directory) const override;
    bool beginRetrieval(const QString &directory) override;
    void endRetrieval() override;
    ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

    enum Applies : quint8 {
        InRepoDir = 1,
        InRepoFile = 2,
        OutOfVcs = 4,
    };

    struct ActionDef {
        const char *text;
        const char *icon;
        const char *module;
        const char *extraArgs;
        quint8 applies;
        bool takesPaths;
    };

private:
    bool retrieveGit(const QString &directory);
    bool retrieveSvn(const QString &directory);
    void showActions(quint8 context) const;
    void runAction(const QString &module, const QStringList &extraArgs, bool takesPaths);

    const Vcs m_vcs;
    QHash<QString, ItemVersion> m_status;
    QMenu m_menu;
    QList<QAction *> m_actions;
    const ActionDef *m_actionDefs;
    mutable KFileItemList m_contextItems;
};

#endif
