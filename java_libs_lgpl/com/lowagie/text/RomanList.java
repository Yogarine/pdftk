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
 * A special-version of <CODE>LIST</CODE> which use roman-letters.
 * 
 * @see com.lowagie.text.List
 * @version 2003-06-22
 * @author Michael Niedermair
 */

public class RomanList extends List {

	/**
	 * UpperCase or LowerCase
	 */
	protected boolean romanlower;

	/**
	 * Initialization
	 * 
	 * @param symbolIndent	indent
	 */
	public RomanList(int symbolIndent) {
		super(true, symbolIndent);
	}

	/**
	 * Initialization 
	 * @param	romanlower		roman-char in lowercase   
	 * @param 	symbolIndent	indent
	 */
	public RomanList(boolean romanlower, int symbolIndent) {
		super(true, symbolIndent);
		this.romanlower = romanlower;
	}

	/**
	 * set the roman-letters to lowercase otherwise to uppercase
	 * 
	 * @param romanlower
	 */
	public void setRomanLower(boolean romanlower) {
		this.romanlower = romanlower;
	}

	/**
	 * Checks if the list is roman-letter with lowercase
	 *
	 * @return	<CODE>true</CODE> if the roman-letter is lowercase, <CODE>false</CODE> otherwise.
	 */
	public boolean isRomanLower() {
		return romanlower;
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
			if (romanlower)
				chunk = new Chunk(toRomanLowerCase(first + list.size()), symbol.font());
			else
				chunk = new Chunk(toRomanUppercase(first + list.size()), symbol.font());
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

	// ****************************************************************************************

	/*
	 * Wandelt eine Integer-Zahl in römische Schreibweise um
	 *
	 * Regeln: http://de.wikipedia.org/wiki/R%F6mische_Ziffern
	 *  
	 * 1. Die Ziffern werden addiert, wobei sie von groß nach klein sortiert sind:
	 *
	 *  XVII = 10+5+1+1=17 
	 *
	 * 2. Eine kleinere Ziffer, die links von einer größeren steht, wird abgezogen:
	 * 
	 *  IV = 5-1=4 
	 *  CM = 1000-100=900 
	 *
	 * 3. Maximal drei gleiche Ziffern stehen nebeneinander (Ausnahme: IIII auf Zifferblaettern von Uhren):
	 * 
	 *  XL = 40 (und nicht XXXX) 
	 *  IX = 9 (und nicht VIIII) 
	 *  Diese "Subtraktionsschreibweise" ist erst im Mittelalter allgemein gebräuchlich geworden. 
	 *  Vorher wurde oft "IIII" für "4" geshrieben. 
	 *
	 * 4. Bei mehreren möglichen Schreibweisen wird in der Regel der kürzesten der Vorzug gegeben:
	 *
	 *  IC = 99 (auch LXLIX) 
	 *  IL = 49 (auch XLIX oder sogar XLVIV) 
	 *  Andererseits gibt es die Vorschrift, nach der ein Symbol, das einen Wert von 10n darstellt, 
	 *  nicht einem Symbol, das einen Wert von 10(n+1) darstellt, direkt voranstehen darf. 
	 *  Nach dieser Regel wäre die Schreibweise "XCIX" für "99" der Schreibweise "IC" vorzuziehen. 
	 *
	 * 5. Die römischen Zahlen V, L und D können nicht größeren Zahlen voran gestellt werden:
	 *
	 *  XCV = 95 (nicht VC) 
	 * 
	 *  Zahlen über 3000 werden dargestellt durch Einkastung der Tausender: |IX|LIV=9054
	 * 
	 *
	 * Zahlen größer als 3.000.000 werden durch Doppelstrich etc. dargestellt.
	 */

	/**
	 * Array with Roman digits.
	 */
	private static final RomanDigit[] roman =
		{
			new RomanDigit('m', 1000, false),
			new RomanDigit('d', 500, false),
			new RomanDigit('c', 100, true),
			new RomanDigit('l', 50, false),
			new RomanDigit('x', 10, true),
			new RomanDigit('v', 5, false),
			new RomanDigit('i', 1, true)};

	/** 
	 * changes an int into a lower case roman number.
	 * @param number the original number
	 * @return the roman number (lower case)
	 */
	public static String toRoman(int number) {
		return toRomanLowerCase(number);
	}

	/** 
	 * Changes an int into an upper case roman number.
	 * @param number the original number
	 * @return the roman number (upper case)
	 */
	public static String toRomanUppercase(int number) {
		return toRomanLowerCase(number).toUpperCase();
	}

	/** 
	 * Changes an int into a lower case roman number.
	 * @param number the original number
	 * @return the roman number (lower case)
	 */
	public static String toRomanLowerCase(int number) {

		// Buffer
		StringBuffer buf = new StringBuffer();

		// kleiner 0 ? Vorzeichen festlegen
		if (number < 0) {
			buf.append('-');
			number = -number;
		}

		// größer 3000
		if (number > 3000) {
			// rekursiver Aufruf (ohne tausender-Bereich)
			buf.append('|');
			buf.append(toRomanLowerCase(number / 1000));
			buf.append('|');
			// tausender-Bereich 
			number = number - (number / 1000) * 1000;
		}

		// Schleife
		int pos = 0;
		while (true) {
			// roman-array durchlaufen
			RomanDigit dig = roman[pos];

			// solange Zahl größer roman-Wert
			while (number >= dig.value) {
				// Zeichen hinzufügen
				buf.append(dig.digit);
				// Wert des Zeichens abziehen
				number -= dig.value;
			}

			// Abbruch
			if (number <= 0) {
				break;
			}
			// pre=false suchen (ab Stelle pos)
			int j = pos;
			while (!roman[++j].pre);

			// neuer Wert größer
			if (number + roman[j].value >= dig.value) {
				// hinzufügen
				buf.append(roman[j].digit).append(dig.digit);
				// Wert vom Rest abziehen
				number -= dig.value - roman[j].value;
			}
			pos++;
		}
		return buf.toString();
	}

	/**
	 * Hilfsklasse für römische Zeichen
	 */
	private static class RomanDigit {

		/** part of a roman number */
		public char digit;

		/** value of the roman digit */
		public int value;

		/** can the digit be used as a prefix */
		public boolean pre;

		/**
		 * Constructs a roman digit
		 * @param digit the roman digit
		 * @param value the value
		 * @param pre can it be used as a prefix
		 */
		RomanDigit(char digit, int value, boolean pre) {
			this.digit = digit;
			this.value = value;
			this.pre = pre;
		}
	}

}