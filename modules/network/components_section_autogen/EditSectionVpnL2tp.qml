// This file is automatically generated, please don't edit manully.
import QtQuick 2.1
import Deepin.Widgets 1.0
import "../components"

BaseEditSection {
    id: sectionVpnL2tp
    section: "vpn-l2tp"
    
    header.sourceComponent: EditDownArrowHeader{
        text: dsTr("VPN")
    }

    content.sourceComponent: Column { 
        EditLineTextInput {
            id: lineGateway
            key: "gateway"
            text: dsTr("Gateway")
        }
        EditLineTextInput {
            id: lineUser
            key: "user"
            text: dsTr("Username")
        }
        EditLineComboBox {
            id: linePasswordFlags
            key: "password-flags"
            text: dsTr("Ask for Pwd")
        }
        EditLinePasswordInput {
            id: linePassword
            key: "password"
            text: dsTr("Password")
        }
        EditLineTextInput {
            id: lineDomain
            key: "domain"
            text: dsTr("Domain")
        }
    }
}
