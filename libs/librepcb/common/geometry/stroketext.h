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

#ifndef LIBREPCB_STROKETEXT_H
#define LIBREPCB_STROKETEXT_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "../fileio/serializableobjectlist.h"
#include "../fileio/cmd/cmdlistelementinsert.h"
#include "../fileio/cmd/cmdlistelementremove.h"
#include "../fileio/cmd/cmdlistelementsswap.h"
#include "../units/all_length_units.h"
#include "../alignment.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Interface IF_StrokeTextObserver
 ****************************************************************************************/

/**
 * @brief The IF_StrokeTextObserver class
 */
class IF_StrokeTextObserver
{
    public:
        virtual void strokeTextLayerNameChanged(const QString& newLayerName) noexcept = 0;
        virtual void strokeTextTextChanged(const QString& newText) noexcept = 0;
        virtual void strokeTextPositionChanged(const Point& newPos) noexcept = 0;
        virtual void strokeTextRotationChanged(const Angle& newRot) noexcept = 0;
        virtual void strokeTextHeightChanged(const Length& newHeight) noexcept = 0;
        virtual void strokeTextStrokeWidthRatioChanged(const Ratio& ratio) noexcept = 0;
        virtual void strokeTextLineSpacingFactorChanged(const Ratio& factor) noexcept = 0;
        virtual void strokeTextAlignChanged(const Alignment& newAlign) noexcept = 0;
        virtual void strokeTextMirroredChanged(bool newMirrored) noexcept = 0;

    protected:
        IF_StrokeTextObserver() noexcept {}
        explicit IF_StrokeTextObserver(const IF_StrokeTextObserver& other) = delete;
        virtual ~IF_StrokeTextObserver() noexcept {}
        IF_StrokeTextObserver& operator=(const IF_StrokeTextObserver& rhs) = delete;
};

/*****************************************************************************************
 *  Class StrokeText
 ****************************************************************************************/

/**
 * @brief The StrokeText class
 */
class StrokeText final : public SerializableObject
{
        Q_DECLARE_TR_FUNCTIONS(StrokeText)

    public:

        // Constructors / Destructor
        StrokeText() = delete;
        StrokeText(const StrokeText& other) noexcept;
        StrokeText(const Uuid& uuid, const StrokeText& other) noexcept;
        StrokeText(const Uuid& uuid, const QString& layerName, const QString& text,
                   const Point& pos, const Angle& rotation, const Length& height,
                   const Ratio& strokeWidth, const Ratio& lineSpacing,
                   const Alignment& align, bool mirrored) noexcept;
        explicit StrokeText(const SExpression& node);
        ~StrokeText() noexcept;

        // Getters
        const Uuid& getUuid() const noexcept {return mUuid;}
        const QString& getLayerName() const noexcept {return mLayerName;}
        const Point& getPosition() const noexcept {return mPosition;}
        const Angle& getRotation() const noexcept {return mRotation;}
        const Length& getHeight() const noexcept {return mHeight;}
        const Ratio& getStrokeWidthRatio() const noexcept {return mStrokeWidthRatio;}
        const Ratio& getLineSpacingFactor() const noexcept {return mLineSpacingFactor;}
        const Alignment& getAlign() const noexcept {return mAlign;}
        bool getMirrored() const noexcept {return mMirrored;}
        const QString& getText() const noexcept {return mText;}
        Length calcStrokeWidth() const noexcept;

        // Setters
        void setLayerName(const QString& name) noexcept;
        void setText(const QString& text) noexcept;
        void setPosition(const Point& pos) noexcept;
        void setRotation(const Angle& rotation) noexcept;
        void setHeight(const Length& height) noexcept;
        void setStrokeWidthRatio(const Ratio& ratio) noexcept;
        void setLineSpacingFactor(const Ratio& factor) noexcept;
        void setAlign(const Alignment& align) noexcept;
        void setMirrored(bool mirrored) noexcept;

        // General Methods
        void registerObserver(IF_StrokeTextObserver& object) const noexcept;
        void unregisterObserver(IF_StrokeTextObserver& object) const noexcept;

        /// @copydoc librepcb::SerializableObject::serialize()
        void serialize(SExpression& root) const override;

        // Operator Overloadings
        bool operator==(const StrokeText& rhs) const noexcept;
        bool operator!=(const StrokeText& rhs) const noexcept {return !(*this == rhs);}
        StrokeText& operator=(const StrokeText& rhs) noexcept;


    private: // Methods
        bool checkAttributesValidity() const noexcept;


    private: // Data
        Uuid mUuid;
        QString mLayerName;
        QString mText;
        Point mPosition;
        Angle mRotation;
        Length mHeight;
        Ratio mStrokeWidthRatio;
        Ratio mLineSpacingFactor;
        Alignment mAlign;
        bool mMirrored;

        // Misc
        mutable QSet<IF_StrokeTextObserver*> mObservers; ///< A list of all observer objects
};

/*****************************************************************************************
 *  Class TextList
 ****************************************************************************************/

struct StrokeTextListNameProvider {static constexpr const char* tagname = "stroke_text";};
using StrokeTextList = SerializableObjectList<StrokeText, StrokeTextListNameProvider>;
using CmdStrokeTextInsert = CmdListElementInsert<StrokeText, StrokeTextListNameProvider>;
using CmdStrokeTextRemove = CmdListElementRemove<StrokeText, StrokeTextListNameProvider>;
using CmdStrokeTextsSwap = CmdListElementsSwap<StrokeText, StrokeTextListNameProvider>;

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb

#endif // LIBREPCB_STROKETEXT_H
