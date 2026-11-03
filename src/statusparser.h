/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STATUSPARSER_H
#define STATUSPARSER_H

#include <KVersionControlPlugin>

#include <QByteArray>
#include <QHash>
#include <QString>

using StatusMap = QHash<QString, KVersionControlPlugin::ItemVersion>;

// `git status --porcelain -z` output; `root` is the repository root without trailing slash.
StatusMap parseGitStatus(const QByteArray &output, const QString &root);
// `svn status` output; `directory` is the directory it ran in (with trailing slash).
StatusMap parseSvnStatus(const QByteArray &output, const QString &directory);

#endif
