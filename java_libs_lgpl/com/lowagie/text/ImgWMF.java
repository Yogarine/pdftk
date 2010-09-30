/*
 * $Id: ImgWMF.java,v 1.45 2004/12/14 12:14:31 blowagie Exp $
 * $Name:  $
 *
 * Copyright 1999, 2000, 2001, 2002 by Paulo Soares.
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

import java.io.InputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import com.lowagie.text.pdf.codec.wmf.InputMeta;
import com.lowagie.text.pdf.codec.wmf.MetaDo;
import com.lowagie.text.pdf.*;

/**
 * An <CODE>ImgWMF</CODE> is the representation of a windows metafile
 * that has to be inserted into the document
 *
 * @see		Element
 * @see		Image
 */

public class ImgWMF extends Image implements Element {
    
    // Constructors
    
    ImgWMF(Image image) {
        super(image);
    }
    
    /**
     * Constructs an <CODE>ImgWMF</CODE>-object, using an <VAR>url</VAR>.
     *
     * @param url the <CODE>URL</CODE> where the image can be found
     * @throws BadElementException on error
     * @throws IOException on error
     */
    
    public ImgWMF(URL url) throws BadElementException, IOException {
        super(url);
        processParameters();
    }
    
    /**
     * Constructs an <CODE>ImgWMF</CODE>-object, using a <VAR>filename</VAR>.
     *
     * @param filename a <CODE>String</CODE>-representation of the file that contains the image.
     * @throws BadElementException on error
     * @throws MalformedURLException on error
     * @throws IOException on error
     */
    
    public ImgWMF(String filename) throws BadElementException, MalformedURLException, IOException {
        this(Image.toURL(filename));
    }
    
    /**
     * Constructs an <CODE>ImgWMF</CODE>-object from memory.
     *
     * @param img the memory image
     * @throws BadElementException on error
     * @throws IOException on error
     */
    
    public ImgWMF(byte[] img) throws BadElementException, IOException {
        super((URL)null);
        rawData = img;
        originalData = img;
        processParameters();
    }
    
/**
 * This method checks if the image is a valid WMF and processes some parameters.
 * @throws BadElementException
 * @throws IOException
 */
    
    private void processParameters() throws BadElementException, IOException {
        type = IMGTEMPLATE;
        originalType = ORIGINAL_WMF;
        InputStream is = null;
        try {
            String errorID;
            if (rawData == null){
                is = url.openStream();
                errorID = url.toString();
            }
            else{
                is = new java.io.ByteArrayInputStream(rawData);
                errorID = "Byte array";
            }
            InputMeta in = new InputMeta(is);
            if (in.readInt() != 0x9AC6CDD7)	{
                throw new BadElementException(errorID + " is not a valid placeable windows metafile.");
            }
            in.readWord();
            int left = in.readShort();
            int top = in.readShort();
            int right = in.readShort();
            int bottom = in.readShort();
            int inch = in.readWord();
            dpiX = 72;
            dpiY = 72;
            scaledHeight = (float)(bottom - top) / inch * 72f;
            setTop(scaledHeight);
            scaledWidth = (float)(right - left) / inch * 72f;
            setRight(scaledWidth);
        }
        finally {
            if (is != null) {
                is.close();
            }
            plainWidth = width();
            plainHeight = height();
        }
    }
    
    /** Reads the WMF into a template.
     * @param template the template to read to
     * @throws IOException on error
     * @throws DocumentException on error
     */    
    public void readWMF(PdfTemplate template) throws IOException, DocumentException {
        setTemplateData(template);
        template.setWidth(width());
        template.setHeight(height());
        InputStream is = null;
        try {
            if (rawData == null){
                is = url.openStream();
            }
            else{
                is = new java.io.ByteArrayInputStream(rawData);
            }
            MetaDo meta = new MetaDo(is, template);
            meta.readAll();
        }
        finally {
            if (is != null) {
                is.close();
            }
        }
    }
}
