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

import java.io.*;
import java.net.*;
import com.lowagie.text.pdf.*;
import com.lowagie.text.pdf.codec.postscript.*;
import java.util.StringTokenizer;

/**
 * An <CODE>ImgPostscript</CODE> is the representation of an EPS
 * that has to be inserted into the document
 *
 * @see		Element
 * @see		Image
 */

public class ImgPostscript
extends Image
implements Element {
    
    // Constructors
    
    ImgPostscript(Image image) {
        super(image);
    }
    
    /**
     * Constructs an <CODE>ImgPostscript</CODE>-object, using an <VAR>url</VAR>.
     *
     * @param url the <CODE>URL</CODE> where the image can be found
     * @throws BadElementException on error
     * @throws IOException on error
     */
    
    public ImgPostscript(URL url) throws BadElementException, IOException {
        super(url);
        processParameters();
    }
    
    /**
     * Constructs an <CODE>ImgPostscript</CODE>-object, using a <VAR>filename</VAR>.
     *
     * @param filename a <CODE>String</CODE>-representation of the file that contains the image.
     * @throws BadElementException on error
     * @throws MalformedURLException on error
     * @throws IOException on error
     */
    
    public ImgPostscript(String filename) throws BadElementException,
    MalformedURLException, IOException {
        this(Image.toURL(filename));
    }
    
    /**
     * Constructs an <CODE>ImgPostscript</CODE>-object from memory.
     *
     * @param img the memory image
     * @throws BadElementException on error
     * @throws IOException on error
     */
    
    public ImgPostscript(byte[] img) throws BadElementException, IOException {
        super( (URL)null);
        rawData = img;
        originalData = img;
        processParameters();
    }
    
    /**
     * This method checks if the image is a valid Postscript and processes some parameters.
     * @throws BadElementException
     * @throws IOException
     */
    
    private void processParameters() throws BadElementException, IOException {
        type = IMGTEMPLATE;
        originalType = ORIGINAL_PS;
        InputStream is = null;
        try {
            String errorID;
            if (rawData == null) {
                is = url.openStream();
                errorID = url.toString();
            }
            else {
                is = new java.io.ByteArrayInputStream(rawData);
                errorID = "Byte array";
            }
            String boundingbox=null;
            Reader r = new BufferedReader(new InputStreamReader(is));
            //  StreamTokenizer st = new StreamTokenizer(r);
            while (r.ready()) {
                char c;
                StringBuffer sb = new StringBuffer();
                while ( (c = ( (char) r.read())) != '\n') {
                    sb.append(c);
                }
                //System.out.println("<<" + sb.toString() + ">>");
                if (sb.toString().startsWith("%%BoundingBox:")) {
                    boundingbox = sb.toString();
                    
                }
                if (sb.toString().startsWith("%%TemplateBox:")) {
                    boundingbox = sb.toString();
                }
                if (sb.toString().startsWith("%%EndComments")) {
                    break;
                }
                
            }
            if(boundingbox==null)return;
            StringTokenizer st=new StringTokenizer(boundingbox,": \r\n");
            st.nextElement();
            String xx1=st.nextToken();
            String yy1=st.nextToken();
            String xx2=st.nextToken();
            String yy2=st.nextToken();
            
            int left = Integer.parseInt(xx1);
            int top = Integer.parseInt(yy1);
            int right = Integer.parseInt(xx2);
            int bottom = Integer.parseInt(yy2);
            int inch = 1;
            dpiX = 72;
            dpiY = 72;
            scaledHeight = (float) (bottom - top) / inch *1f;
            scaledHeight=800;
            setTop(scaledHeight);
            scaledWidth = (float) (right - left) / inch * 1f;
            scaledWidth=800;
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
    
    /** Reads the Postscript into a template.
     * @param template the template to read to
     * @throws IOException on error
     * @throws DocumentException on error
     */
    public void readPostscript(PdfTemplate template) throws IOException,
    DocumentException {
        setTemplateData(template);
        template.setWidth(width());
        template.setHeight(height());
        InputStream is = null;
        try {
            if (rawData == null) {
                is = url.openStream();
            }
            else {
                is = new java.io.ByteArrayInputStream(rawData);
            }
            MetaDoPS meta = new MetaDoPS(is, template);
            meta.readAll();
        }
        finally {
            if (is != null) {
                is.close();
            }
        }
    }
}
