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
#include "bi_stroketext.h"
#include "../board.h"
#include "../boardlayerstack.h"
#include "../../project.h"
#include <librepcb/common/font/strokefontpool.h>
#include <librepcb/common/graphics/graphicsscene.h>
#include <librepcb/common/graphics/primitivepathgraphicsitem.h>
#include <librepcb/common/geometry/stroketext.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BI_StrokeText::BI_StrokeText(Board& board, const BI_StrokeText& other) :
    BI_Base(board)
{
    mText.reset(new StrokeText(Uuid::createRandom(), *other.mText));
    init();
}

BI_StrokeText::BI_StrokeText(Board& board, const SExpression& node) :
    BI_Base(board)
{
    mText.reset(new StrokeText(node));
    init();    //setLineWidth(Length(mText.getHeight().toNm() * mText.getStrokeWidthRatio().toNormalized()));

}

BI_StrokeText::BI_StrokeText(Board& board, const StrokeText& text) :
    BI_Base(board)
{
    mText.reset(new StrokeText(text));
    init();
}

//BI_StrokeText::BI_StrokeText(Board& board, const Uuid& uuid, const QString& layerName, const Length& lineWidth, bool fill,
//                             bool isGrabArea, const Path& path) :
//    BI_Base(board)
//{
//    mPolygon.reset(new Polygon(uuid, layerName, lineWidth, fill, isGrabArea, path));
//    init();
//}

void BI_StrokeText::init()
{
    mGraphicsItem.reset(new PrimitivePathGraphicsItem());
    mGraphicsItem->setPosition(mText->getPosition());
    mGraphicsItem->setRotation(mText->getRotation());
    mGraphicsItem->setLineLayer(mBoard.getLayerStack().getLayer(mText->getLayerName()));
    mGraphicsItem->setLineWidth(mText->calcStrokeWidth());
    mGraphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    mGraphicsItem->setZValue(Board::ZValue_Default);

    // register to the text to get attribute updates
    mText->registerObserver(*this);

    // connect to the "attributes changed" signal of the board
    connect(&mBoard, &Board::attributesChanged, this, &BI_StrokeText::boardAttributesChanged);
}

BI_StrokeText::~BI_StrokeText() noexcept
{
    mText->unregisterObserver(*this);
    mGraphicsItem.reset();
    mText.reset();
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void BI_StrokeText::addToBoard()
{
    if (isAddedToBoard()) {
        throw LogicError(__FILE__, __LINE__);
    }
    BI_Base::addToBoard(mGraphicsItem.data());
}

void BI_StrokeText::removeFromBoard()
{
    if (!isAddedToBoard()) {
        throw LogicError(__FILE__, __LINE__);
    }
    BI_Base::removeFromBoard(mGraphicsItem.data());
}

void BI_StrokeText::serialize(SExpression& root) const
{
    mText->serialize(root);
}

/*****************************************************************************************
 *  Inherited from BI_Base
 ****************************************************************************************/

const Point& BI_StrokeText::getPosition() const noexcept
{
    return mText->getPosition();
}

QPainterPath BI_StrokeText::getGrabAreaScenePx() const noexcept
{
    return mGraphicsItem->sceneTransform().map(mGraphicsItem->shape());
}

bool BI_StrokeText::isSelectable() const noexcept
{
    const GraphicsLayer* layer = mBoard.getLayerStack().getLayer(mText->getLayerName());
    return layer && layer->isVisible();
}

void BI_StrokeText::setSelected(bool selected) noexcept
{
    BI_Base::setSelected(selected);
    mGraphicsItem->setSelected(selected);
}

/*****************************************************************************************
 *  Private Slots
 ****************************************************************************************/

void BI_StrokeText::boardAttributesChanged()
{
    updatePaths();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void BI_StrokeText::strokeTextLayerNameChanged(const QString& newLayerName) noexcept
{
    mGraphicsItem->setLineLayer(mBoard.getLayerStack().getLayer(newLayerName));
}

void BI_StrokeText::strokeTextTextChanged(const QString& newText) noexcept
{
    Q_UNUSED(newText);
    updatePaths();
}

void BI_StrokeText::strokeTextPositionChanged(const Point& newPos) noexcept
{
    mGraphicsItem->setPosition(newPos);
}

void BI_StrokeText::strokeTextRotationChanged(const Angle& newRot) noexcept
{
    mGraphicsItem->setRotation(newRot);
}

void BI_StrokeText::strokeTextHeightChanged(const Length& newHeight) noexcept
{
    Q_UNUSED(newHeight);
    mGraphicsItem->setLineWidth(mText->calcStrokeWidth());
    updatePaths();
}

void BI_StrokeText::strokeTextStrokeWidthRatioChanged(const Ratio& ratio) noexcept
{
    Q_UNUSED(ratio);
    mGraphicsItem->setLineWidth(mText->calcStrokeWidth());
    updatePaths();
}

void BI_StrokeText::strokeTextLineSpacingFactorChanged(const Ratio& factor) noexcept
{
    Q_UNUSED(factor);
    updatePaths();
}

void BI_StrokeText::strokeTextAlignChanged(const Alignment& newAlign) noexcept
{
    Q_UNUSED(newAlign);
    updatePaths();
}

void BI_StrokeText::strokeTextMirroredChanged(bool mirrored) noexcept
{
    Q_UNUSED(mirrored);
    updatePaths();
}

void BI_StrokeText::updatePaths() noexcept
{
    try {
        const StrokeFont& font = getProject().getStrokeFonts().getFont("librepcb.bene"); // can throw
        mPaths = font.stroke(mText->getText(), mText->getHeight(),
            mText->getLineSpacingFactor(), mText->getAlign());
        if (mText->getMirrored()) {
            for (Path& p : mPaths) {
                p.mirror(Qt::Horizontal);
            }
        }
        mGraphicsItem->setPath(Path::toQPainterPathPx(mPaths));
    } catch (const Exception& e) {
        qCritical() << "Failed to draw text:" << e.getMsg();
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
