/*
 * $Id: Row.java,v 1.63 2005/04/13 09:17:11 blowagie Exp $
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

import java.util.ArrayList;
import java.util.Properties;
import java.util.Set;

/**
 * A <CODE>Row</CODE> is part of a <CODE>Table</CODE>
 * and contains some <CODE>Cells</CODE>.
 * <P>
 * All <CODE>Row</CODE>s are constructed by a <CODE>Table</CODE>-object.
 * You don't have to construct any <CODE>Row</CODE> yourself.
 * In fact you can't construct a <CODE>Row</CODE> outside the package.
 * <P>
 * Since a <CODE>Cell</CODE> can span several rows and/or columns
 * a row can contain reserved space without any content.
 *
 * @see   Element
 * @see   Cell
 * @see   Table
 */

public class Row implements Element, MarkupAttributes {
    
    // membervariables
    
/** id of a null element in a Row*/
    public static final int NULL = 0;
    
/** id of the Cell element in a Row*/
    public static final int CELL = 1;
    
/** id of the Table element in a Row*/
    public static final int TABLE = 2;
    
/** This is the number of columns in the <CODE>Row</CODE>. */
    protected int columns;
    
/** This is a valid position the <CODE>Row</CODE>. */
    protected int currentColumn;
    
/** This is the array that keeps track of reserved cells. */
    protected boolean[] reserved;
    
/** This is the array of Objects (<CODE>Cell</CODE> or <CODE>Table</CODE>). */
    protected Object[] cells;
    
/** This is the vertical alignment. */
    protected int horizontalAlignment;
    
/** This is the vertical alignment. */
    protected int verticalAlignment;

/** Contains extra markupAttributes */
    protected Properties markupAttributes;
    
    // constructors
    
/**
 * Constructs a <CODE>Row</CODE> with a certain number of <VAR>columns</VAR>.
 *
 * @param columns   a number of columns
 */
 
    protected Row(int columns) {
        this.columns = columns;
        reserved = new boolean[columns];
        cells = new Object[columns];
        currentColumn = 0;
    }
    
    // implementation of the Element-methods
    
/**
 * Processes the element by adding it (or the different parts) to a
 * <CODE>ElementListener</CODE>.
 *
 * @param listener  an <CODE>ElementListener</CODE>
 * @return  <CODE>true</CODE> if the element was processed successfully
 */
    
    public boolean process(ElementListener listener) {
        try {
            return listener.add(this);
        }
        catch(DocumentException de) {
            return false;
        }
    }
    
/**
 * Gets the type of the text element.
 *
 * @return  a type
 */
    
    public int type() {
        return Element.ROW;
    }
    
/**
 * Gets all the chunks in this element.
 *
 * @return  an <CODE>ArrayList</CODE>
 */
    
    public ArrayList getChunks() {
        return new ArrayList();
    }
    
/**
 * Returns a <CODE>Row</CODE> that is a copy of this <CODE>Row</CODE>
 * in which a certain column has been deleted.
 *
 * @param column  the number of the column to delete
 */
    
    void deleteColumn(int column) {
        if ((column >= columns) || (column < 0)) {
            throw new IndexOutOfBoundsException("getCell at illegal index : " + column);
        }
        columns--;
        boolean newReserved[] = new boolean[columns];
        Object newCells[] = new Cell[columns];
        
        for (int i = 0; i < column; i++) {
            newReserved[i] = reserved[i];
            newCells[i] = cells[i];
            if (newCells[i] != null && (i + ((Cell) newCells[i]).colspan() > column)) {
                ((Cell) newCells[i]).setColspan(((Cell) cells[i]).colspan() - 1);
            }
        }
        for (int i = column; i < columns; i++) {
            newReserved[i] = reserved[i + 1];
            newCells[i] = cells[i + 1];
        }
        if (cells[column] != null && ((Cell) cells[column]).colspan() > 1) {
            newCells[column] = cells[column];
            ((Cell) newCells[column]).setColspan(((Cell) newCells[column]).colspan() - 1);
        }
        reserved = newReserved;
        cells = newCells;
    }
    
    // methods
    
/**
 * Adds a <CODE>Cell</CODE> to the <CODE>Row</CODE>.
 *
 * @param       element the element to add (currently only Cells and Tables supported)
 * @return      the column position the <CODE>Cell</CODE> was added,
 *                      or <CODE>-1</CODE> if the <CODE>element</CODE> couldn't be added.
 */
    
    int addElement(Object element) {
        return addElement(element, currentColumn);
    }
    
/**
 * Adds an element to the <CODE>Row</CODE> at the position given.
 *
 * @param       element the element to add. (currently only Cells and Tables supported
 * @param       column  the position where to add the cell.
 * @return      the column position the <CODE>Cell</CODE> was added,
 *                      or <CODE>-1</CODE> if the <CODE>Cell</CODE> couldn't be added.
 */
    
    int addElement(Object element, int column) {
        if (element == null) throw new NullPointerException("addCell - null argument");
        if ((column < 0) || (column > columns)) throw new IndexOutOfBoundsException("addCell - illegal column argument");
        if ( !((getObjectID(element) == CELL) || (getObjectID(element) == TABLE)) ) throw new IllegalArgumentException("addCell - only Cells or Tables allowed");
        
        int lColspan = ( (Cell.class.isInstance(element)) ? ((Cell) element).colspan() : 1);
        
        if ( reserve(column, lColspan) == false ) {
            return -1;
        }
        
        cells[column] = element;
        currentColumn += lColspan - 1;
        
        return column;
    }
    
/**
 * Puts <CODE>Cell</CODE> to the <CODE>Row</CODE> at the position given, doesn't reserve colspan.
 *
 * @param   aElement    the cell to add.
 * @param   column  the position where to add the cell.
 */
    
