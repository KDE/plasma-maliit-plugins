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
    M_STYLE_ATTRIBUTE(QColor, dimmingColor, dimmingColor)
    M_STYLE_ATTRIBUTE(qreal, initialDimmingOpacity, InitialDimmingOpacity)
    M_STYLE_ATTRIBUTE(QEasingCurve, catchingUpAnimationCurve, CatchingUpAnimationCurve)
    M_STYLE_ATTRIBUTE(int, catchingUpAnimationMaximumDuration, CatchingUpAnimationMaximumDuration)
    M_STYLE_ATTRIBUTE(qreal, catchingUpAnimationSpeed, CatchingUpAnimationSpeed)
    M_STYLE_ATTRIBUTE(qreal, minimumMovementThreshold, MinimumMovementThreshold)
    M_STYLE_ATTRIBUTE(qreal, suddenMovementThreshold, SuddenMovementThreshold)

    M_STYLE_ATTRIBUTE(qreal, outgoingFromOpacity, OutgoingFromOpacity)
    M_STYLE_ATTRIBUTE(qreal, outgoingToOpacity, OutgoingToOpacity)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityStartProgress, OutgoingOpacityStartProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityEndProgress, OutgoingOpacityEndProgress)
};

class LayoutPannerStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(LayoutPannerStyle)
};

#endif
