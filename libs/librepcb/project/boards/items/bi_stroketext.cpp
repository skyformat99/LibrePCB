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
#include <librepcb/common/graphics/graphicsscene.h>
#include <librepcb/common/graphics/stroketextgraphicsitem.h>
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
    init();
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
    mGraphicsItem.reset(new StrokeTextGraphicsItem(*mText, mBoard.getLayerStack()));
    mGraphicsItem->setZValue(Board::ZValue_Default);

    // connect to the "attributes changed" signal of the board
    connect(&mBoard, &Board::attributesChanged, this, &BI_StrokeText::boardAttributesChanged);
}

BI_StrokeText::~BI_StrokeText() noexcept
{
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
    mGraphicsItem->update();
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
