/*
 * SPDX-FileCopyrightText: (C) 2021 Wangrui <Wangrui@jingos.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import org.kde.mauikit 1.0 as Maui
import org.kde.kirigami 2.7 as Kirigami

Maui.Editor {
    id: control
    anchors.fill: parent
    footBar.visible: false
    body.readOnly: true
    document.enableSyntaxHighlighting: true
    Kirigami.Theme.backgroundColor: "transparent"
    anchors{
        left: parent.left
        leftMargin: 80
        right: parent.right
        rightMargin: 80
    }

    fileUrl: currentUrl
    Connections {
        target: control.document

        function onLoaded() {
            infoModel.insert(0, {key:"Length", value: control.body.length.toString()})
            infoModel.insert(0, {key:"Line Count", value: control.body.lineCount.toString()})
        }
    }
}
