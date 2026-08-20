import QtQuick
import QtQuick.Controls 2.15 as QQC
import QtQuick.Layouts
import org.mauikit.controls as Maui
import org.nitrux.firewall 1.0

Maui.ApplicationWindow {
    id: root

    readonly property string statusText: backend.state === "running"
        ? (backend.panic ? qsTr("Lockdown") : "")
        : qsTr("Error")

    readonly property string currentZone: profileCombo.currentIndex >= 0
                                          ? profileCombo.currentText.toLowerCase()
                                          : ""

    readonly property string profileText: profileCombo.currentIndex >= 0
                                          ? profileCombo.currentText
                                          : qsTr("No Profile")

    readonly property var consolidatedRuleModel: {
        var services = backend.services || []
        var ports = backend.ports || []
        var sources = backend.sources || []
        var forwardRules = backend.forwardRules || []

        var s = services.map(val => ({ "type": "service", "value": val }))
        var p = ports.map(val => ({ "type": "port", "value": val }))
        var src = sources.map(val => ({ "type": "source", "value": val }))
        var f = forwardRules.map(val => ({ "type": "forward", "value": val }))

        return s.concat(p).concat(src).concat(f)
    }

    title: "Zone — " + profileText + (statusText ? " (" + statusText + ")" : "")

    color: "transparent"
    background: null

    FirewallBackend {
        id: backend
        onOperationError: (error) => {
            console.warn("Firewall Error: " + error)
        }
    }

    Maui.WindowBlur {
        view: root
        geometry: Qt.rect(0, 0, root.width, root.height)
        windowRadius: Maui.Style.radiusV
        enabled: true
    }

    Rectangle {
        anchors.fill: parent
        color: Maui.Theme.backgroundColor
        opacity: 0.76
        radius: Maui.Style.radiusV
        border.color: Qt.rgba(1, 1, 1, 0)
        border.width: 1
    }

    Maui.Page {
        anchors.fill: parent
        flickable: scrollColumn.flickable
        background: null
        headerMargins: Maui.Style.contentMargins

        Component.onCompleted: {
            backend.refresh("")

            var currentDefault = backend.defaultZone

            if (currentDefault.length > 0) {
                var displayZone = currentDefault.charAt(0).toUpperCase() + currentDefault.slice(1)
                var zoneIndex = profileCombo.model.indexOf(displayZone)
                if (zoneIndex !== -1) {
                    profileCombo.currentIndex = zoneIndex
                }
            }
        }

        headBar.leftContent: [
            QQC.Label {
                text: qsTr("Profile")
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            },
            QQC.ComboBox {
                id: profileCombo
                implicitWidth: 200
                currentIndex: -1
                displayText: currentIndex === -1 ? qsTr("Select Profile") : currentText
                model: ["Public", "Home", "Block", "Work", "Internal", "External", "Dmz", "Trusted", "Drop"]
                onActivated: {
                    backend.changeDefaultZone(currentText.toLowerCase())
                }
            }
        ]

        headBar.rightContent: [
            Maui.ToolButtonMenu {
                icon.name: "overflow-menu"

                QQC.MenuItem {
                    text: qsTr("About")
                    icon.name: "documentinfo"
                    onTriggered: Maui.App.aboutDialog()
                }
            }
        ]

        Maui.ScrollColumn {
            id: scrollColumn
            anchors.fill: parent
            spacing: Maui.Style.space.big

            Maui.SectionHeader {
                Layout.fillWidth: true
                text1: qsTr("Zone Configuration")
                text2: qsTr("Manage global switches and add traffic rules.")
                label2.wrapMode: Text.Wrap
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: behaviorLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: behaviorLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Firewall Behavior")
                        text2: qsTr("Configure how this profile handles network traffic.")
                        label2.wrapMode: Text.Wrap
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: qsTr("Lockdown")
                        label2.text: qsTr("Immediately block all incoming and outgoing connections.")
                        label2.wrapMode: Text.Wrap

                        QQC.Switch {
                            checked: backend.panic
                            onClicked: backend.setPanic(!backend.panic)
                        }
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: qsTr("Masquerading")
                        label2.text: qsTr("Enable Network Address Translation (NAT) for outbound traffic.")
                        label2.wrapMode: Text.Wrap

                        QQC.Switch {
                            checked: backend.masquerade
                            onToggled: backend.setMasquerade(checked, root.currentZone)
                        }
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: qsTr("Stealth Mode")
                        label2.text: qsTr("Silently drop all uninvited packets (Target: DROP).")
                        label2.wrapMode: Text.Wrap
                        QQC.Switch {
                            checked: backend.stealthMode
                            onToggled: backend.setStealthMode(checked, root.currentZone)
                        }
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: qsTr("Strict ICMP")
                        label2.text: qsTr("Allow critical ICMP types required for a healthy connection, while still blocking everything else.")
                        label2.wrapMode: Text.Wrap
                        QQC.Switch {
                            checked: backend.strictIcmp
                            onToggled: backend.setStrictIcmp(checked, root.currentZone)
                        }
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: qsTr("Log Denied")
                        label2.text: qsTr("Control logging for packets that are rejected or dropped.")
                        label2.wrapMode: Text.Wrap

                        QQC.Switch {
                            checked: backend.logDenied
                            onToggled: backend.setLogDenied(checked)
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: servicesLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: servicesLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Services")
                        text2: qsTr("Allow predefined network services.")
                        label2.wrapMode: Text.Wrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.medium

                        QQC.ComboBox {
                            id: serviceCombo
                            Layout.fillWidth: true
                            model: backend.knownServices
                        }

                        QQC.Button {
                            Layout.fillWidth: false
                            display: QQC.AbstractButton.IconOnly
                            icon.name: "list-add"

                            onClicked: {
                                backend.addService(serviceCombo.currentText, root.currentZone)
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: portsLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: portsLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Ports")
                        text2: qsTr("Manually open specific ports.")
                        label2.wrapMode: Text.Wrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.medium

                        Maui.TextField {
                            id: portText
                            Layout.fillWidth: true
                            placeholderText: qsTr("Port (e.g. 8080)")
                            inputMethodHints: Qt.ImhDigitsOnly
                        }

                        QQC.ComboBox {
                            id: protocolCombo
                            Layout.preferredWidth: 100
                            model: ["TCP", "UDP"]
                        }

                        QQC.Button {
                            display: QQC.AbstractButton.IconOnly
                            icon.name: "list-add"

                            onClicked: {
                                backend.addPort(portText.text,
                                                protocolCombo.currentText.toLowerCase(),
                                                root.currentZone)
                                portText.text = ""
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: sourcesLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: sourcesLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Sources")
                        text2: qsTr("Trust traffic from specific IPs/Subnets.")
                        label2.wrapMode: Text.Wrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.medium

                        Maui.TextField {
                            id: sourceText
                            Layout.fillWidth: true
                            placeholderText: qsTr("IP or CIDR (e.g. 192.168.1.5)")
                        }

                        QQC.Button {
                            display: QQC.AbstractButton.IconOnly
                            icon.name: "list-add"

                            onClicked: {
                                backend.addSource(sourceText.text, root.currentZone)
                                sourceText.text = ""
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: forwardingLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: forwardingLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Port Forwarding")
                        text2: qsTr("Redirect network traffic from one port to another port.")
                        label2.wrapMode: Text.Wrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: Maui.Style.space.medium
                        columnSpacing: Maui.Style.space.medium

                        Maui.TextField {
                            id: sourcePortText
                            Layout.fillWidth: true
                            placeholderText: qsTr("Source Port")
                        }

                        QQC.ComboBox {
                            id: forwardProtocolCombo
                            Layout.fillWidth: true
                            model: ["TCP", "UDP"]
                        }

                        Maui.TextField {
                            id: destPortText
                            Layout.fillWidth: true
                            placeholderText: qsTr("Dest Port")
                        }

                        Maui.TextField {
                            id: destIPText
                            Layout.fillWidth: true
                            placeholderText: qsTr("Dest IP (Optional)")
                        }

                        QQC.Button {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignRight
                            display: QQC.AbstractButton.IconOnly
                            icon.name: "list-add"

                            onClicked: {
                                backend.addForwardRule(
                                    sourcePortText.text,
                                    forwardProtocolCombo.currentText.toLowerCase(),
                                    destPortText.text,
                                    destIPText.text,
                                    root.currentZone
                                )
                                sourcePortText.text = ""
                                destPortText.text = ""
                                destIPText.text = ""
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: rulesLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: rulesLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: qsTr("Active Rules")
                        text2: qsTr("Current rules configured for this profile.")
                        label2.wrapMode: Text.Wrap
                    }

                    Maui.ListBrowser {
                        id: listBrowser
                        Layout.fillWidth: true
                        Layout.preferredHeight: 300
                        clip: true

                        model: root.consolidatedRuleModel

                        holder.visible: count === 0
                        holder.emoji: "preferences-system-network-connection"
                        holder.title: qsTr("Active Rules")
                        holder.body: qsTr("No rules configured for this profile.")

                        delegate: Maui.ListDelegate {
                            width: ListView.view.width

                            iconName: modelData.type === "service" ? "applications-other" :
                                      modelData.type === "port" ? "network-wired" :
                                      modelData.type === "source" ? "preferences-system-network" :
                                      "preferences-system-network-sharing"

                            label: modelData.value

                            QQC.Button {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                display: QQC.AbstractButton.IconOnly
                                icon.name: "edit-delete"

                                onClicked: {
                                    var val = modelData.value
                                    var type = modelData.type
                                    var zone = root.currentZone

                                    if (type === "service") {
                                        backend.removeService(val, zone)
                                    } else if (type === "port") {
                                        var parts = val.split("/")
                                        if (parts.length === 2) {
                                            backend.removePort(parts[0], parts[1], zone)
                                        }
                                    } else if (type === "source") {
                                        backend.removeSource(val, zone)
                                    } else if (type === "forward") {
                                        var getValue = function(k) {
                                            var match = val.match(new RegExp(k + "=([^:]*)"))
                                            return match ? match[1] : ""
                                        }
                                        backend.removeForwardRule(
                                            getValue("port"),
                                            getValue("proto"),
                                            getValue("toport"),
                                            getValue("toaddr"),
                                            zone
                                        )
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
