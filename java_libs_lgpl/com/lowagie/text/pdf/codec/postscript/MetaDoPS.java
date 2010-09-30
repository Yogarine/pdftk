/*
 * $Id: MetaDo.java,v 1.28 2003/05/02 09:01:33 blowagie Exp $
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

package com.lowagie.text.pdf.codec.postscript;

import java.io.*;
import java.awt.*;
import com.lowagie.text.*;
import com.lowagie.text.pdf.*;

public class MetaDoPS {

  public PdfContentByte cb;
  InputStream in;
  int left;
  int top;
  int right;
  int bottom;
  int inch;

  public MetaDoPS(InputStream in, PdfContentByte cb) {
    this.cb = cb;
    this.in = in;
  }

  public void readAll() throws IOException, DocumentException {

    cb.saveState();
    java.awt.Graphics2D g2 = cb.createGraphicsShapes(PageSize.A4.
        width(), PageSize.A4.height());
    try {
      PAContext context = new PAContext( (Graphics2D) g2,
                                        new Dimension( (int) PageSize.A4.width(),
          (int) PageSize.A4.height()));
      context.draw(new BufferedInputStream(in));
      // ( (Graphics2D) backBuffer.getGraphics()).dispose();
      in.close();
    }
    catch (IOException ex) {
      ex.printStackTrace();
    }
    catch (PainterException ex) {
      ex.printStackTrace();
    }

    cb.restoreState();

  }

}
