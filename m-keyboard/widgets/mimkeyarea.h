/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef MIMKEYAREA_H
#define MIMKEYAREA_H

#include "mimabstractkeyarea.h"
#include "mimkey.h"

#include <QSize>
#include <QFontMetrics>
#include <QSharedPointer>

class MImKeyAreaPrivate;

//! \brief MImKeyArea reimplements MImAbstractKeyArea and is optimized for drawing.
class MImKeyArea
    : public MImAbstractKeyArea
{
    Q_OBJECT
    Q_DISABLE_COPY(MImKeyArea)

public:
    //! \brief Contructor, see \a MImAbstractKeyArea
    explicit MImKeyArea(const LayoutData::SharedLayoutSection &section,
                        bool usePopup = false,
                        QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~MImKeyArea();

    //! \reimp
    virtual void modifiersChanged(bool shift,
                                  const QChar &accent = QChar());
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *view);
    virtual QRectF boundingRect() const;
    virtual void setShiftState(ModifierState newShiftState);
    virtual QList<const MImAbstractKey *> keys() const;
    virtual MImAbstractKey * findKey(const QString &id);
    virtual void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);
    virtual void setContentType(M::TextContentType type);
    virtual void setToggleKeyState(bool on);
    virtual void setComposeKeyState(bool isComposing);
    virtual void resetActiveKeys();
    //! \reimp_end

private slots:
    void updateKeyAttributes(const QString &, MKeyOverride::KeyOverrideAttributes);

protected:
    //! \reimp
    virtual QSizeF sizeHint(Qt::SizeHint which,
                            const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view);
    virtual void updateKeyGeometries(int availableWidth);
    virtual MImAbstractKey *keyAt(const QPoint &pos) const;
    virtual void onThemeChangeCompleted();
    virtual void applyStyle();
    //! \reimp_end

    //! \brief Release given \a key if it is required by current attribure override
    void releaseKey(MImKey *key);

private:
    Q_DECLARE_PRIVATE(MImKeyArea);
    MImKeyAreaPrivate * const d_ptr;

#ifdef UNIT_TEST
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_KeyEventHandler;
    friend class Ut_SymbolView;
    friend class Bm_MImAbstractKeyArea; //benchmarks
    friend class Bm_Painting;
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // MIMKEYAREA_H
