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



#ifndef MIMTESKEYAREA_H
#define MIMTESKEYAREA_H

#include "mimabstractkeyarea.h"

class MImAbstractKey;

//! \brief MImTestKeyArea provides minimal implementation of MImAbstractKeyArea for unit testing
class MImTestKeyArea
    : public MImAbstractKeyArea
{
    Q_OBJECT
    Q_DISABLE_COPY(MImTestKeyArea)

public:
    //! \brief Constructor
    //! \param section section that is shown by this key area
    //! \param usePopup whether popup should be used
    //! \param parent key area's parent
    explicit MImTestKeyArea(const LayoutData::SharedLayoutSection &section,
                                bool usePopup = false,
                                QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~MImTestKeyArea();

    //! \reimp
    virtual QList<const MImAbstractKey *> keys() const;
    virtual MImAbstractKey * findKey(const QString &);
    virtual MImAbstractKey *keyAt(const QPoint &) const;
    virtual void updateKeyGeometries(int);
    virtual void setContentType(M::TextContentType);
    //! \reimp_end

public slots:
    //! \reimp
    virtual void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);
    //! \reimp_end

public:
    int setKeyOverridesCalls; //!< Amount of calls to setKeyOverrides()
    QMap<QString, QSharedPointer<MKeyOverride> > setKeyOverridesParam; //!< Parameter used for last call to setKeyOverrides()
};

#endif

