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
#include <QtConcurrent/QtConcurrent>
#include "strokefont.h"
#include "../fileio/fileutils.h"
#include <fontobene/font.h>
#include <fontobene/glyphlistaccessor.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

namespace fb = fontobene;

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

StrokeFont::StrokeFont(const FilePath& fontFilePath) noexcept :
    QObject(nullptr), mFilePath(fontFilePath)
{
    // load the font in another thread because it takes some time to load it
    qDebug() << "Start loading font" << mFilePath.toNative();
    mFuture = QtConcurrent::run([fontFilePath](){return fb::Font(fontFilePath.toStr());});
    connect(&mWatcher, &QFutureWatcher<fb::Font>::finished, this, &StrokeFont::fontLoaded);
    mWatcher.setFuture(mFuture);
}

StrokeFont::~StrokeFont() noexcept
{
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

QVector<Path> StrokeFont::stroke(const QString& text, const Length& height,
                                 const Ratio& lineSpacingFactor, const Alignment& align) const noexcept
{
    QVector<Path> paths;
    Length totalWidth;
    QVector<QPair<QVector<Path>, Length>> lines = strokeLines(text, height, totalWidth);
    for (int i = 0; i < lines.count(); ++i) {
        Point pos(0, 0);
        if (align.getH() == HAlign::left()) {
            pos.setX(Length(0));
        } else if (align.getH() == HAlign::right()) {
            pos.setX((totalWidth - lines.at(i).second) - totalWidth);
        } else {
            pos.setX(lines.at(i).second / -2);
        }
        Length lineSpacing = calcLineSpacing(height, lineSpacingFactor);
        if (align.getV() == VAlign::bottom()) {
            pos.setY(lineSpacing * (lines.count() - i - 1));
        } else if (align.getV() == VAlign::top()) {
            pos.setY(-height - lineSpacing * i);
        } else {
            Length h = lineSpacing * (lines.count() - i - 1);
            Length totalHeight = height + lineSpacing * (lines.count() - 1);
            pos.setY(h - (totalHeight / 2));
        }
        foreach (const Path& p, lines.at(i).first) {
            paths.append(p.translated(pos));
        }
    }
    return paths;
}

QVector<QPair<QVector<Path>, Length>> StrokeFont::strokeLines(const QString& text,
    const Length& height, Length& width) const noexcept
{
    QVector<QPair<QVector<Path>, Length>> result;
    foreach (const QString& line, text.split('\n')) {
        QPair<QVector<Path>, Length> pair;
        pair.first = strokeLine(line, height, pair.second);
        result.append(pair);
        if (pair.second > width) width = pair.second;
    }
    return result;
}

QVector<Path> StrokeFont::strokeLine(const QString& text, const Length& height,
                                     Length& width) const noexcept
{
    QVector<Path> paths;
    Length offset(0);
    width = Length(0); // same as offset, but without last letter spacing
    for (int i = 0; i < text.length(); ++i) {
        if (text.at(i) == ' ') {
            offset += calcWordSpacing(height);
            width = offset;
        } else {
            QVector<Path> glyphPaths = strokeGlyph(text.at(i), height);
            if (glyphPaths.isEmpty()) continue; // don't add letter space for empty glyphs
            Point bottomLeft, topRight;
            computeBoundingRect(glyphPaths, bottomLeft, topRight);
            foreach (const Path& p, glyphPaths) {
                paths.append(p.translated(Point(offset, Length(0))));
            }
            width = offset + topRight.getX().abs(); // same concept as in LibreCAD, even if not fully understood ;)
            offset = width + calcLetterSpacing(height);
        }
    }
    return paths;
}

QVector<Path> StrokeFont::strokeGlyph(const QChar& glyph, const Length& height) const noexcept
{
    bool ok = false;
    QVector<fb::Polyline> polylines = accessor().getAllPolylinesOfGlyph(glyph.unicode(), &ok);
    if (!ok) {
        qWarning() << "Failed to load stroke font glyph" << glyph;
    }
    return polylines2paths(polylines, height);
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void StrokeFont::fontLoaded() noexcept
{
    accessor(); // trigger the message about loading succeeded or failed
}

const fb::GlyphListAccessor& StrokeFont::accessor() const noexcept
{
    if (!mFont) {
        try {
            mFont.reset(new fb::Font(mFuture.result())); // can throw
            qDebug() << "Successfully loaded font" << mFilePath.toNative()
                     << "with" << mFont->glyphs.count() << "glyphs";
        } catch (const fb::Exception& e) {
            mFont.reset(new fb::Font());
            qCritical() << "Failed to load font" << mFilePath.toNative();
            qCritical() << "Error:" << e.msg();
        }
        mGlyphListAccessor.reset(new fontobene::GlyphListAccessor(mFont->glyphs));
        mGlyphListAccessor->addReplacements({0x00B5, 0x03BC}); // MICRO SIGN μ
        mGlyphListAccessor->addReplacements({0x2126, 0x03A9}); // OHM SIGN Ω
    }
    return *mGlyphListAccessor;
}

Length StrokeFont::calcLetterSpacing(const Length& height) const noexcept
{
    return Length(height.toNm() * mFont->header.letterSpacing / 9);
}

Length StrokeFont::calcWordSpacing(const Length& height) const noexcept
{
    return Length(height.toNm() * mFont->header.wordSpacing / 9);
}

Length StrokeFont::calcLineSpacing(const Length& height, const Ratio& factor) const noexcept
{
    return Length(height.toNm() * mFont->header.lineSpacing * factor.toNormalized() / 9);
}

QVector<Path> StrokeFont::polylines2paths(const QVector<fontobene::Polyline>& polylines,
                                          const Length& height) noexcept
{
    QVector<Path> paths;
    foreach (const fb::Polyline& p, polylines) {
        if (p.isEmpty()) continue;
        paths.append(polyline2path(p, height));
    }
    return paths;
}

Path StrokeFont::polyline2path(const fontobene::Polyline& p, const Length& height) noexcept
{
    Path path;
    for (int i = 0; i < p.size(); ++i) {
        Vertex v = convertVertex(p.at(i), height);
        Vertex v2 = convertVertex(p.at((i + 1) % p.size()), height);
        path.addVertex(Vertex(v.getPos(), v2.getAngle()));
    }
    return path;
}

Vertex StrokeFont::convertVertex(const fb::Vertex& v, const Length& height) noexcept
{
    return Vertex(Point::fromMm(v.scaledX(height.toMm()), v.scaledY(height.toMm())),
                  Angle::fromDeg(v.scaledBulge(180)));
}

void StrokeFont::computeBoundingRect(const QVector<Path>& paths,
                                     Point& bottomLeft, Point& topRight) noexcept
{
    QRectF rect = Path::toQPainterPathPx(paths).boundingRect();
    bottomLeft = Point::fromPx(rect.bottomLeft());
    topRight = Point::fromPx(rect.topRight());
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
