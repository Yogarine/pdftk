/*
 * $Id: HeaderFooter.java,v 1.44 2003/04/29 07:45:47 blowagie Exp $
 * $Name:  $
 *
 * Copyright 1999, 2000, 2001, 2002 by Bruno Lowagie.
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

import com.lowagie.text.Paragraph;
import com.lowagie.text.Phrase;

/**
 * A <CODE>HeaderFooter</CODE>-object is a <CODE>Rectangle</CODe> with text
 * that can be put above and/or below every page.
 * <P>
 * Example:
 * <BLOCKQUOTE><PRE>
 * <STRONG>HeaderFooter header = new HeaderFooter(new Phrase("This is a header."), false);</STRONG>
 * <STRONG>HeaderFooter footer = new HeaderFooter(new Phrase("This is page "), new Phrase("."));</STRONG>
 * document.setHeader(header);
 * document.setFooter(footer);
 * </PRE></BLOCKQUOTE>
 */

public class HeaderFooter extends Rectangle implements MarkupAttributes {
    
    // membervariables
    
/** Does the page contain a pagenumber? */
    private boolean numbered;
    
/** This is the <CODE>Phrase</CODE> that comes before the pagenumber. */
    private Phrase before = null;
    
/** This is number of the page. */
    private int pageN;
    
/** This is the <CODE>Phrase</CODE> that comes after the pagenumber. */
    private Phrase after = null;
    
/** This is alignment of the header/footer. */
    private int alignment;
    
    // constructors
    
/**
 * Constructs a <CODE>HeaderFooter</CODE>-object.
 *
 * @param	before		the <CODE>Phrase</CODE> before the pagenumber
 * @param	after		the <CODE>Phrase</CODE> before the pagenumber
 */
    
    public HeaderFooter(Phrase before, Phrase after) {
        super(0, 0, 0, 0);
        setBorder(TOP + BOTTOM);
        setBorderWidth(1);
        
        numbered = true;
        this.before = before;
        this.after = after;
    }
    
/**
 * Constructs a <CODE>Header</CODE>-object with a pagenumber at the end.
 *
 * @param	before		the <CODE>Phrase</CODE> before the pagenumber
 * @param	numbered	<CODE>true</CODE> if the page has to be numbered
 */
    
    public HeaderFooter(Phrase before, boolean numbered) {
        super(0, 0, 0, 0);
        setBorder(TOP + BOTTOM);
        setBorderWidth(1);
        
        this.numbered = numbered;
        this.before = before;
    }
    
    // methods
    
/**
 * Checks if the HeaderFooter contains a page number.
 *
 * @return  true if the page has to be numbered
 */
    
    public boolean isNumbered() {
        return numbered;
    }
    
/**
 * Gets the part that comes before the pageNumber.
 *
 * @return  a Phrase
 */
    
    public Phrase getBefore() {
        return before;
    }
    
/**
 * Gets the part that comes after the pageNumber.
 *
 * @return  a Phrase
 */
    
    public Phrase getAfter() {
        return after;
    }
    
/**
 * Sets the page number.
 *
 * @param		pageN		the new page number
 */
    
    public void setPageNumber(int pageN) {
        this.pageN = pageN;
    }
    
/**
 * Sets the alignment.
 *
 * @param		alignment	the new alignment
 */
    
    public void setAlignment(int alignment) {
        this.alignment = alignment;
    }

    // methods to retrieve the membervariables
    
/**
 * Gets the <CODE>Paragraph</CODE> that can be used as header or footer.
 *
 * @return		a <CODE>Paragraph</CODE>
 */
    
    public Paragraph paragraph() {
        Paragraph paragraph = new Paragraph(before.leading());
        paragraph.add(before);
        if (numbered) {
            paragraph.addSpecial(new Chunk(String.valueOf(pageN), before.font()));
        }
        if (after != null) {
            paragraph.addSpecial(after);
        }
        paragraph.setAlignment(alignment);
        return paragraph;
    }

    /**
     * Gets the alignment of this HeaderFooter.
     *
     * @return	alignment
     */

        public int alignment() {
            return alignment;
        }

}