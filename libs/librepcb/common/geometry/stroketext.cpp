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
#include "stroketext.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

StrokeText::StrokeText(const StrokeText& other) noexcept
{
    *this = other; // use assignment operator
}

StrokeText::StrokeText(const Uuid& uuid, const StrokeText& other) noexcept :
    StrokeText(other)
{
    mUuid = uuid;
}

StrokeText::StrokeText(const Uuid& uuid, const QString& layerName, const QString& text,
        const Point& pos, const Angle& rotation, const Length& height,
        const Ratio& strokeWidth, const Ratio& lineSpacing, const Alignment& align,
        bool mirrored) noexcept :
    mUuid(uuid), mLayerName(layerName), mText(text), mPosition(pos), mRotation(rotation),
    mHeight(height), mStrokeWidthRatio(strokeWidth), mLineSpacingFactor(lineSpacing),
    mAlign(align), mMirrored(mirrored)
{
}

StrokeText::StrokeText(const SExpression& node)
{
    if (!Uuid(node.getChildByIndex(0).getValue<QString>(false)).isNull()) {
        mUuid = node.getChildByIndex(0).getValue<Uuid>(true);
        mText = node.getValueByPath<QString>("value", true);
    } else {
        // backward compatibility, remove this some time!
        mUuid = Uuid::createRandom();
        mText = node.getChildByIndex(0).getValue<QString>(true);
    }
    mLayerName = node.getValueByPath<QString>("layer", true);

    // load geometry attributes
    mPosition = Point(node.getChildByPath("pos"));
    mRotation = node.getValueByPath<Angle>("rot", true);
    mHeight = node.getValueByPath<Length>("height", true);
    if (!(mHeight > 0)) {
        throw RuntimeError(__FILE__, __LINE__, tr("The height of a text element is <= 0."));
    }
    if (const SExpression* child = node.tryGetChildByPath("stroke_width_ratio")) {
        mStrokeWidthRatio = child->getValueOfFirstChild<Ratio>(true);
    } else {
        // backward compatibility, remove this some time!
        mStrokeWidthRatio = Ratio::percent100() / 10;
    }
    if (const SExpression* child = node.tryGetChildByPath("line_spacing_factor")) {
        mLineSpacingFactor = child->getValueOfFirstChild<Ratio>(true);
    } else {
        // backward compatibility, remove this some time!
        mLineSpacingFactor = Ratio(1500000); // 150%
    }
    if (const SExpression* child = node.tryGetChildByPath("mirror")) {
        mMirrored = child->getValueOfFirstChild<bool>(true);
    } else {
        // backward compatibility, remove this some time!
        mMirrored = false;
    }
    mAlign = Alignment(node.getChildByPath("align"));

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

StrokeText::~StrokeText() noexcept
{
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void StrokeText::setLayerName(const QString& name) noexcept
{
    if (name == mLayerName) return;
    mLayerName = name;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextLayerNameChanged(mLayerName);
    }
}

void StrokeText::setText(const QString& text) noexcept
{
    if (text == mText) return;
    mText = text;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextTextChanged(mText);
    }
}

void StrokeText::setPosition(const Point& pos) noexcept
{
    if (pos == mPosition) return;
    mPosition = pos;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextPositionChanged(mPosition);
    }
}

void StrokeText::setRotation(const Angle& rotation) noexcept
{
    if (rotation == mRotation) return;
    mRotation = rotation;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextRotationChanged(mRotation);
    }
}

void StrokeText::setHeight(const Length& height) noexcept
{
    if (height == mHeight) return;
    mHeight = height;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextHeightChanged(mHeight);
    }
}

void StrokeText::setStrokeWidthRatio(const Ratio& ratio) noexcept
{
    if (ratio == mStrokeWidthRatio) return;
    mStrokeWidthRatio = ratio;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextStrokeWidthRatioChanged(mStrokeWidthRatio);
    }
}

void StrokeText::setLineSpacingFactor(const Ratio& factor) noexcept
{
    if (factor == mLineSpacingFactor) return;
    mLineSpacingFactor = factor;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextLineSpacingFactorChanged(mLineSpacingFactor);
    }
}

void StrokeText::setAlign(const Alignment& align) noexcept
{
    if (align == mAlign) return;
    mAlign = align;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextAlignChanged(mAlign);
    }
}

void StrokeText::setMirrored(bool mirrored) noexcept
{
    if (mirrored == mMirrored) return;
    mMirrored = mirrored;
    foreach (IF_StrokeTextObserver* object, mObservers) {
        object->strokeTextMirroredChanged(mMirrored);
    }
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void StrokeText::registerObserver(IF_StrokeTextObserver& object) const noexcept
{
    mObservers.insert(&object);
}

void StrokeText::unregisterObserver(IF_StrokeTextObserver& object) const noexcept
{
    mObservers.remove(&object);
}

void StrokeText::serialize(SExpression& root) const
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    root.appendToken(mUuid);
    root.appendTokenChild("layer", mLayerName, false);
    root.appendTokenChild("height", mHeight, true);
    root.appendTokenChild("stroke_width_ratio", mStrokeWidthRatio, false);
    root.appendTokenChild("line_spacing_factor", mLineSpacingFactor, false);
    root.appendChild(mAlign.serializeToDomElement("align"), true);
    root.appendChild(mPosition.serializeToDomElement("pos"), false);
    root.appendTokenChild("rot", mRotation, false);
    root.appendTokenChild("mirror", mMirrored, false);
    root.appendStringChild("value", mText, true);
}

/*****************************************************************************************
 *  Operator Overloadings
 ****************************************************************************************/

bool StrokeText::operator==(const StrokeText& rhs) const noexcept
{
    if (mUuid != rhs.mUuid)                             return false;
    if (mLayerName != rhs.mLayerName)                   return false;
    if (mText != rhs.mText)                             return false;
    if (mPosition != rhs.mPosition)                     return false;
    if (mRotation != rhs.mRotation)                     return false;
    if (mHeight != rhs.mHeight)                         return false;
    if (mStrokeWidthRatio != rhs.mStrokeWidthRatio)     return false;
    if (mLineSpacingFactor != rhs.mLineSpacingFactor)   return false;
    if (mAlign != rhs.mAlign)                           return false;
    if (mMirrored != rhs.mMirrored)                     return false;
    return true;
}

StrokeText& StrokeText::operator=(const StrokeText& rhs) noexcept
{
    mUuid = rhs.mUuid;
    mLayerName = rhs.mLayerName;
    mText = rhs.mText;
    mPosition = rhs.mPosition;
    mRotation = rhs.mRotation;
    mHeight = rhs.mHeight;
    mStrokeWidthRatio = rhs.mStrokeWidthRatio;
    mLineSpacingFactor = rhs.mLineSpacingFactor;
    mAlign = rhs.mAlign;
    mMirrored = rhs.mMirrored;
    return *this;
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool StrokeText::checkAttributesValidity() const noexcept
{
    if (mUuid.isNull())         return false;
    if (mText.isEmpty())        return false;
    if (mHeight <= 0)           return false;
    if (mStrokeWidthRatio < 0)  return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
