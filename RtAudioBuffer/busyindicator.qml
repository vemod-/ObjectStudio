import QtQuick 2.4

    BusyIndicator {
        style: BusyIndicatorStyle {
            indicator: Image {
                visible: control.running
                source: "spinner.png"
                RotationAnimator on rotation {
                    running: control.running
                    loops: Animation.Infinite
                    duration: 2000
                    from: 0 ; to: 360
                }
            }
        }
    }
