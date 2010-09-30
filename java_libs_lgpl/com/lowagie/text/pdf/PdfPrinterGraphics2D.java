/*
 * $Id: PdfPrinterGraphics2D.java,v 1.3 2005/02/17 09:20:53 blowagie Exp $
 * $Name:  $
 *
 * Copyright 2004 Paulo Soares and Alexandru Carstoiu
 *
 *
 * The Original Code is 'iText, a free JAVA-PDF library'.
 *
 * The Initial Developer of the Original Code is Bruno Lowagie. Portions created by
 * the Initial Developer are Copyright (C) 1999-2005 by Bruno Lowagie.
 * All Rights Reserved.
 * Co-Developer of the code is Paulo Soares. Portions created by the Co-Developer
 * are Copyright (C) 2000-2005 by Paulo Soares. All Rights Reserved.
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

import java.awt.print.PrinterGraphics;
import java.awt.print.PrinterJob;

/**
 * This is an extension class for the sole purpose of implementing the
 * {@link java.awt.print.PrinterGraphics PrinterGraphics} interface.
 */
public class PdfPrinterGraphics2D extends PdfGraphics2D implements PrinterGraphics
{
	private PrinterJob printerJob;
	
	public PdfPrinterGraphics2D(PdfContentByte cb, float width, float height, FontMapper fontMapper,
			boolean onlyShapes, boolean convertImagesToJPEG, float quality, PrinterJob printerJob)	{
		super(cb, width, height, fontMapper, onlyShapes, convertImagesToJPEG, quality);
		this.printerJob = printerJob;
	}

	public PrinterJob getPrinterJob()	{
		return printerJob;
	}
}