    void setElement(Object aElement, int column) {
        if (reserved[column] == true) throw new IllegalArgumentException("setElement - position already taken");
        
        cells[column] = aElement;
        if (aElement != null) {
            reserved[column] = true;
        }
    }
    
/**
 * Reserves a <CODE>Cell</CODE> in the <CODE>Row</CODE>.
 *
 * @param   column  the column that has to be reserved.
 * @return  <CODE>true</CODE> if the column was reserved, <CODE>false</CODE> if not.
 */
    
    boolean reserve(int column) {
        return reserve(column, 1);
    }
    
    
/**
 * Reserves a <CODE>Cell</CODE> in the <CODE>Row</CODE>.
 *
 * @param   column  the column that has to be reserved.
 * @param   size    the number of columns
 * @return  <CODE>true</CODE> if the column was reserved, <CODE>false</CODE> if not.
 */
    
    boolean reserve(int column, int size) {
        if ((column < 0) || ((column + size) > columns)) throw new IndexOutOfBoundsException("reserve - incorrect column/size");
        
        for(int i=column; i < column + size; i++)
        {
            if (reserved[i] == true) {
                // undo reserve
                for(int j=i; j >= column; j--) {
                    reserved[i] = false;
                }
                return false;
            }
            reserved[i] = true;
        }
        return true;
    }
    
/**
 * Sets the horizontal alignment.
 *
 * @param value the new value
 */
    
    public void setHorizontalAlignment(int value) {
        horizontalAlignment = value;
    }
    
/**
 * Sets the vertical alignment.
 *
 * @param value the new value
 */
    
    public void setVerticalAlignment(int value) {
        verticalAlignment = value;
    }
    
    // methods to retrieve information
    
/**
 * Returns true/false when this position in the <CODE>Row</CODE> has been reserved, either filled or through a colspan of an Element.
 *
 * @param       column  the column.
 * @return      <CODE>true</CODE> if the column was reserved, <CODE>false</CODE> if not.
 */
    
    boolean isReserved(int column) {
        return reserved[column];
    }
    
/**
 * Returns the type-id of the element in a Row.
 *
 * @param       column  the column of which you'd like to know the type
 * @return the type-id of the element in the row
 */
    
    int getElementID(int column) {
        if (cells[column] == null) return NULL;
        else if (Cell.class.isInstance(cells[column])) return CELL;
        else if (Table.class.isInstance(cells[column])) return TABLE;
        
        return -1;
    }
    
    
/**
 * Returns the type-id of an Object.
 *
 * @param       element the object of which you'd like to know the type-id, -1 if invalid
 * @return the type-id of an object
 */
    
    int getObjectID(Object element) {
        if (element == null) return NULL;
        else if (Cell.class.isInstance(element)) return CELL;
        else if (Table.class.isInstance(element)) return TABLE;
        
        return -1;
    }
    
    
/**
 * Gets a <CODE>Cell</CODE> or <CODE>Table</CODE> from a certain column.
 *
 * @param   column  the column the <CODE>Cell/Table</CODE> is in.
 * @return  the <CODE>Cell</CODE>,<CODE>Table</CODE> or <VAR>Object</VAR> if the column was
 *                  reserved or null if empty.
 */
    
    public Object getCell(int column) {
        if ((column < 0) || (column > columns)) {
            throw new IndexOutOfBoundsException("getCell at illegal index :" + column + " max is " + columns);
        }
        return cells[column];
    }
    
/**
 * Checks if the row is empty.
 *
 * @return  <CODE>true</CODE> if none of the columns is reserved.
 */
    
    public boolean isEmpty() {
        for (int i = 0; i < columns; i++) {
            if (cells[i] != null) {
                return false;
            }
        }
        return true;
    }
    
/**
 * Gets the index of the current, valid position
 *
 * @return  a value
 */
    
    int validPosition() {
        return currentColumn;
    }
    
/**
 * Gets the number of columns.
 *
 * @return  a value
 */
    
    public int columns() {
        return columns;
    }
    
/**
 * Gets the horizontal alignment.
 *
 * @return  a value
 */
    
    public int horizontalAlignment() {
        return horizontalAlignment;
    }
    
/**
 * Gets the vertical alignment.
 *
 * @return  a value
 */
    
    public int verticalAlignment() {
        return verticalAlignment;
    }
    
/**
 * Checks if a given tag corresponds with this object.
 *
 * @param   tag     the given tag
 * @return  true if the tag corresponds
 */
    
    public static boolean isTag(String tag) {
        return ElementTags.ROW.equals(tag);
    }
    
    
/**
 * @see com.lowagie.text.MarkupAttributes#setMarkupAttribute(java.lang.String, java.lang.String)
 */
    public void setMarkupAttribute(String name, String value) {
        if (markupAttributes == null) markupAttributes = new Properties();
        markupAttributes.put(name, value);
    }
    
/**
 * @see com.lowagie.text.MarkupAttributes#setMarkupAttributes(java.util.Properties)
 */
    public void setMarkupAttributes(Properties markupAttributes) {
        this.markupAttributes = markupAttributes;
    }
    
/**
 * @see com.lowagie.text.MarkupAttributes#getMarkupAttribute(java.lang.String)
 */
    public String getMarkupAttribute(String name) {
        return (markupAttributes == null) ? null : String.valueOf(markupAttributes.get(name));
    }
    
/**
 * @see com.lowagie.text.MarkupAttributes#getMarkupAttributeNames()
 */
    public Set getMarkupAttributeNames() {
        return Chunk.getKeySet(markupAttributes);
    }
    
/**
 * @see com.lowagie.text.MarkupAttributes#getMarkupAttributes()
 */
    public Properties getMarkupAttributes() {
        return markupAttributes;
    }
}