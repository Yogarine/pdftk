/*
 * $Id: Watermark.java,v 1.52 2004/12/14 11:52:47 blowagie Exp $
 * $Name:  $
 *
 * Copyright 2000, 2001, 2002 by Bruno Lowagie.
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

import java.net.MalformedURLException;

/**
 * A <CODE>Watermark</CODE> is a graphic element (GIF or JPEG)
 * that is shown on a certain position on each page.
 *
 * @see		Element
 * @see		Jpeg
 */

public class Watermark extends Image implements Element {
    
    // membervariables
    
/** This is the offset in x-direction of the Watermark. */
    private float offsetX = 0;
    
/** This is the offset in y-direction of the Watermark. */
    private float offsetY = 0;
    
    // Constructors
    
/**
 * Constructs a <CODE>Watermark</CODE>-object, using an <CODE>Image</CODE>.
 *
 * @param		image		an <CODE>Image</CODE>-object
 * @param		offsetX		the offset in x-direction
 * @param		offsetY		the offset in y-direction
 * @throws MalformedURLException
 */
    
    public Watermark(Image image, float offsetX, float offsetY) throws MalformedURLException {
        super(image);
        this.offsetX = offsetX;
        this.offsetY = offsetY;
    }
    
    // implementation of the Element interface
    
/**
 * Gets the type of the text element.
 *
 * @return	a type
 */
    
    public int type() {
        return type;
    }
    
    // methods to retrieve information
    
/**
 * Returns the offset in x direction.
 *
 * @return		an offset
 */
    
    public float offsetX() {
        return offsetX;
    }
    
/**
 * Returns the offset in y direction.
 *
 * @return		an offset
 */
    
    public float offsetY() {
        return offsetY;
    }
}
