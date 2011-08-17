#ifndef LAYOUTPANNERSTYLE_H
#define LAYOUTPANNERSTYLE_H

#include <MWidgetStyle>
#include <QEasingCurve>
#include <QColor>

class LayoutPannerStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(LayoutPannerStyle)

    M_STYLE_ATTRIBUTE(int, commitThreshold, CommitThreshold)
    M_STYLE_ATTRIBUTE(QEasingCurve, panningAnimationCurve, PanningAnimationCurve)
    M_STYLE_ATTRIBUTE(int, panningAnimationDuration, PanningAnimationDuration)
    M_STYLE_ATTRIBUTE(qreal, initialDimmingOpacity, InitialDimmingOpacity)
    M_STYLE_ATTRIBUTE(int, catchingUpAnimationMaximumDuration, CatchingUpAnimationMaximumDuration)
    M_STYLE_ATTRIBUTE(QEasingCurve, layoutsCatchingUpAnimationCurve, LayoutsCatchingUpAnimationCurve)
    M_STYLE_ATTRIBUTE(qreal, layoutsCatchingUpAnimationDurationMultiplier, LayoutsCatchingUpAnimationDurationMultiplier)
    M_STYLE_ATTRIBUTE(QEasingCurve, notificationsCatchingUpAnimationCurve, NotificationsCatchingUpAnimationCurve)
    M_STYLE_ATTRIBUTE(qreal, notificationsCatchingUpAnimationDurationMultiplier, NotificationsCatchingUpAnimationDurationMultiplier)
    M_STYLE_ATTRIBUTE(qreal, notificationsCatchingUpAnimationPause, NotificationsCatchingUpAnimationPause)
    M_STYLE_ATTRIBUTE(qreal, minimumMovementThreshold, MinimumMovementThreshold)
    M_STYLE_ATTRIBUTE(qreal, suddenMovementThreshold, SuddenMovementThreshold)

    M_STYLE_ATTRIBUTE(qreal, outgoingLayoutFromOpacity, OutgoingLayoutFromOpacity)
    M_STYLE_ATTRIBUTE(qreal, outgoingLayoutToOpacity, OutgoingLayoutToOpacity)
    M_STYLE_ATTRIBUTE(qreal, outgoingLayoutOpacityStartProgress, OutgoingLayoutOpacityStartProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingLayoutOpacityEndProgress, OutgoingLayoutOpacityEndProgress)

    M_STYLE_ATTRIBUTE(qreal, incomingLayoutFromOpacity, IncomingLayoutFromOpacity)
    M_STYLE_ATTRIBUTE(qreal, incomingLayoutToOpacity, IncomingLayoutToOpacity)
};

class LayoutPannerStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(LayoutPannerStyle)
};

#endif
