#include "XULWin/Layout.h"
#include "XULWin/ErrorReporter.h"
#include <assert.h>


namespace XULWin
{

    void LinearLayoutManager::GetSizes(int inLength,
                                       const std::vector<int> & inFlexValues,
                                       std::vector<int> & outSizes)
    {
        assert(outSizes.empty());

        int sumOfProportions = 0;
        for (size_t idx = 0; idx != inFlexValues.size(); ++idx)
        {
            sumOfProportions += inFlexValues[idx];
        }

        for (size_t idx = 0; idx != inFlexValues.size(); ++idx)
        {
            int proportion = 0;
            if (inFlexValues[idx] != 0 && sumOfProportions != 0)
            {
                proportion = (int)(0.5 + (float)inLength*(float)inFlexValues[idx]/(float)sumOfProportions);
            }
            outSizes.push_back(proportion);
        }
    }


    void LinearLayoutManager::GetSizes
    (
        int inLength,
        const std::vector<SizeInfo> & inFlexValues,
        std::vector<int> & outSizes
    )
    {
        assert(outSizes.empty());

        int availableLength = inLength;
        std::vector<int> flexValues;
        for (size_t idx = 0; idx != inFlexValues.size(); ++idx)
        {
            if (inFlexValues[idx].Flex != 0)
            {
                // it's a widget with flex
                flexValues.push_back(inFlexValues[idx].Flex);
            }
            else
            {
                // it's a widget without flex
                availableLength -= inFlexValues[idx].MinSize;
            }
        }
        std::vector<int> sizes;
        GetSizes(availableLength, flexValues, sizes);

        std::vector<int> componentsToTrim_FlexValues;
        std::vector<int> componentsToTrim_Indices;

        int sizesIdx = 0;
        int amountExceeded = 0;
        for (size_t idx = 0; idx != inFlexValues.size(); ++idx)
        {
            if (inFlexValues[idx].Flex != 0)
            {
                int size = sizes[sizesIdx];
                if (size < inFlexValues[idx].MinSize)
                {
                    int minSize = inFlexValues[idx].MinSize;
                    amountExceeded += minSize - size;
                    size = inFlexValues[idx].MinSize;
                }
                else if (size > inFlexValues[idx].MinSize)
                {
                    componentsToTrim_FlexValues.push_back(inFlexValues[idx].Flex);
                    componentsToTrim_Indices.push_back(idx);
                }
                outSizes.push_back(size);
                sizesIdx++;
            }
            else
            {
                outSizes.push_back(inFlexValues[idx].MinSize);
            }
        }

        // If we lack the space to display all components, we gonna trim the
        // flex widgets according to their own flex values. So widgets with
        // high flex will lose more space.
        if (amountExceeded > 0 && !componentsToTrim_FlexValues.empty())
        {
            std::vector<int> spacerCuts;
            GetSizes(amountExceeded, componentsToTrim_FlexValues, spacerCuts);
            if (!spacerCuts.empty())
            {
                for (size_t idx = 0; idx != componentsToTrim_Indices.size(); ++idx)
                {
                    int flexIdx = componentsToTrim_Indices[idx];
                    outSizes[flexIdx] -= spacerCuts[idx];
                    if (outSizes[idx] < inFlexValues[flexIdx].MinSize)
                    {
                        int size = inFlexValues[flexIdx].MinSize;
                        outSizes[flexIdx] = size;
                    }
                }
            }
        }
    }


    LinearLayoutManager::LinearLayoutManager(Orient inOrient) :
        mOrient(inOrient)
    {
    }


    Orient LinearLayoutManager::orientation() const
    {
        return mOrient;
    }


