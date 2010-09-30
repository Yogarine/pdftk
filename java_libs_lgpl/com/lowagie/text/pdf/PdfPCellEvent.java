/*
 * Copyright 2003 Paulo Soares
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
package com.lowagie.text.pdf;

import com.lowagie.text.Rectangle;

/** An event called for a single cell.
 * @author Paulo Soares (psoares@consiste.pt)
 */
public interface PdfPCellEvent {
    /** This method is called at the end of the cell rendering. The text or graphics are added to
     * one of the 4 <CODE>PdfContentByte</CODE> contained in
     * <CODE>canvases</CODE>.<br>
     * The indexes to <CODE>canvases</CODE> are:<p>
     * <ul>
     * <li><CODE>PdfPTable.BASECANVAS</CODE> - the original <CODE>PdfContentByte</CODE>. Anything placed here
     * will be under the cell.
     * <li><CODE>PdfPTable.BACKGROUNDCANVAS</CODE> - the layer where the background goes to.
     * <li><CODE>PdfPTable.LINECANVAS</CODE> - the layer where the lines go to.
     * <li><CODE>PdfPTable.TEXTCANVAS</CODE> - the layer where the text go to. Anything placed here
     * will be over the cell.
     * </ul>
     * The layers are placed in sequence on top of each other.
     * <p>
     * @param cell the cell
     * @param position the coordinates of the cell
     * @param canvases an array of <CODE>PdfContentByte</CODE>
     */    
    public void cellLayout(PdfPCell cell, Rectangle position, PdfContentByte[] canvases);
}
