/*
 * Copyright 2003 by Michael Niedermair.
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

/**
 * 
 * A special-version of <CODE>LIST</CODE> whitch use greek-letters.
 * 
 * @see com.lowagie.text.List
 * @version 2003-06-22
 * @author Michael Niedermair
 */

public class GreekList extends List {

	/**
	 * UpperCase or LowerCase
	 */
	protected boolean greeklower;

	/**
	 * Initialisierung
	 * 
	 * @param symbolIndent	indent
	 */
	public GreekList(int symbolIndent) {
		super(true, symbolIndent);
		setGreekFont();
	}

	/**
	 * Initialisierung 
	 * @param	greeklower		greek-char in lowercase   
	 * @param 	symbolIndent	indent
	 */
	public GreekList(boolean greeklower, int symbolIndent) {
		super(true, symbolIndent);
		this.greeklower = greeklower;
		setGreekFont();
	}

	/**
	 * change the font to SYMBOL
	 */
	protected void setGreekFont() {
		float fontsize = symbol.font().size();
		symbol.setFont(FontFactory.getFont(FontFactory.SYMBOL, fontsize, Font.NORMAL));
	}

	/**
	 * set the greek-letters to lowercase otherwise to uppercase
	 * 
	 * @param greeklower
	 */
	public void setGreekLower(boolean greeklower) {
		this.greeklower = greeklower;
	}

	/**
	 * Checks if the list is greek-letter with lowercase
	 *
	 * @return	<CODE>true</CODE> if the greek-letter is lowercase, <CODE>false</CODE> otherwise.
	 */
	public boolean isGreekLower() {
		return greeklower;
	}

	/**
	 * Adds an <CODE>Object</CODE> to the <CODE>List</CODE>.
	 *
	 * @param	o	the object to add.
	 * @return true if adding the object succeeded
	 */
	public boolean add(Object o) {
		if (o instanceof ListItem) {
			ListItem item = (ListItem) o;
			Chunk chunk;
			if (greeklower)
				chunk = new Chunk((char) (first + list.size() + 96), symbol.font());
			else
				chunk = new Chunk((char) (first + list.size() + 64), symbol.font());
			chunk.append(".");
			item.setListSymbol(chunk);
			item.setIndentationLeft(symbolIndent);
			item.setIndentationRight(0);
			list.add(item);
		} else if (o instanceof List) {
			List nested = (List) o;
			nested.setIndentationLeft(nested.indentationLeft() + symbolIndent);
			first--;
			return list.add(nested);
		} else if (o instanceof String) {
			return this.add(new ListItem((String) o));
		}
		return false;
	}
}
