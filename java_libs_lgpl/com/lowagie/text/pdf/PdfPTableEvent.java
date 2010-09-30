/*
 * $Id: PdfPTableEvent.java,v 1.13 2002/06/20 13:30:25 blowagie Exp $
 * $Name:  $
 *
 * Copyright 2001, 2002 Paulo Soares
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

/** An interface that can be used to retrieve the position of cells in <CODE>PdfPTable</CODE>.
 *
 * @author Paulo Soares (psoares@consiste.pt)
 */
public interface PdfPTableEvent {
    
    /** This method is called at the end of the table rendering. The text or graphics are added to
     * one of the 4 <CODE>PdfContentByte</CODE> contained in
     * <CODE>canvases</CODE>.<br>
     * The indexes to <CODE>canvases</CODE> are:<p>
     * <ul>
     * <li><CODE>PdfPTable.BASECANVAS</CODE> - the original <CODE>PdfContentByte</CODE>. Anything placed here
     * will be under the table.
     * <li><CODE>PdfPTable.BACKGROUNDCANVAS</CODE> - the layer where the background goes to.
     * <li><CODE>PdfPTable.LINECANVAS</CODE> - the layer where the lines go to.
     * <li><CODE>PdfPTable.TEXTCANVAS</CODE> - the layer where the text go to. Anything placed here
     * will be over the table.
     * </ul>
     * The layers are placed in sequence on top of each other.
     * <p>
     * The <CODE>widths</CODE> and <CODE>heights</CODE> have the coordinates of the cells.<br>
     * The size of the <CODE>widths</CODE> array is the number of rows.
     * Each sub-array in <CODE>widths</CODE> corresponds to the x column border positions where
     * the first element is the x coordinate of the left table border and the last
     * element is the x coordinate of the right table border. 
     * If colspan is not used all the sub-arrays in <CODE>widths</CODE>
     * are the same.<br>
     * For the <CODE>heights</CODE> the first element is the y coordinate of the top table border and the last
     * element is the y coordinate of the bottom table border.
     * @param table the <CODE>PdfPTable</CODE> in use
     * @param widths an array of arrays with the cells' x positions. It has the length of the number
     * of rows
     * @param heights an array with the cells' y positions. It has a length of the number
     * of rows + 1
     * @param headerRows the number of rows defined for the header.
     * @param rowStart the first row number after the header
     * @param canvases an array of <CODE>PdfContentByte</CODE>
     */    
    public void tableLayout(PdfPTable table, float widths[][], float heights[], int headerRows, int rowStart, PdfContentByte[] canvases);

}

