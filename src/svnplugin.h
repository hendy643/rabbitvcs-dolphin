/*
 * SPDX-FileCopyrightText: 2026 RabbitVCS contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RABBITVCSSVNPLUGIN_H
#define RABBITVCSSVNPLUGIN_H

#include "rabbitvcsplugin.h"

class RabbitVCSSvnPlugin : public RabbitVCSPlugin
{
    Q_OBJECT

public:
    explicit RabbitVCSSvnPlugin(QObject *parent = nullptr, const QList<QVariant> &args = {})
        : RabbitVCSPlugin(Vcs::Svn, parent)
    {
        Q_UNUSED(args);
    }
};

#endif
