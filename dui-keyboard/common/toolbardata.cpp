/* * This file is part of dui-keyboard *
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



#include "toolbardata.h"

#include <QFile>
#include <QFileInfo>
#include <QDomDocument>
#include <QDebug>

namespace
{
    const QString ToolbarConfigurationPath = "/usr/share/dui/imtoolbars/";
    const QString ImTagToolbar               = QString("toolbar");
    const QString ImTagButton                = QString("button");
    const QString ImTagActions               = QString("actions");
    const QString ImTagName                  = QString("name");
    const QString ImTagGroup                 = QString("group");
    const QString ImTagPriority              = QString("priority");
    const QString ImTagOrientation           = QString("orientation");
    const QString ImTagShowOn                = QString("showon");
    const QString ImTagHideOn                = QString("hideon");
    const QString ImTagAlignment             = QString("alignment");
    const QString ImTagIcon                  = QString("icon");
    const QString ImTagText                  = QString("text");
    const QString ImTagTextId                = QString("text_id");
    const QString ImTagSelectedText          = QString("selectedtext");
    const QString ImTagAlways                = QString("always");
    const QString ImTagLeft                  = QString("left");
    const QString ImTagRight                 = QString("right");
    const QString ImTagSendKeySequence        = QString("sendkeysequence");
    const QString ImTagSendString            = QString("sendstring");
    const QString ImTagSendCommand           = QString("sendcommand");
    const QString ImTagCopy                  = QString("copy");
    const QString ImTagPaste                 = QString("paste");
    const QString ImTagShowGroup             = QString("showgroup");
    const QString ImTagHideGroup             = QString("hidegroup");
    const QString ImTagKeySequence            = QString("keysequence");
    const QString ImTagString                = QString("string");
    const QString ImTagCommand               = QString("command");
    const QString ImTagOrientationLandscape = QString("landscape");
    const QString ImTagOrientationPortrait  = QString("portrait");
}

struct TBParseParameters {
    //! Contains true if current XML tag was successfully parsed
    bool validTag;

    QString fileName;

    ToolbarButton *currentButton;

    TBParseParameters();
};

struct TBParseStructure {
    TBParseStructure(const QString &name, ToolbarData::TagParser p)
        : tagName(name),
          parser(p) {
    };
    QString tagName;
    ToolbarData::TagParser parser;
};

TBParseParameters::TBParseParameters()
    : validTag(true),
      fileName(""),
      currentButton(0)
{
}

ToolbarData::ToolbarData()
{
}

ToolbarData::~ToolbarData()
{
}

QString ToolbarData::fileName() const
{
    return toolbarFileName;
}

bool ToolbarData::equal(const QString &toolbar) const
{
    QString absoluteFileName = toolbar;
    QFileInfo info(toolbar);
    if (info.isRelative())
        absoluteFileName = ToolbarConfigurationPath + info.fileName();
    return (absoluteFileName == toolbarFileName);
}

bool ToolbarData::loadNokiaToolbarXml(const QString &fileName)
{
    QString absoluteFileName = fileName;
    QFileInfo info(absoluteFileName);
    if (info.isRelative())
        absoluteFileName = ToolbarConfigurationPath + info.fileName();
    if (!QFile::exists(absoluteFileName)) {
        qDebug() << "can not find file:" << absoluteFileName;
        return false;
    }

    TBParseParameters params;
    params.fileName = absoluteFileName;
    toolbarFileName = absoluteFileName;
    bool valid = true;
    QFile infile(absoluteFileName);
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if (!infile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open toolbar xml file" << absoluteFileName;
        return false;
    }
    if (!doc.setContent((QIODevice *)(&infile), true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning() << __PRETTY_FUNCTION__ << "can not parse xml" << absoluteFileName
                   << "error line:" << errorLine << ", column:" << errorColumn;
        infile.close();
        return false;
    }

    const QDomElement root = doc.documentElement();
    //check the root tag
    if (!root.isNull() && root.tagName() != ImTagToolbar) {
        qWarning() << __PRETTY_FUNCTION__
                   << "wrong format xml" << absoluteFileName << "for virtual keyboard tool bar";
        valid = false;
    } else {
        const TBParseStructure parser(ImTagButton, &ToolbarData::parseTagButton);
        parseChildren(root, params, &parser);
        valid = params.validTag;
    }

    infile.close();

    return valid;
}

void ToolbarData::parseChildren(const QDomElement &element, TBParseParameters &params,
                                const TBParseStructure *parserList, int parserCount)
{
    Q_ASSERT(parserCount > 0);

    for (QDomNode child = element.firstChild(); !child.isNull() && params.validTag;
            child = child.nextSibling()) {
        if (child.isElement()) {
            const QDomElement childElement = child.toElement();
            bool found = false;
            for (int i = 0; i < parserCount; ++i) {
                if (childElement.tagName() == parserList[i].tagName) {
                    (this->*(parserList[i].parser))(childElement, params);
                    found = true;
                    break;
                }
            }
            if (!found) {
                qWarning() << "Unexpected tag" << childElement.tagName() << "on line"
                           << childElement.lineNumber() << "column" << childElement.columnNumber()
                           << "in layout file" << params.fileName;
                params.validTag = false;
            }
        }
    }
}

Qt::Alignment ToolbarData::alignment(const QString &alignmentString)
{
    Qt::Alignment align = Qt::AlignRight;
    if (alignmentString == ImTagLeft)
        align = Qt::AlignLeft;
    else if (alignmentString == ImTagRight)
        align = Qt::AlignRight;
    return align;
}

Dui::Orientation ToolbarData::orientation(const QString &orientationString)
{
    Dui::Orientation orient = Dui::Portrait;
    if (orientationString == ImTagOrientationLandscape)
        orient = Dui::Landscape;
    return orient;
}

ToolbarButton::VisibleType ToolbarData::visibleType(const QString &visibleTypeString)
{
    ToolbarButton::VisibleType type = ToolbarButton::Undefined;
    if (visibleTypeString == ImTagSelectedText)
        type = ToolbarButton::WhenSelectingText;
    else if (visibleTypeString == ImTagAlways)
        type = ToolbarButton::Always;
    return type;
}

void ToolbarData::parseTagButton(const QDomElement &element, TBParseParameters &params)
{
    const QString name = element.attribute(ImTagName);
    //check if the name is unique
    foreach(ToolbarButton * b, buttons) {
        if (b->name.compare(name, Qt::CaseInsensitive) >= 0) {
            return;
        }
    }

    ToolbarButton *b = new ToolbarButton();
    b->name = name;
    b->group = element.attribute(ImTagGroup);
    b->priority = element.attribute(ImTagPriority).toInt();
    b->orientation = orientation(element.attribute(ImTagOrientation));
    b->showOn = visibleType(element.attribute(ImTagShowOn));
    b->hideOn = visibleType(element.attribute(ImTagHideOn));
    b->alignment = alignment(element.attribute(ImTagAlignment));
    b->text = element.attribute(ImTagText);
    b->textId = element.attribute(ImTagTextId);
    buttons.append(b);
    params.currentButton = b;

    const TBParseStructure parser(ImTagActions, &ToolbarData::parseTagActions);
    parseChildren(element, params, &parser);
}

void ToolbarData::parseTagActions(const QDomElement &element, TBParseParameters &params)
{
    const TBParseStructure parsers[7] = {TBParseStructure(ImTagSendKeySequence, &ToolbarData::parseTagSendKeySequence),
                                         TBParseStructure(ImTagSendString, &ToolbarData::parseTagSendString),
                                         TBParseStructure(ImTagSendCommand, &ToolbarData::parseTagSendCommand),
                                         TBParseStructure(ImTagCopy, &ToolbarData::parseTagCopy),
                                         TBParseStructure(ImTagPaste, &ToolbarData::parseTagPaste),
                                         TBParseStructure(ImTagShowGroup, &ToolbarData::parseTagShowGroup),
                                         TBParseStructure(ImTagHideGroup, &ToolbarData::parseTagHideGroup)
                                        };
    parseChildren(element, params, parsers, 7);
}

void ToolbarData::parseTagSendKeySequence(const QDomElement &element, TBParseParameters &params)
{
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::SendKeySequence);
    action->keys = element.attribute(ImTagKeySequence);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagSendString(const QDomElement &element, TBParseParameters &params)
{
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::SendString);
    action->text = element.attribute(ImTagString);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagSendCommand(const QDomElement &element, TBParseParameters &params)
{
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::SendCommand);
    action->command = element.attribute(ImTagCommand);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagCopy(const QDomElement &element, TBParseParameters &params)
{
    Q_UNUSED(element);
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::Copy);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagPaste(const QDomElement &element, TBParseParameters &params)
{
    Q_UNUSED(element);
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::Paste);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagShowGroup(const QDomElement &element, TBParseParameters &params)
{
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::ShowGroup);
    action->group = element.attribute(ImTagGroup);
    params.currentButton->actions.append(action);
}

void ToolbarData::parseTagHideGroup(const QDomElement &element, TBParseParameters &params)
{
    ToolbarButton::Action *action = new ToolbarButton::Action(ToolbarButton::HideGroup);
    action->group = element.attribute(ImTagGroup);
    params.currentButton->actions.append(action);
}
