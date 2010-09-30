/*
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

import java.awt.color.ICC_Profile;

import com.lowagie.text.ExceptionConverter;

/**
 * A <CODE>PdfICCBased</CODE> defines a ColorSpace
 *
 * @see		PdfStream
 */

class PdfICCBased extends PdfStream {
    
    protected int NumberOfComponents;
    
    PdfICCBased(ICC_Profile profile) {
        super();
        try {
            NumberOfComponents = profile.getNumComponents();
            PdfNumber pNumber = new PdfNumber(NumberOfComponents);
            switch (NumberOfComponents) {
                case 1:
                    put(PdfName.ALTERNATE, PdfName.DEVICEGRAY);
                    break;
                case 3:
                    put(PdfName.ALTERNATE, PdfName.DEVICERGB);
                    break;
                case 4:
                    put(PdfName.ALTERNATE, PdfName.DEVICECMYK);
                    break;
                default:
                    throw new PdfException(NumberOfComponents + " component(s) is not supported in PDF1.4");
            }
            put(PdfName.N, new PdfNumber(NumberOfComponents));
            bytes = profile.getData();
            flateCompress();
        } catch (Exception e) {
            throw new ExceptionConverter(e);
        }
    }
}
