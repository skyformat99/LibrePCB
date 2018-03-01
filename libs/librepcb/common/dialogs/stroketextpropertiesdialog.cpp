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
#include "stroketextpropertiesdialog.h"
#include "ui_stroketextpropertiesdialog.h"
#include "../geometry/stroketext.h"
#include "../geometry/cmd/cmdstroketextedit.h"
#include "../graphics/graphicslayer.h"
#include "../undostack.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

StrokeTextPropertiesDialog::StrokeTextPropertiesDialog(StrokeText& text, UndoStack& undoStack,
                                                       QList<GraphicsLayer*> layers, QWidget* parent) noexcept :
    QDialog(parent), mText(text), mUndoStack(undoStack),
    mUi(new Ui::StrokeTextPropertiesDialog)
{
    mUi->setupUi(this);

    foreach (const GraphicsLayer* layer, layers) {
        mUi->cbxLayer->addItem(layer->getNameTr(), layer->getName());
    }

    connect(mUi->buttonBox, &QDialogButtonBox::clicked,
            this, &StrokeTextPropertiesDialog::on_buttonBox_clicked);

    // load text attributes
    selectLayerNameInCombobox(mText.getLayerName());
    mUi->edtText->setPlainText(mText.getText());
    mUi->alignmentSelector->setAlignment(mText.getAlign());
    mUi->spbHeight->setValue(mText.getHeight().toMm());
    mUi->spbxStrokeWidthRatio->setValue(mText.getStrokeWidthRatio().toPercent());
    mUi->spbxLineSpacingFactor->setValue(mText.getLineSpacingFactor().toPercent());
    mUi->spbPosX->setValue(mText.getPosition().getX().toMm());
    mUi->spbPosY->setValue(mText.getPosition().getY().toMm());
    mUi->spbRotation->setValue(mText.getRotation().toDeg());
    mUi->cbxMirrored->setChecked(mText.getMirrored());
}

StrokeTextPropertiesDialog::~StrokeTextPropertiesDialog() noexcept
{
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void StrokeTextPropertiesDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    switch (mUi->buttonBox->buttonRole(button)) {
        case QDialogButtonBox::ApplyRole:
            applyChanges();
            break;
        case QDialogButtonBox::AcceptRole:
            if (applyChanges()) {
                accept();
            }
            break;
        case QDialogButtonBox::RejectRole:
            reject();
            break;
        default: Q_ASSERT(false); break;
    }
}

bool StrokeTextPropertiesDialog::applyChanges() noexcept
{
    try {
        QScopedPointer<CmdStrokeTextEdit> cmd(new CmdStrokeTextEdit(mText));
        if (mUi->cbxLayer->currentIndex() >= 0 && mUi->cbxLayer->currentData().isValid()) {
            cmd->setLayerName(mUi->cbxLayer->currentData().toString(), false);
        }
        cmd->setText(mUi->edtText->toPlainText(), false);
        cmd->setAlignment(mUi->alignmentSelector->getAlignment(), false);
        cmd->setStrokeWidthRatio(Ratio::fromPercent(mUi->spbxStrokeWidthRatio->value()), false);
        cmd->setLineSpacingFactor(Ratio::fromPercent(mUi->spbxLineSpacingFactor->value()), false);
        cmd->setHeight(Length::fromMm(mUi->spbHeight->value()), false);
        cmd->setPosition(Point::fromMm(mUi->spbPosX->value(), mUi->spbPosY->value()), false);
        cmd->setRotation(Angle::fromDeg(mUi->spbRotation->value()), false);
        cmd->setMirrored(mUi->cbxMirrored->isChecked(), false);
        mUndoStack.execCmd(cmd.take());
        return true;
    } catch (const Exception& e) {
        QMessageBox::critical(this, tr("Error"), e.getMsg());
        return false;
    }
}

void StrokeTextPropertiesDialog::selectLayerNameInCombobox(const QString& name) noexcept
{
    mUi->cbxLayer->setCurrentIndex(mUi->cbxLayer->findData(name));
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