    void LinearLayoutManager::getRects(const Rect & inRect,
                                       Align inAlign,
                                       const std::vector<ExtendedSizeInfo> & inSizeInfos,
                                       std::vector<Rect> & outRects)
    {
        std::vector<int> sizes;

        std::vector<SizeInfo> sizeInfos;
        for (size_t idx = 0; idx != inSizeInfos.size(); ++idx)
        {
            sizeInfos.push_back(inSizeInfos[idx]);
        }
        GetSizes(mOrient == Horizontal ? inRect.width() : inRect.height(), sizeInfos, sizes);
        int xOffset = inRect.x();
        int yOffset = inRect.y();
        bool horizontal = mOrient == Horizontal;
        bool vertical = mOrient == Vertical;
        for (size_t idx = 0; idx != inSizeInfos.size(); ++idx)
        {
            int x = horizontal ? xOffset : inRect.x();
            int y = vertical   ? yOffset : inRect.y();
            int w = horizontal ? sizes[idx] : inSizeInfos[idx].MinSizeOpposite;
            int h = vertical   ? sizes[idx] : inSizeInfos[idx].MinSizeOpposite;
            if (inAlign == Start)
            {
                if (horizontal)
                {
                    y = inRect.y();
                }
                else if (vertical)
                {
                    x = inRect.x();
                }
            }
            if (inAlign == Center)
            {
                if (horizontal)
                {
                    y = yOffset + (inRect.height() - inSizeInfos[idx].MinSizeOpposite)/2;
                }
                else if (vertical)
                {
                    x = xOffset + (inRect.width() - inSizeInfos[idx].MinSizeOpposite)/2;
                }
            }
            if (inAlign == End)
            {
                if (horizontal)
                {
                    y = yOffset + inRect.height() - inSizeInfos[idx].MinSizeOpposite;
                }
                else if (vertical)
                {
                    x = xOffset + inRect.width() - inSizeInfos[idx].MinSizeOpposite;
                }
            }
            else if (inAlign == Stretch)
            {
                if (horizontal)
                {
                    h = inRect.height();
                }
                else if (vertical)
                {
                    w = inRect.width();
                }
            }
            outRects.push_back(Rect(x, y, w, h));
            if (horizontal)
            {
                xOffset += w;
            }
            else
            {
                yOffset += h;
            }
        }
    }


    void GridLayoutManager::GetOuterRects(const Rect & inRect,
                                          const std::vector<SizeInfo> & inColWidths,
                                          const std::vector<SizeInfo> & inRowHeights,
                                          GenericGrid<Rect> & outRects)
    {
        std::vector<int> colSizes;
        LinearLayoutManager::GetSizes(inRect.width(), inColWidths, colSizes);

        std::vector<int> rowSizes;
        LinearLayoutManager::GetSizes(inRect.height(), inRowHeights, rowSizes);

        int offsetX = inRect.x();
        int offsetY = inRect.y();
        for (size_t colIdx = 0; colIdx != outRects.numColumns(); ++colIdx)
        {
            int width = colSizes[colIdx];
            for (size_t rowIdx = 0; rowIdx != outRects.numRows(); ++rowIdx)
            {
                int height = rowSizes[rowIdx];
                outRects.set(rowIdx, colIdx, Rect(offsetX, offsetY, width, height));
                offsetY += height;
            }
            offsetX += width;
            offsetY = inRect.y();
        }
    }

    void GridLayoutManager::GetInnerRects(const GenericGrid<Rect> & inOuterRects,
                                          const GenericGrid<CellInfo> & inWidgetInfos,
                                          GenericGrid<Rect> & outInnerRects)
    {
        for (size_t rowIdx = 0; rowIdx != inOuterRects.numRows(); ++rowIdx)
        {
            for (size_t colIdx = 0; colIdx != inOuterRects.numColumns(); ++colIdx)
            {
                const Rect & outerRect = inOuterRects.get(rowIdx, colIdx);
                const CellInfo & info = inWidgetInfos.get(rowIdx, colIdx);

                int x = outerRect.x();
                int y = outerRect.y();
                
                // The cell must not exceed the boundaries of its outer rect.
                int w = std::min<int>(info.MinWidgetWidth, outerRect.width());
                int h = std::min<int>(info.MinWidgetHeight, outerRect.height());

                if (info.ColAlign == Center)
                {
                    x = x + (outerRect.width() - info.MinWidgetWidth)/2;
                }
                else if (info.ColAlign == End)
                {
                    x = x + outerRect.width() - info.MinWidgetWidth;
                }
                else if (info.ColAlign == Stretch)
                {
                    w = outerRect.width();
                }

                if (info.RowAlign == Center)
                {
                    y = y + (outerRect.height() - info.MinWidgetHeight)/2;
                }
                else if (info.RowAlign == End)
                {
                    y = y + outerRect.height() - info.MinWidgetHeight;
                }
                else if (info.RowAlign == Stretch)
                {
                    h = outerRect.height();
                }
                outInnerRects.set(rowIdx, colIdx, Rect(x, y, w, h));
            }
        }
    }


} // namespace XulWin
