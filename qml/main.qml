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

        headBar.rightContent: QQC.Switch {
            text: backend.panic
                  ? qsTr("Disable Lockdown")
                  : qsTr("Enable Lockdown")

            checked: backend.panic
            onClicked: backend.setPanic(!backend.panic)

            QQC.ToolTip {
                visible: parent.hovered
                text: qsTr("Immediately block all incoming and outgoing connections.")
                delay: 500
            }
        }

        Maui.ScrollColumn {
            id: scrollColumn
            anchors.fill: parent
            spacing: Maui.Style.space.big

            Maui.SectionGroup {
                title: qsTr("Behavior")
                description: qsTr("Network visibility and translation.")

                Maui.FlexSectionItem {
                    label1.text: qsTr("Masquerading")
                    label2.text: qsTr("Enable Network Address Translation (NAT) for outbound traffic.")

                    QQC.Switch {
                        checked: backend.masquerade
                        onToggled: backend.setMasquerade(checked, root.currentZone)
                    }
                }

                Maui.FlexSectionItem {
                    label1.text: qsTr("Stealth Mode")
                    label2.text: qsTr("Silently drop all uninvited packets (Target: DROP).")
                    QQC.Switch {
                        checked: backend.stealthMode
                        onToggled: backend.setStealthMode(checked, root.currentZone)
                    }
                }

                Maui.FlexSectionItem {
                    label1.text: qsTr("Strict ICMP")
                    label2.text: qsTr("Allow critical ICMP types required for a healthy connection, while still blocking everything else.")
                    QQC.Switch {        
                        checked: backend.strictIcmp
                        onToggled: backend.setStrictIcmp(checked, root.currentZone)
                    }
                }

                Maui.FlexSectionItem {
                    label1.text: qsTr("Log Denied")
                    label2.text: qsTr("Control logging for packets that are rejected or dropped.")

                    QQC.Switch {
                        checked: backend.logDenied
                        onToggled: backend.setLogDenied(checked)
                    }
                }
            }

            Maui.SectionGroup {
                title: qsTr("Services")
                description: qsTr("Allow predefined network services.")

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

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

            Maui.SectionGroup {
                Layout.fillWidth: true
                title: qsTr("Ports")
                description: qsTr("Manually open specific ports.")

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

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

            Maui.SectionGroup {
                Layout.fillWidth: true
                title: qsTr("Sources")
                description: qsTr("Trust traffic from specific IPs/Subnets.")

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

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

            Maui.SectionGroup {
                Layout.fillWidth: true
                title: qsTr("Port Forwarding")
                description: qsTr("Redirect network traffic from one port to another port.")

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    rowSpacing: 10
                    columnSpacing: 10

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
