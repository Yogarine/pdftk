/*
 * $Id: ImgRaw.java,v 1.29 2002/08/23 08:59:32 blowagie Exp $
 * $Name:  $
 *
 * Copyright 2000, 2001, 2002 by Paulo Soares.
 *
 *
 * The Original Code is 'iText, a free JAVA-PDF library'.
 *
 * The Initial Developer of the Original Code is Bruno Lowagie. Portions created by
 * the Initial Developer are Copyright (C) 1999, 2000, 2001, 2002 by Bruno Lowagie.
 * All Rights Reserved.
 * Co-Developer of the code is Paulo Soares. Portions created by the Co-Developer
 * are Copyright (C) 2000, 2001, 2002 by Paulo Soares. All Rights Reserved.
 *
 * Contributor(s): all the names of the contributors are added in the source code
 * where applicable.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * If you didn't download this code from the following link, you should check if
 * you aren't using an obsolete version:
 * http://www.lowagie.com/iText/
 */

package com.lowagie.text;

import java.net.URL;

/**
 * Raw Image data that has to be inserted into the document
 *
 * @see		Element
 * @see		Image
 *
 * @author  Paulo Soares
 */

public class ImgRaw extends Image implements Element {

    ImgRaw(Image image) {
        super(image);
    }

/** Creats an Image in raw mode.
 *
 * @param width the exact width of the image
 * @param height the exact height of the image
 * @param components 1,3 or 4 for GrayScale, RGB and CMYK
 * @param bpc bits per component. Must be 1,2,4 or 8
 * @param data the image data
 * @throws BadElementException on error
 */
    
    public ImgRaw(int width, int height, int components, int bpc, byte[] data) throws BadElementException{
        super((URL)null);
        type = IMGRAW;
        scaledHeight = height;
        setTop(scaledHeight);
        scaledWidth = width;
        setRight(scaledWidth);
        if (components != 1 && components != 3 && components != 4)
            throw new BadElementException("Components must be 1, 3, or 4.");
        if (bpc != 1 && bpc != 2 && bpc != 4 && bpc != 8)
            throw new BadElementException("Bits-per-component must be 1, 2, 4, or 8.");
        colorspace = components;
        this.bpc = bpc;
        rawData = data;
        plainWidth = width();
        plainHeight = height();
    }
}