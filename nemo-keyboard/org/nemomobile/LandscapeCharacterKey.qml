import QtQuick 2.0
import "KeyboardUiConstants.js" as UI

CharacterKey {
    width: keyArea.width / 10
    height: keyArea.height / 4

    topPadding: UI.landscapeVerticalPadding
    bottomPadding: UI.landscapeVerticalPadding
    leftPadding: UI.landscapeHorizontalPadding
    rightPadding: UI.landscapeHorizontalPadding
}
