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
#include "cmdmoveselectedboarditems.h"
#include <librepcb/common/geometry/cmd/cmdpolygonedit.h>
#include <librepcb/common/gridproperties.h>
#include <librepcb/project/project.h>
#include <librepcb/project/boards/board.h>
#include <librepcb/project/boards/items/bi_device.h>
#include <librepcb/project/boards/items/bi_footprint.h>
#include <librepcb/project/boards/items/bi_netpoint.h>
#include <librepcb/project/boards/items/bi_via.h>
#include <librepcb/project/boards/items/bi_polygon.h>
#include <librepcb/project/boards/cmd/cmddeviceinstanceedit.h>
#include <librepcb/project/boards/cmd/cmdboardviaedit.h>
#include <librepcb/project/boards/cmd/cmdboardnetpointedit.h>
#include <librepcb/project/boards/cmd/cmdboardplaneedit.h>
#include <librepcb/project/boards/boardselectionquery.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {
namespace editor {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdMoveSelectedBoardItems::CmdMoveSelectedBoardItems(Board& board, const Point& startPos) noexcept :
    UndoCommandGroup(tr("Move Board Elements")),
    mBoard(board), mStartPos(startPos), mDeltaPos(0, 0)
{
    // get all selected items
    std::unique_ptr<BoardSelectionQuery> query(mBoard.createSelectionQuery());
    query->addSelectedFootprints();
    query->addSelectedVias();
    query->addSelectedNetPoints(BoardSelectionQuery::NetPointFilter::Floating);
    query->addSelectedNetLines(BoardSelectionQuery::NetLineFilter::All);
    query->addNetPointsOfNetLines(BoardSelectionQuery::NetLineFilter::All,
                                  BoardSelectionQuery::NetPointFilter::Floating);
    query->addSelectedPlanes();
    query->addSelectedPolygons();

    foreach (BI_Footprint* footprint, query->getFootprints()) { Q_ASSERT(footprint);
        BI_Device& device = footprint->getDeviceInstance();
        CmdDeviceInstanceEdit* cmd = new CmdDeviceInstanceEdit(device);
        mDeviceEditCmds.append(cmd);
    }
    foreach (BI_Via* via, query->getVias()) { Q_ASSERT(via);
        CmdBoardViaEdit* cmd = new CmdBoardViaEdit(*via);
        mViaEditCmds.append(cmd);
    }
    foreach (BI_NetPoint* netpoint, query->getNetPoints()) { Q_ASSERT(netpoint);
        CmdBoardNetPointEdit* cmd = new CmdBoardNetPointEdit(*netpoint);
        mNetPointEditCmds.append(cmd);
    }
    foreach (BI_Plane* plane, query->getPlanes()) { Q_ASSERT(plane);
        CmdBoardPlaneEdit* cmd = new CmdBoardPlaneEdit(*plane, false);
        mPlaneEditCmds.append(cmd);
    }
    foreach (BI_Polygon* polygon, query->getPolygons()) { Q_ASSERT(polygon);
        CmdPolygonEdit* cmd = new CmdPolygonEdit(polygon->getPolygon());
        mPolygonEditCmds.append(cmd);
    }
}

CmdMoveSelectedBoardItems::~CmdMoveSelectedBoardItems() noexcept
{
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void CmdMoveSelectedBoardItems::setCurrentPosition(const Point& pos) noexcept
{
    Point delta = pos - mStartPos;
    delta.mapToGrid(mBoard.getGridProperties().getInterval());

    if (delta != mDeltaPos) {
        // move selected elements
        foreach (CmdDeviceInstanceEdit* cmd, mDeviceEditCmds) {
            cmd->setDeltaToStartPos(delta, true);
        }
        foreach (CmdBoardViaEdit* cmd, mViaEditCmds) {
            cmd->setDeltaToStartPos(delta, true);
        }
        foreach (CmdBoardNetPointEdit* cmd, mNetPointEditCmds) {
            cmd->setDeltaToStartPos(delta, true);
        }
        foreach (CmdBoardPlaneEdit* cmd, mPlaneEditCmds) {
            cmd->setDeltaToStartPos(delta, true);
        }
        foreach (CmdPolygonEdit* cmd, mPolygonEditCmds) {
            cmd->setDeltaToStartPos(delta, true);
        }
        mDeltaPos = delta;
    }
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdMoveSelectedBoardItems::performExecute()
{
    if (mDeltaPos.isOrigin()) {
        // no movement required --> discard all move commands
        qDeleteAll(mDeviceEditCmds);    mDeviceEditCmds.clear();
        qDeleteAll(mViaEditCmds);       mViaEditCmds.clear();
        qDeleteAll(mNetPointEditCmds);  mNetPointEditCmds.clear();
        qDeleteAll(mPlaneEditCmds);     mPlaneEditCmds.clear();
        qDeleteAll(mPolygonEditCmds);   mPolygonEditCmds.clear();
        return false;
    }

    foreach (CmdDeviceInstanceEdit* cmd, mDeviceEditCmds) {
        appendChild(cmd); // can throw
    }
    foreach (CmdBoardViaEdit* cmd, mViaEditCmds) {
        appendChild(cmd); // can throw
    }
    foreach (CmdBoardNetPointEdit* cmd, mNetPointEditCmds) {
        appendChild(cmd); // can throw
    }
    foreach (CmdBoardPlaneEdit* cmd, mPlaneEditCmds) {
        appendChild(cmd); // can throw
    }
    foreach (CmdPolygonEdit* cmd, mPolygonEditCmds) {
        appendChild(cmd); // can throw
    }

    // execute all child commands
    return UndoCommandGroup::performExecute(); // can throw
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace editor
} // namespace project
} // namespace librepcb
