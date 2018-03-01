/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "cmdstroketextedit.h"
#include "../stroketext.h"
#include <librepcb/common/graphics/graphicslayer.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdStrokeTextEdit::CmdStrokeTextEdit(StrokeText& text) noexcept :
    UndoCommand(tr("Edit stroke text")), mText(text),
    mOldLayerName(text.getLayerName()), mNewLayerName(mOldLayerName),
    mOldText(text.getText()), mNewText(mOldText),
    mOldPosition(text.getPosition()), mNewPosition(mOldPosition),
    mOldRotation(text.getRotation()), mNewRotation(mOldRotation),
    mOldHeight(text.getHeight()), mNewHeight(mOldHeight),
    mOldStrokeWidthRatio(text.getStrokeWidthRatio()), mNewStrokeWidthRatio(mOldStrokeWidthRatio),
    mOldLineSpacingFactor(text.getLineSpacingFactor()), mNewLineSpacingFactor(mOldLineSpacingFactor),
    mOldAlign(text.getAlign()), mNewAlign(mOldAlign),
    mOldMirrored(text.getMirrored()), mNewMirrored(mOldMirrored)
{
}

CmdStrokeTextEdit::~CmdStrokeTextEdit() noexcept
{
    if (!wasEverExecuted()) {
        performUndo(); // discard possible executed immediate changes
    }
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void CmdStrokeTextEdit::setLayerName(const QString& name, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewLayerName = name;
    if (immediate) mText.setLayerName(mNewLayerName);
}

void CmdStrokeTextEdit::setText(const QString& text, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewText = text;
    if (immediate) mText.setText(mNewText);
}

void CmdStrokeTextEdit::setHeight(const Length& height, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewHeight = height;
    if (immediate) mText.setHeight(mNewHeight);
}

void CmdStrokeTextEdit::setStrokeWidthRatio(const Ratio& ratio, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewStrokeWidthRatio = ratio;
    if (immediate) mText.setStrokeWidthRatio(mNewStrokeWidthRatio);
}

void CmdStrokeTextEdit::setLineSpacingFactor(const Ratio& factor, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewLineSpacingFactor = factor;
    if (immediate) mText.setLineSpacingFactor(mNewLineSpacingFactor);
}

void CmdStrokeTextEdit::setAlignment(const Alignment& align, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewAlign = align;
    if (immediate) mText.setAlign(mNewAlign);
}

void CmdStrokeTextEdit::setPosition(const Point& pos, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewPosition = pos;
    if (immediate) mText.setPosition(mNewPosition);
}

void CmdStrokeTextEdit::setDeltaToStartPos(const Point& deltaPos, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewPosition = mOldPosition + deltaPos;
    if (immediate) mText.setPosition(mNewPosition);
}

void CmdStrokeTextEdit::setRotation(const Angle& angle, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewRotation = angle;
    if (immediate) mText.setRotation(mNewRotation);
}

void CmdStrokeTextEdit::rotate(const Angle& angle, const Point& center, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewPosition.rotate(angle, center);
    mNewRotation += angle;
    if (immediate) {
        mText.setPosition(mNewPosition);
        mText.setRotation(mNewRotation);
    }
}

void CmdStrokeTextEdit::setMirrored(bool mirrored, bool immediate) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewMirrored = mirrored;
    if (immediate) mText.setMirrored(mNewMirrored);
}

void CmdStrokeTextEdit::mirror(const Point& center, Qt::Orientation orientation, bool immediate) noexcept
{
    setLayerName(GraphicsLayer::getMirroredLayerName(mNewLayerName), immediate);
    setMirrored(!mNewMirrored, immediate);
    setPosition(mNewPosition.mirrored(orientation, center), immediate);
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdStrokeTextEdit::performExecute()
{
    performRedo(); // can throw

    if (mNewLayerName           != mOldLayerName)           return true;
    if (mNewText                != mOldText)                return true;
    if (mNewPosition            != mOldPosition)            return true;
    if (mNewRotation            != mOldRotation)            return true;
    if (mNewHeight              != mOldHeight)              return true;
    if (mNewStrokeWidthRatio    != mOldStrokeWidthRatio)    return true;
    if (mNewLineSpacingFactor   != mOldLineSpacingFactor)   return true;
    if (mNewAlign               != mOldAlign)               return true;
    if (mNewMirrored            != mOldMirrored)            return true;
    return false;
}

void CmdStrokeTextEdit::performUndo()
{
    mText.setLayerName(mOldLayerName);
    mText.setText(mOldText);
    mText.setPosition(mOldPosition);
    mText.setRotation(mOldRotation);
    mText.setHeight(mOldHeight);
    mText.setStrokeWidthRatio(mOldStrokeWidthRatio);
    mText.setLineSpacingFactor(mOldLineSpacingFactor);
    mText.setAlign(mOldAlign);
    mText.setMirrored(mOldMirrored);
}

void CmdStrokeTextEdit::performRedo()
{
    mText.setLayerName(mNewLayerName);
    mText.setText(mNewText);
    mText.setPosition(mNewPosition);
    mText.setRotation(mNewRotation);
    mText.setHeight(mNewHeight);
    mText.setStrokeWidthRatio(mNewStrokeWidthRatio);
    mText.setLineSpacingFactor(mNewLineSpacingFactor);
    mText.setAlign(mNewAlign);
    mText.setMirrored(mNewMirrored);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
