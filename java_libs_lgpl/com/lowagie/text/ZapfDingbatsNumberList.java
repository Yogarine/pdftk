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
 * A special-version of <CODE>LIST</CODE> whitch use zapfdingbats-numbers (1..10).
 * 
 * @see com.lowagie.text.List
 * @version 2003-06-22
 * @author Michael Niedermair
 */

public class ZapfDingbatsNumberList extends List {

	/**
	 * which type
	 */
	protected int type;

	/**
	 * Creates a ZapdDingbatsNumberList
	 * @param type the type of list
	 * @param symbolIndent	indent
	 */
	public ZapfDingbatsNumberList(int type, int symbolIndent) {
		super(true, symbolIndent);
		this.type = type;
		float fontsize = symbol.font().size();
		symbol.setFont(FontFactory.getFont(FontFactory.ZAPFDINGBATS, fontsize, Font.NORMAL));
	}

	/**
	 * set the type 
	 * 
	 * @param type
	 */
	public void setType(int type) {
		this.type = type;
	}

	/**
	 * get the type
	 *
	 * @return	char-number
	 */
	public int getType() {
		return type;
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
			switch (type ) {
				case 0:
					chunk = new Chunk((char)(first + list.size() + 171), symbol.font());
					break;
				case 1:
					chunk = new Chunk((char)(first + list.size() + 181), symbol.font());
					break;
				case 2:
					chunk = new Chunk((char)(first + list.size() + 191), symbol.font());
					break;
				default:
					chunk = new Chunk((char)(first + list.size() + 201), symbol.font());
			}
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
