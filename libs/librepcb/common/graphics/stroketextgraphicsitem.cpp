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
#include <QtWidgets>
#include "stroketextgraphicsitem.h"
#include "origincrossgraphicsitem.h"
#include "../graphics/graphicslayer.h"
#include "../font/strokefontpool.h"
#include "../application.h"
#include "../toolbox.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

StrokeTextGraphicsItem::StrokeTextGraphicsItem(StrokeText& text,
        const IF_GraphicsLayerProvider& lp, QGraphicsItem* parent) noexcept :
    PrimitivePathGraphicsItem(parent), mText(text), mLayerProvider(lp)
{
    setPosition(mText.getPosition());
    setRotation(mText.getRotation());
    setLineLayer(mLayerProvider.getLayer(mText.getLayerName()));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(5);
    updatePath();

    // add origin cross
    mOriginCrossGraphicsItem.reset(new OriginCrossGraphicsItem(this));
    mOriginCrossGraphicsItem->setSize(Length(1000000));
    mOriginCrossGraphicsItem->setLayer(mLayerProvider.getLayer(GraphicsLayer::sTopReferences)); // TODO

    // register to the text to get attribute updates
    mText.registerObserver(*this);
}

StrokeTextGraphicsItem::~StrokeTextGraphicsItem() noexcept
{
    mText.unregisterObserver(*this);
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void StrokeTextGraphicsItem::strokeTextLayerNameChanged(const QString& newLayerName) noexcept
{
    setLineLayer(mLayerProvider.getLayer(newLayerName));
}

void StrokeTextGraphicsItem::strokeTextTextChanged(const QString& newText) noexcept
{
    Q_UNUSED(newText);
    updatePath();
}

void StrokeTextGraphicsItem::strokeTextPositionChanged(const Point& newPos) noexcept
{
    setPosition(newPos);
}

void StrokeTextGraphicsItem::strokeTextRotationChanged(const Angle& newRot) noexcept
{
    setRotation(newRot);
}

void StrokeTextGraphicsItem::strokeTextHeightChanged(const Length& newHeight) noexcept
{
    Q_UNUSED(newHeight);
    updatePath();
}

void StrokeTextGraphicsItem::strokeTextStrokeWidthRatioChanged(const Ratio& ratio) noexcept
{
    Q_UNUSED(ratio);
    updatePath();
}

void StrokeTextGraphicsItem::strokeTextLineSpacingFactorChanged(const Ratio& factor) noexcept
{
    Q_UNUSED(factor);
    updatePath();
}

void StrokeTextGraphicsItem::strokeTextAlignChanged(const Alignment& newAlign) noexcept
{
    Q_UNUSED(newAlign);
    updatePath();
}

void StrokeTextGraphicsItem::strokeTextMirroredChanged(bool mirrored) noexcept
{
    Q_UNUSED(mirrored);
    updatePath();
}

void StrokeTextGraphicsItem::updatePath() noexcept
{
    setLineWidth(Length(mText.getHeight().toNm() * mText.getStrokeWidthRatio().toNormalized()));
    try {
        const StrokeFont& font = qApp->getStrokeFonts().getFont("librepcb.bene");
        QVector<Path> paths = font.stroke(mText.getText(), mText.getHeight(),
            mText.getLineSpacingFactor(), mText.getAlign());
        if (mText.getMirrored()) {
            for (Path& p : paths) {
                p.mirror(Qt::Horizontal);
            }
        }
        setPath(Path::toQPainterPathPx(paths));
    } catch (const Exception& e) {
        qCritical() << "Failed to stroke font:" << e.getMsg();
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
