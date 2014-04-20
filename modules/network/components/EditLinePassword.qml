import QtQuick 2.1
import Deepin.Widgets 1.0

BaseEditLine {
    id: root
    
    property int echoMode
    
    rightLoader.sourceComponent: DTextInput{
        id: passwordArea
        width: valueWidth
        echoMode: root.echoMode
        text: getKey()
        onTextChanged: {
            root.value = text
            setKey()
        }
    }
    
}

