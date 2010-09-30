/*
 * $Id: Chapter.java,v 1.69 2005/04/13 09:51:14 blowagie Exp $
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
 *
 */

package com.lowagie.text;

import java.util.ArrayList;
import java.util.Properties;

/**
 * A <CODE>Chapter</CODE> is a special <CODE>Section</CODE>.
 * <P>
 * A chapter number has to be created using a <CODE>Paragraph</CODE> as title
 * and an <CODE>int</CODE> as chapter number. The chapter number is shown be
 * default. If you don't want to see the chapter number, you have to set the
 * numberdepth to <VAR>0</VAR>.
 * <P>
 * Example:
 * <BLOCKQUOTE><PRE>
 * Paragraph title2 = new Paragraph("This is Chapter 2", FontFactory.getFont(FontFactory.HELVETICA, 18, Font.BOLDITALIC, new Color(0, 0, 255)));
 * <STRONG>Chapter chapter2 = new Chapter(title2, 2);</STRONG>
 * <STRONG>chapter2.setNumberDepth(0);</STRONG>
 * Paragraph someText = new Paragraph("This is some text");
 * <STRONG>chapter2.add(someText);</STRONG>
 * Paragraph title21 = new Paragraph("This is Section 1 in Chapter 2", FontFactory.getFont(FontFactory.HELVETICA, 16, Font.BOLD, new Color(255, 0, 0)));
 * Section section1 = <STRONG>chapter2.addSection(title21);</STRONG>
 * Paragraph someSectionText = new Paragraph("This is some silly paragraph in a chapter and/or section. It contains some text to test the functionality of Chapters and Section.");
 * section1.add(someSectionText);
 * </PRE></BLOCKQUOTE>
 */

public class Chapter extends Section implements TextElementArray {
    
    // constructors
    
	/**
	 * Constructs a new <CODE>Chapter</CODE>.
	 *
	 * @param	title		the Chapter title (as a <CODE>Paragraph</CODE>)
	 * @param	number		the Chapter number
     */
    
    public Chapter(Paragraph title, int number) {
        super(title, 1);
        numbers = new ArrayList();
        numbers.add(new Integer(number));
    }
    
/**
 * Constructs a new <CODE>Chapter</CODE>.
 *
 * @param	title		the Chapter title (as a <CODE>String</CODE>)
 * @param	number		the Chapter number
 */
    
    public Chapter(String title, int number) {
        this(new Paragraph(title), number);
    }
    
/**
 * Creates a new <CODE>Chapter</CODE> following a set of attributes.
 *
 * @param	attributes	the attributes
 * @param number a userdefined Chapter number
 */
    
    public Chapter(Properties attributes, int number) {
        this(new Paragraph(""), number);
        
        String value;
        if ((value = (String)attributes.remove(ElementTags.NUMBERDEPTH)) != null) {
            setNumberDepth(Integer.parseInt(value));
        }
        if ((value = (String)attributes.remove(ElementTags.INDENT)) != null) {
            setIndentation(Float.valueOf(value + "f").floatValue());
        }
        if ((value = (String)attributes.remove(ElementTags.INDENTATIONLEFT)) != null) {
            setIndentationLeft(Float.valueOf(value + "f").floatValue());
        }
        if ((value = (String)attributes.remove(ElementTags.INDENTATIONRIGHT)) != null) {
            setIndentationRight(Float.valueOf(value + "f").floatValue());
        }
    }
    
    // implementation of the Element-methods
    
/**
 * Gets the type of the text element.
 *
 * @return	a type
 */
    
    public int type() {
        return Element.CHAPTER;
    }
    
    // methods
    
/**
 * Checks if a given tag corresponds with this object.
 *
 * @param   tag     the given tag
 * @return  true if the tag corresponds
 */
    
    public static boolean isTag(String tag) {
        return ElementTags.CHAPTER.equals(tag);
    }
}