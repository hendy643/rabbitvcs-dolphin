/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RABBITVCSGITPLUGIN_H
#define RABBITVCSGITPLUGIN_H

#include "rabbitvcsplugin.h"

class RabbitVCSGitPlugin : public RabbitVCSPlugin
{
    Q_OBJECT

public:
    explicit RabbitVCSGitPlugin(QObject *parent = nullptr, const QList<QVariant> &args = {})
        : RabbitVCSPlugin(Vcs::Git, parent)
    {
        Q_UNUSED(args);
    }
};

#endif
