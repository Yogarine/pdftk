/* -*- Mode: C++; tab-width: 2; c-basic-offset: 2 -*- */
/*
	pdftk, the PDF Tool Kit
	Copyright (c) 2003, 2004 Sid Steward

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	Visit: http://www.gnu.org/licenses/gpl.txt
	for more details on this license.

	Visit: http://www.pdftk.com for the latest information on pdftk

	Please contact Sid Steward with bug reports:
	ssteward at AccessPDF dot com
*/

// Tell C++ compiler to use Java-style exceptions.
#pragma GCC java_exceptions

#include <gcj/cni.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <java/lang/System.h>
#include <java/lang/Throwable.h>
#include <java/lang/String.h>
#include <java/io/IOException.h>
#include <java/io/PrintStream.h>
#include <java/io/FileOutputStream.h>
#include <java/util/Vector.h>
#include <java/util/ArrayList.h>
#include <java/util/Iterator.h>
#include <java/util/HashMap.h>

#include "com/lowagie/text/Document.h"
#include "com/lowagie/text/Rectangle.h"
#include "com/lowagie/text/pdf/PdfName.h"
#include "com/lowagie/text/pdf/PdfString.h"
#include "com/lowagie/text/pdf/PdfNumber.h"
#include "com/lowagie/text/pdf/PdfArray.h"
#include "com/lowagie/text/pdf/PdfDictionary.h"
#include "com/lowagie/text/pdf/PdfOutline.h"
#include "com/lowagie/text/pdf/PdfCopy.h"
#include "com/lowagie/text/pdf/PdfReader.h"
#include "com/lowagie/text/pdf/PdfImportedPage.h"
#include "com/lowagie/text/pdf/PdfWriter.h"
#include "com/lowagie/text/pdf/PdfStamperImp.h"
#include "com/lowagie/text/pdf/PdfEncryptor.h"
#include "com/lowagie/text/pdf/PdfNameTree.h"
#include "com/lowagie/text/pdf/FdfReader.h"
#include "com/lowagie/text/pdf/AcroFields.h"
#include "com/lowagie/text/pdf/PdfIndirectReference.h"
#include "com/lowagie/text/pdf/PdfIndirectObject.h"
#include "com/lowagie/text/pdf/PdfFileSpecification.h"
#include "com/lowagie/text/pdf/PdfBoolean.h"

using namespace std;

namespace java {
	using namespace java::lang;
	using namespace java::io;
	using namespace java::util;
}

namespace itext {
	using namespace com::lowagie::text;
	using namespace com::lowagie::text::pdf;
}

#include "pdftk.h"
#include "attachments.h"
#include "report.h"

// store java::PdfReader* here to 
// prevent unwanted garbage collection
//
static java::Vector* g_dont_collect_p= 0;


static void
describe_header();

static void
describe_synopsis();

static void
describe_full();

static void
prompt_for_password( const string pass_name, 
										 const string pass_app,
										 string& password )
{
	cout << "Please enter the " << pass_name << " password to use on " << pass_app << "." << endl;
	cout << "   It can be empty, or have a maximum of 32 characters:" << endl;
	char buff[64];
	cin.getline( buff, 64 );
	password= buff;
	if( 32< password.size() ) { // too long; trim
		cout << "The password you entered was over 32 characters long," << endl;
		cout << "   so I am dropping: \"" << string(password, 32 ) << "\"" << endl;
		password= string( password, 0, 32 );
	}
}

void
prompt_for_filename( const string message,
										 string& fn )
{
	cout << message << endl;
	const int buff_size= 4096;
	char buff[buff_size];
	cin.getline( buff, buff_size );

	// omit enclosing quotes, if present
	if( buff[0] && buff[strlen(buff)- 1]== '"' ) {
		buff[strlen(buff)- 1]= 0;
	}
	if( buff[0]== '"' ) {
		fn= buff+1;
	}
	else {
		fn= buff;
	}

	if( buff_size== fn.size() ) { // might have been too long for buff
		cout << "The name you entered might have exceeded our internal buffer." << endl;
		cout << "   Please review it and make sure it wasn't truncated:" << endl;
		cout << fn << endl;
	}
}

bool
TK_Session::add_reader( InputPdf* input_pdf_p )
{
	bool open_success_b= true;

	try {
		itext::PdfReader* reader= 0;
		if( input_pdf_p->m_filename== "PROMPT" ) {
			prompt_for_filename( "Please enter a filename for an input PDF:",
													 input_pdf_p->m_filename );
		}
		if( input_pdf_p->m_password.empty() ) {
			reader=
				new itext::PdfReader( JvNewStringLatin1( input_pdf_p->m_filename.c_str() ) );
		}
		else {
			if( input_pdf_p->m_password== "PROMPT" ) {
				prompt_for_password( "open", "the input PDF:\n   "+ input_pdf_p->m_filename, input_pdf_p->m_password );
			}
			jbyteArray password= JvNewByteArray( input_pdf_p->m_password.size() );
			memcpy( (char*)(elements(password)), 
							input_pdf_p->m_password.c_str(),
							input_pdf_p->m_password.size() );

			reader= 
				new itext::PdfReader( JvNewStringLatin1( input_pdf_p->m_filename.c_str() ),
															password );
		}
		reader->consolidateNamedDestinations();
		reader->removeUnusedObjects();
		reader->shuffleSubsetNames();

		input_pdf_p->m_num_pages= reader->getNumberOfPages();

		// keep tally of which pages have been laid claim to in this reader;
		// when creating the final PDF, this tally will be decremented
		input_pdf_p->m_readers.push_back( pair< set<jint>, itext::PdfReader* >( set<jint>(), reader ) );

		// store in this java object so the gc can trace it
		g_dont_collect_p->addElement( reader );

		input_pdf_p->m_authorized_b= ( !reader->encrypted || reader->passwordIsOwner );
		if( !input_pdf_p->m_authorized_b ) {
			open_success_b= false;
		}
	}
	catch( java::io::IOException* ioe_p ) { // file open error
		if( ioe_p->getMessage()->equals( JvNewStringLatin1( "Bad password" ) ) ) {
			input_pdf_p->m_authorized_b= false;
		}
		open_success_b= false;
	}
	catch( java::lang::Throwable* t_p ) { // unexpected error
		cerr << "Error: Unexpected Exception in open_reader()" << endl;
		open_success_b= false;
							
		//t_p->printStackTrace(); // debug
	}

	if( !input_pdf_p->m_authorized_b && m_ask_about_warnings_b ) {
		// prompt for a new password
		cerr << "The password you supplied for the input PDF:" << endl;
		cerr << "   " << input_pdf_p->m_filename << endl;
		cerr << "   did not work.  This PDF is encrypted, and you must supply the" << endl;
		cerr << "   owner password to open it.  If it has no owner password, then" << endl;
		cerr << "   enter the user password, instead.  To quit, enter a blank password" << endl;
		cerr << "   at the next prompt." << endl;

		prompt_for_password( "open", "the input PDF:\n   "+ input_pdf_p->m_filename, input_pdf_p->m_password );
		if( !input_pdf_p->m_password.empty() ) { // reset flags try again
			input_pdf_p->m_authorized_b= true;
			return( add_reader(input_pdf_p) ); // <--- recurse, return
		}
	}

	// report
	if( !open_success_b ) { // file open error
		cerr << "Error: Failed to open PDF file: " << endl;
		cerr << "   " << input_pdf_p->m_filename << endl;
		if( !input_pdf_p->m_authorized_b ) {
			cerr << "   OWNER PASSWORD REQUIRED, but not given (or incorrect)" << endl;
		}
	}

	// update session state
	m_authorized_b= m_authorized_b && input_pdf_p->m_authorized_b;

	return open_success_b;
}

bool 
TK_Session::open_input_pdf_readers()
{
	// try opening the input files and init m_input_pdf readers
	bool open_success_b= true;

	if( !m_input_pdf_readers_opened_b ) {
		for( vector< InputPdf >::iterator it= m_input_pdf.begin(); it!= m_input_pdf.end(); ++it ) {
			open_success_b= add_reader( &(*it) ) && open_success_b;
		}
		m_input_pdf_readers_opened_b= open_success_b;
	}

	return open_success_b;
}

static int
copy_downcase( char* ll, int ll_len,
							 char* rr )
{
  int ii= 0;
  for( ; rr[ii] && ii< ll_len- 1; ++ii ) {
    ll[ii]= 
      ( 'A'<= rr[ii] && rr[ii]<= 'Z' ) ? 
      rr[ii]- ('A'- 'a') : 
      rr[ii];
  }

  ll[ii]= 0;

	return ii;
}

TK_Session::keyword
TK_Session::is_keyword( char* ss, int* keyword_len_p )
{
  *keyword_len_p= 0;

  const int ss_copy_max= 256;
  char ss_copy[ss_copy_max]= "";
  int ss_copy_size= copy_downcase( ss_copy, ss_copy_max, ss );

	*keyword_len_p= ss_copy_size; // possibly modified, below

	// operations
  if( strcmp( ss_copy, "cat" )== 0 ) {
    return cat_k;
  }
	else if( strcmp( ss_copy, "burst" )== 0 ) {
		return burst_k;
	}
	else if( strcmp( ss_copy, "filter" )== 0 ) {
		return filter_k;
	}
	else if( strcmp( ss_copy, "dump_data" )== 0 ||
					 strcmp( ss_copy, "dumpdata" )== 0 ||
					 strcmp( ss_copy, "data_dump" )== 0 ||
					 strcmp( ss_copy, "datadump" )== 0 ) {
		return dump_data_k;
	}
	else if( strcmp( ss_copy, "dump_data_fields" )== 0 ) {
		return dump_data_fields_k;
	}
	else if( strcmp( ss_copy, "fill_form" )== 0 ||
					 strcmp( ss_copy, "fillform" )== 0 ) {
		return fill_form_k;
	}
	else if( strcmp( ss_copy, "attach_file" )== 0 ||
					 strcmp( ss_copy, "attach_files" )== 0 ||
					 strcmp( ss_copy, "attachfile" )== 0 ) {
		return attach_file_k;
	}
	else if( strcmp( ss_copy, "unpack_file" )== 0 ||
					 strcmp( ss_copy, "unpack_files" )== 0 ||
					 strcmp( ss_copy, "unpackfiles" )== 0 ) {
		return unpack_files_k;
	}
	else if( strcmp( ss_copy, "update_info" )== 0 ||
					 strcmp( ss_copy, "undateinfo" )== 0 ) {
		return update_info_k;
	}
	else if( strcmp( ss_copy, "background" )== 0 ) {
		// pdftk 1.10: making background an operation
		// (and preserving old behavior for backwards compatibility)
		return background_k;
	}
	
	// cat range keywords
  else if( strncmp( ss_copy, "end", 3 )== 0 ) { // note: strncmp
    *keyword_len_p= 3; // note: fixed size
    return end_k;
  }
  else if( strcmp( ss_copy, "even" )== 0 ) {
    return even_k;
  }
  else if( strcmp( ss_copy, "odd" )== 0 ) {
    return odd_k;
  }

	// file attachment option
	else if( strcmp( ss_copy, "to_page" )== 0 ||
					 strcmp( ss_copy, "topage" )== 0 ) {
		return attach_file_to_page_k;
	}

  else if( strcmp( ss_copy, "output" )== 0 ) {
    return output_k;
  }

	// encryption & decryption; depends on context
	else if( strcmp( ss_copy, "owner_pw" )== 0 ||
					 strcmp( ss_copy, "ownerpw" )== 0 ) {
		return owner_pw_k;
	}
	else if( strcmp( ss_copy, "user_pw" )== 0 ||
					 strcmp( ss_copy, "userpw" )== 0 ) {
		return user_pw_k;
	}
	else if( strcmp( ss_copy, "input_pw" )== 0 ||
					 strcmp( ss_copy, "inputpw" )== 0 ) {
		return input_pw_k;
	}
	else if( strcmp( ss_copy, "allow" )== 0 ) {
		return user_perms_k;
	}

	// expect these only in output section
	else if( strcmp( ss_copy, "encrypt_40bit" )== 0 ||
					 strcmp( ss_copy, "encrypt_40bits" )== 0 ||
					 strcmp( ss_copy, "encrypt40bit" )== 0 ||
					 strcmp( ss_copy, "encrypt40bits" )== 0 ||
					 strcmp( ss_copy, "encrypt40_bit" )== 0 ||
					 strcmp( ss_copy, "encrypt40_bits" )== 0 ||
					 strcmp( ss_copy, "encrypt_40_bit" )== 0 ||
					 strcmp( ss_copy, "encrypt_40_bits" )== 0 ) {
		return encrypt_40bit_k;
	}
	else if( strcmp( ss_copy, "encrypt_128bit" )== 0 ||
					 strcmp( ss_copy, "encrypt_128bits" )== 0 ||
					 strcmp( ss_copy, "encrypt128bit" )== 0 ||
					 strcmp( ss_copy, "encrypt128bits" )== 0 ||
					 strcmp( ss_copy, "encrypt128_bit" )== 0 ||
					 strcmp( ss_copy, "encrypt128_bits" )== 0 ||
					 strcmp( ss_copy, "encrypt_128_bit" )== 0 ||
					 strcmp( ss_copy, "encrypt_128_bits" )== 0 ) {
		return encrypt_128bit_k;
	}
	
	// user permissions; must follow user_perms_k;
	else if( strcmp( ss_copy, "printing" )== 0 ) {
		return perm_printing_k;
	}
	else if( strcmp( ss_copy, "modifycontents" )== 0 ) {
		return perm_modify_contents_k;
	}
	else if( strcmp( ss_copy, "copycontents" )== 0 ) {
		return perm_copy_contents_k;
	}
	else if( strcmp( ss_copy, "modifyannotations" )== 0 ) {
		return perm_modify_annotations_k;
	}
	else if( strcmp( ss_copy, "fillin" )== 0 ) {
		return perm_fillin_k;
	}
	else if( strcmp( ss_copy, "screenreaders" )== 0 ) {
		return perm_screen_readers_k;
	}
	else if( strcmp( ss_copy, "assembly" )== 0 ) {
		return perm_assembly_k;
	}
	else if( strcmp( ss_copy, "degradedprinting" )== 0 ) {
		return perm_degraded_printing_k;
	}
	else if( strcmp( ss_copy, "allfeatures" )== 0 ) {
		return perm_all_k;
	}
	else if( strcmp( ss_copy, "uncompress" )== 0 ) {
		return filt_uncompress_k;
	}
	else if( strcmp( ss_copy, "compress" )== 0 ) {
		return filt_compress_k;
	}
	else if( strcmp( ss_copy, "flatten" )== 0 ) {
		return flatten_k;
	}
	else if( strcmp( ss_copy, "verbose" )== 0 ) {
		return verbose_k;
	}
	else if( strcmp( ss_copy, "dont_ask" )== 0 ||
					 strcmp( ss_copy, "dontask" )== 0 ) {
		return dont_ask_k;
	}
	else if( strcmp( ss_copy, "do_ask" )== 0 ) {
		return do_ask_k;
	}
	
	
  return none_k;
}

bool
TK_Session::is_valid() const
{
	return( m_valid_b &&
					( m_operation== dump_data_k || m_operation== dump_data_fields_k || m_authorized_b ) &&
					!m_input_pdf.empty() &&
					m_input_pdf_readers_opened_b &&

					first_operation_k<= m_operation &&
					m_operation<= final_operation_k &&

					// these op.s require a single input PDF file
					( !( m_operation== burst_k ||
							 m_operation== filter_k ) ||
						( m_input_pdf.size()== 1 ) ) &&

					// these op.s do not require an output filename
					( m_operation== burst_k ||
					  m_operation== dump_data_k ||
						m_operation== dump_data_fields_k ||
						m_operation== unpack_files_k ||
					  !m_output_filename.empty() ) );
}

void
TK_Session::dump_session_data() const
{
	if( !m_verbose_reporting_b )
		return;

	if( !m_input_pdf_readers_opened_b ) {
		cout << "Input PDF Open Errors" << endl;
		return;
	}

	//
	if( is_valid() ) {
		cout << "Command Line Data is valid." << endl;
	}
	else { 
		cout << "Command Line Data is NOT valid." << endl;
	}

	// input files
	cout << endl;
	cout << "Input PDF Filenames & Passwords in Order\n( <filename>[, <password>] ) " << endl;
	if( m_input_pdf.empty() ) {
		cout << "   No input PDF filenames have been given." << endl;
	}
	else {
		for( vector< InputPdf >::const_iterator it= m_input_pdf.begin();
				 it!= m_input_pdf.end(); ++it )
			{
				cout << "   " << it->m_filename;
				if( !it->m_password.empty() ) {
					cout << ", " << it->m_password;
				}

				if( !it->m_authorized_b ) {
					cout << ", OWNER PASSWORD REQUIRED, but not given (or incorrect)";
				}

				cout << endl;
			}
	}

	// operation
	cout << endl;
	cout << "The operation to be performed: " << endl;
	switch( m_operation ) {
	case cat_k:
		cout << "   cat - Catenate given page ranges into a new PDF." << endl;
		break;
	case burst_k:
		cout << "   burst - Split a single, input PDF into individual pages." << endl;
		break;
	case filter_k:
		cout << "   filter - Apply 'filters' to a single, input PDF based on output args." << endl;
		cout << "      (When the operation is omitted, this is the default.)" << endl;
		break;
	case dump_data_k:
		cout << "   dump_data - Report statistics on a single, input PDF." << endl;
		break;
	case dump_data_fields_k:
		cout << "   dump_data_fields - Report form field data on a single, input PDF." << endl;
		break;
	case unpack_files_k:
		cout << "   unpack_files - Copy PDF file attachments into given directory." << endl;
		break;
	case none_k:
		cout << "   NONE - No operation has been given.  See usage instructions." << endl;
		break;
	default:
		cout << "   INTERNAL ERROR - An unexpected operation has been given." << endl;
		break;
	}

	// pages
	/*
	cout << endl;
	cout << "The following pages will be operated on, in the given order." << endl;
	if( m_page_seq.empty() ) {
		cout << "   No pages or page ranges have been given." << endl;
	}
	else {
		for( vector< PageRef >::const_iterator it= m_page_seq.begin();
				 it!= m_page_seq.end(); ++it )
			{
				map< string, InputPdf >::const_iterator jt=
					m_input_pdf.find( it->m_handle );
				if( jt!= m_input_pdf.end() ) {
					cout << "   Handle: " << it->m_handle << "  File: " << jt->second.m_filename;
					cout << "  Page: " << it->m_page_num << endl;
				}
				else { // error
					cout << "   Internal Error: handle not found in m_input_pdf: " << it->m_handle << endl;
				}
			}
	}
	*/

	// output file; may be PDF or text
	cout << endl;
	cout << "The output file will be named:" << endl;
	if( m_output_filename.empty() ) {
		cout << "   No output filename has been given." << endl;
	}
	else {
		cout << "   " << m_output_filename << endl;
	}

	// output encryption
	cout << endl;
	bool output_encrypted_b= 
		m_output_encryption_strength!= none_enc ||
		!m_output_user_pw.empty() ||
		!m_output_owner_pw.empty();

	cout << "Output PDF encryption settings:" << endl;
	if( output_encrypted_b ) {
		cout << "   Output PDF will be encrypted." << endl;

		switch( m_output_encryption_strength ) {
		case none_enc:
			cout << "   Encryption strength not given. Defaulting to: 128 bits." << endl;
			break;
		case bits40_enc:
			cout << "   Given output encryption strength: 40 bits" << endl;
			break;
		case bits128_enc:
			cout << "   Given output encryption strength: 128 bits" << endl;
			break;
		}

		cout << endl;
		{
			using com::lowagie::text::pdf::PdfWriter;

			if( m_output_user_pw.empty() )
				cout << "   No user password given." << endl;
			else
				cout << "   Given user password: " << m_output_user_pw << endl;
			if( m_output_owner_pw.empty() )
				cout << "   No owner password given." << endl;
			else
				cout << "   Given owner password: " << m_output_owner_pw << endl;
			//
			// the printing section: Top Quality or Degraded, but not both;
			// AllowPrinting is a superset of both flag settings
			if( (m_output_user_perms & PdfWriter::AllowPrinting)== PdfWriter::AllowPrinting )
				cout << "   ALLOW Top Quality Printing" << endl;
			else if( (m_output_user_perms & PdfWriter::AllowPrinting)== PdfWriter::AllowDegradedPrinting )
				cout << "   ALLOW Degraded Printing (Top-Quality Printing NOT Allowed)" << endl;
			else
				cout << "   Printing NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowModifyContents)== PdfWriter::AllowModifyContents )
				cout << "   ALLOW Modifying of Contents" << endl;
			else
				cout << "   Modifying of Contents NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowCopy)== PdfWriter::AllowCopy )
				cout << "   ALLOW Copying of Contents" << endl;
			else
				cout << "   Copying of Contents NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowModifyAnnotations)== PdfWriter::AllowModifyAnnotations )
				cout << "   ALLOW Modifying of Annotations" << endl;
			else
				cout << "   Modifying of Annotations NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowFillIn)== PdfWriter::AllowFillIn )
				cout << "   ALLOW Fill-In" << endl;
			else
				cout << "   Fill-In NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowScreenReaders)== PdfWriter::AllowScreenReaders )
				cout << "   ALLOW Screen Readers" << endl;
			else
				cout << "   Screen Readers NOT Allowed" << endl;
			if( (m_output_user_perms & PdfWriter::AllowAssembly)== PdfWriter::AllowAssembly )
				cout << "   ALLOW Assembly" << endl;
			else
				cout << "   Assembly NOT Allowed" << endl;
		}
	}
	else {
		cout << "   Output PDF will not be encrypted." << endl;
	}

	// compression filter
	cout << endl;
	if( m_operation!= filter_k ||
			output_encrypted_b ||
			!( m_output_compress_b ||
				 m_output_uncompress_b ) )
		{
			cout << "No compression or uncompression being performed on output." << endl;
		}
	else {
		if( m_output_compress_b ) {
			cout << "Compression will be applied to some PDF streams." << endl;
		}
		else {
			cout << "Some PDF streams will be uncompressed." << endl;
		}
	}

}

bool
TK_Session::handle_some_output_options( TK_Session::keyword kw, ArgState* arg_state_p )
{
	switch( kw ) {
	case output_k:
		// added this case for the burst operation and "output" support;
		// also helps with backward compatibility of the "background" feature
		// change state
		*arg_state_p= output_filename_e;
		break;

		// state-altering keywords
	case owner_pw_k:
		// change state
		*arg_state_p= output_owner_pw_e;
		break;
	case user_pw_k:
		// change state
		*arg_state_p= output_user_pw_e;
		break;
	case user_perms_k:
		// change state
		*arg_state_p= output_user_perms_e;
		break;

		////
		// no arguments to these keywords, so the state remains unchanged
	case encrypt_40bit_k:
		m_output_encryption_strength= bits40_enc;
		break;
	case encrypt_128bit_k:
		m_output_encryption_strength= bits128_enc;
		break;
	case filt_uncompress_k:
		m_output_uncompress_b= true;
		break;
	case filt_compress_k:
		m_output_compress_b= true;
		break;
	case flatten_k:
		m_output_flatten_b= true;
		break;
	case verbose_k:
		m_verbose_reporting_b= true;
		break;
	case dont_ask_k:
		m_ask_about_warnings_b= false;
		break;
	case do_ask_k:
		m_ask_about_warnings_b= true;
		break;

	case background_k:
		if( m_operation!= filter_k ) { // warning
			cerr << "Warning: the \"background\" output option works only in filter mode." << endl;
			cerr << "  This means it won't work in combination with \"cat\", \"burst\"," << endl;
			cerr << "  \"attach_file\", etc.  To run pdftk in filter mode, simply omit" << endl;
			cerr << "  the operation, e.g.: pdftk in.pdf output out.pdf background back.pdf" << endl;
			cerr << "  Or, use background as an operation; this is the preferred technique:" << endl;
			cerr << "    pdftk in.pdf background back.pdf output out.pdf" << endl;
		}
		// change state
		*arg_state_p= background_filename_e;
		break;

	default: // not handled here; no change to *arg_state_p
		return false;
	}

	return true;
}

TK_Session::TK_Session( int argc, 
												char** argv ) :
	m_valid_b( false ),
	m_authorized_b( true ),
	m_input_pdf_readers_opened_b( false ),
  m_operation( none_k ),
	m_output_user_perms( 0 ),
	m_output_uncompress_b( false ),
	m_output_compress_b( false ),
	m_output_flatten_b( false ),
	m_output_encryption_strength( none_enc ),
	m_verbose_reporting_b( false ),
	m_ask_about_warnings_b( ASK_ABOUT_WARNINGS ), // set default at compile-time
	m_input_attach_file_pagenum( 0 )
{
	TK_Session::ArgState arg_state = input_files_e;

	g_dont_collect_p= new java::Vector();

  bool password_using_handles_not_b= false;
  bool password_using_handles_b= false;
	InputPdfIndex password_input_pdf_index= 0;

  bool fail_b= false;

	// first, look for our "dont_ask" or "do_ask" keywords, since this
	// setting must be known before we begin opening documents, etc.
	for( int ii= 1; ii< argc; ++ii ) {
    int keyword_len= 0;
		keyword kw= is_keyword( argv[ii], &keyword_len );
		if( kw== dont_ask_k ) {
			m_ask_about_warnings_b= false;
		}
		else if( kw== do_ask_k ) {
			m_ask_about_warnings_b= true;
		}
	}

  // iterate over cmd line arguments
  for( int ii= 1; ii< argc && !fail_b && arg_state!= done_e; ++ii ) {
    int keyword_len= 0;
    keyword arg_keyword= is_keyword( argv[ii], &keyword_len );

    switch( arg_state ) {

    case input_files_e: 
		case input_pw_e: {
			// look for keywords that would advance our state,
			// and then handle the specifics of the above cases

			if( arg_keyword== input_pw_k ) { // input PDF passwords keyword

				arg_state= input_pw_e;
			}
      else if( arg_keyword== cat_k ) {
				m_operation= cat_k;
				arg_state= page_seq_e; // collect page sequeces
      }
      else if( arg_keyword== burst_k ) {
				m_operation= burst_k;
				arg_state= output_args_e; // makes "output <fn>" bit optional
      }
			else if( arg_keyword== filter_k ) {
				m_operation= filter_k;
				arg_state= output_e; // look for an output filename
			}
			else if( arg_keyword== dump_data_k ) {
				m_operation= dump_data_k;
				arg_state= output_e;
			}
			else if( arg_keyword== dump_data_fields_k ) {
				m_operation= dump_data_fields_k;
				arg_state= output_e;
			}
			else if( arg_keyword== fill_form_k ) {
				m_operation= filter_k;
				arg_state= form_data_filename_e; // look for an FDF filename
			}
			else if( arg_keyword== attach_file_k ) {
				m_operation= filter_k;
				arg_state= attach_file_filename_e;
			}
			else if( arg_keyword== attach_file_to_page_k ) {
				arg_state= attach_file_pagenum_e;
			}
			else if( arg_keyword== unpack_files_k ) {
				m_operation= unpack_files_k;
				arg_state= output_e;
			}
			else if( arg_keyword== update_info_k ) {
				m_operation= filter_k;
				arg_state= update_info_filename_e;
			}
			else if( arg_keyword== background_k ) {
				m_operation= filter_k;
				arg_state= background_filename_e;
			}
			else if( arg_keyword== output_k ) { // we reached the output section
				arg_state= output_filename_e;
			}
      else if( arg_keyword== none_k ) {
				// here is where the two cases (input_files_e, input_pw_e) diverge

				if( arg_state== input_files_e ) {
					// input_files_e:
					// expecting input handle=filename pairs, or
					// an input filename w/o a handle
					//
					// treat argv[ii] like an optional input handle and filename
					// like this: [<handle>=]<filename>

					char* eq_loc= strchr( argv[ii], '=' );

					if( eq_loc== 0 ) { // no equal sign; no handle

							InputPdf input_pdf;
							input_pdf.m_filename= argv[ii];
							m_input_pdf.push_back( input_pdf );
					}
					else { // use given handle for filename; test, first
						
						if( ( argv[ii]+ 1< eq_loc ) ||
								!( 'A'<= argv[ii][0] && argv[ii][0]<= 'Z' ) ) 
							{ // error
								cerr << "Error: Handle can only be a single, upper-case letter" << endl;
								cerr << "   here: " << argv[ii] << " Exiting." << endl;
								fail_b= true;
							}
						else {
							// look up handle
							map< string, InputPdfIndex >::const_iterator it= 
								m_input_pdf_index.find( string(1, argv[ii][0]) );

							if( it!= m_input_pdf_index.end() ) { // error: alreay in use
								cerr << "Error: Handle given here: " << endl;
								cerr << "      " << argv[ii] << endl;
								cerr << "   is already associated with: " << endl;
								cerr << "      " << m_input_pdf[it->second].m_filename << endl;
								cerr << "   Exiting." << endl;
								fail_b= true;
							}
							else { // add handle/filename association
								*eq_loc= 0;

								InputPdf input_pdf;
								input_pdf.m_filename= eq_loc+ 1;
								m_input_pdf.push_back( input_pdf );

								m_input_pdf_index[ string(1, argv[ii][0]) ]= m_input_pdf.size()- 1;
							}
						}
					}
				} // end: arg_state== input_files_e
				else if( arg_state== input_pw_e ) {
					// expecting input handle=password pairs, or
					// an input PDF password w/o a handle
					//
					// treat argv[ii] like an input handle and password
					// like this <handle>=<password>; if no handle is
					// given, assign passwords to input in order;

					char* eq_loc= strchr( argv[ii], '=' );

					if( eq_loc== 0 ) { // no equal sign; try using default handles
						if( password_using_handles_b ) { // error: expected a handle
							cerr << "Error: Expected a user-supplied handle for this input" << endl;
							cerr << "   PDF password: " << argv[ii] << endl << endl;
							cerr << "   Handles must be supplied with ~all~ input" << endl;
							cerr << "   PDF passwords, or with ~no~ input PDF passwords." << endl;
							cerr << "   If no handles are supplied, then passwords are applied" << endl;
							cerr << "   according to input PDF order." << endl << endl;
							cerr << "   Handles are given like this: <handle>=<password>, and" << endl;
							cerr << "   they must be single, upper case letters, like: A, B, etc." << endl;
							fail_b= true;
						}
						else {
							password_using_handles_not_b= true;

							if( password_input_pdf_index< m_input_pdf.size() ) {
								m_input_pdf[password_input_pdf_index++].m_password= argv[ii]; // set
							}
							else { // error
								cerr << "Error: more input passwords than input PDF documents." << endl;
								cerr << "   Exiting." << endl;
								fail_b= true;
							}
						}
					}
					else { // use given handle for password
						if( password_using_handles_not_b ) { // error; remark and set fail_b
							cerr << "Error: Expected ~no~ user-supplied handle for this input" << endl;
							cerr << "   PDF password: " << argv[ii] << endl << endl;
							cerr << "   Handles must be supplied with ~all~ input" << endl;
							cerr << "   PDF passwords, or with ~no~ input PDF passwords." << endl;
							cerr << "   If no handles are supplied, then passwords are applied" << endl;
							cerr << "   according to input PDF order." << endl << endl;
							cerr << "   Handles are given like this: <handle>=<password>, and" << endl;
							cerr << "   they must be single, upper case letters, like: A, B, etc." << endl;
							fail_b= true;
						}
						else if( argv[ii]+ 0== eq_loc ) { // error
							cerr << "Error: No user-supplied handle found" << endl;
							cerr << "   at: " << argv[ii] << " Exiting." << endl;
							fail_b= true;
						}
						else if( ( argv[ii]+ 1< eq_loc ) ||
										 !( 'A'<= argv[ii][0] && argv[ii][0]<= 'Z' ) ) 
							{ // error
								cerr << "Error: Handle can only be a single, upper-case letter" << endl;
								cerr << "   here: " << argv[ii] << " Exiting." << endl;
								fail_b= true;
							}
						else {
							password_using_handles_b= true;

							// look up this handle
							map< string, InputPdfIndex >::const_iterator it= 
								m_input_pdf_index.find( string(1, argv[ii][0]) );
							if( it!= m_input_pdf_index.end() ) { // found

								if( m_input_pdf[it->second].m_password.empty() ) {
									m_input_pdf[it->second].m_password= eq_loc+ 1; // set
								}
								else { // error: password already given
									cerr << "Error: Handle given here: " << endl;
									cerr << "      " << argv[ii] << endl;
									cerr << "   is already associated with this password: " << endl;
									cerr << "      " << m_input_pdf[it->second].m_password << endl;
									cerr << "   Exiting." << endl;
									fail_b= true;
								}
							}
							else { // error: no input file matches this handle
								cerr << "Error: Password handle: " << argv[ii] << endl;
								cerr << "   is not associated with an input PDF file." << endl;
								cerr << "   Exiting." << endl;
								fail_b= true;
							}
						}
					}
				} // end: arg_state== input_pw_e
				else { // error
					cerr << "Error: Internal error: unexpected arg_state.  Exiting." << endl;
					fail_b= true;
				}
			}
			else { // error: unexpected keyword; remark and set fail_b
				cerr << "Error: Unexpected command-line data: " << endl;
				cerr << "      " << argv[ii] << endl;
				if( arg_state== input_files_e ) {
					cerr << "   where we were expecting an input PDF filename," << endl;
					cerr << "   operation (e.g. \"cat\") or \"input_pw\".  Exiting." << endl;
				}
				else {
					cerr << "   where we were expecting an input PDF password" << endl;
					cerr << "   or operation (e.g. \"cat\").  Exiting." << endl;
				}
				fail_b= true;
			}
    }
    break;

    case page_seq_e: {
      if( m_page_seq.empty() ) {
				// we just got here; validate input filenames

				if( m_input_pdf.empty() ) { // error; remark and set fail_b
					cerr << "Error: No input files.  Exiting." << endl;
					fail_b= true;
					break;
				}

				// try opening input PDF readers
				if( !open_input_pdf_readers() ) { // failure
					fail_b= true;
					break;
				}
      } // end: first pass init. pdf files

      if( arg_keyword== output_k ) {
				arg_state= output_filename_e; // advance state
      }
			else if( arg_keyword== none_k || 
							 arg_keyword== end_k )
				{ // treat argv[ii] like a page sequence

					bool even_pages_b= false;
					bool odd_pages_b= false;
					int jj= 0;

					InputPdfIndex range_pdf_index= 0; { // defaults to first input document
						string handle;
						// in anticipation of multi-character handles (?)
						for( ; argv[ii][jj] && isupper(argv[ii][jj]); ++jj ) {
							handle.push_back( argv[ii][jj] );
						}
						if( !handle.empty() ) {
							// validate handle
							map< string, InputPdfIndex >::const_iterator it= m_input_pdf_index.find( handle );
							if( it== m_input_pdf_index.end() ) { // error
								cerr << "Error: Given handle has no associated file: " << endl;
								cerr << "   " << handle << ", used here: " << argv[ii] << endl;
								cerr << "   Exiting." << endl;
								fail_b= true;
								break;
							}
							else {
								range_pdf_index= it->second;
							}
						}
					}

					char* hyphen_loc= strchr( argv[ii]+ jj, '-' );
					if( hyphen_loc )
						*hyphen_loc= 0;

					////
					// start of page range

					PageNumber page_num_beg= 0;
					for( ; argv[ii][jj] && isdigit(argv[ii][jj]); ++jj ) {
						page_num_beg= page_num_beg* 10+ argv[ii][jj]- '0';
					}

					if( argv[ii][jj] ) { // process possible text keyword in page range start
						if( page_num_beg ) { // error: can't have numbers ~and~ a keyword at the beginning
							cerr << "Error: Unexpected combination of digits and text in" << endl;
							cerr << "   page range start, here: " << argv[ii] << endl;
							cerr << "   Exiting." << endl;
							fail_b= true;
							break;
						}
					
						// read keyword
						int keyword_len= 0;
						keyword arg_keyword= is_keyword( argv[ii]+ jj, &keyword_len );

						if( arg_keyword== end_k ) {
							page_num_beg= m_input_pdf[range_pdf_index].m_num_pages;
						}
						else if( !hyphen_loc ) { // no end of page range given

							// even and odd keywords could be used when referencing
							// an entire document by handle, e.g. Aeven, Aodd
							//
							if( arg_keyword== even_k ) {
								even_pages_b= true;
							}
							else if( arg_keyword== odd_k ) {
								odd_pages_b= true;
							}
							else { // error; unexpected keyword or string
								cerr << "Error: Unexpected text in page reference, here: " << endl;
								cerr << "   " << argv[ii] << endl;
								cerr << "   Exiting." << endl;
								cerr << "   Acceptable keywords, here, are: \"even\", \"odd\", or \"end\"." << endl;
								fail_b= true;
								break;
							}
						}
						else { // error; unexpected keyword or string
							cerr << "Error: Unexpected letters in page range start, here: " << endl;
							cerr << "   " << argv[ii] << endl;
							cerr << "   Exiting." << endl;
							cerr << "   The acceptable keyword, here, is \"end\"." << endl;
							fail_b= true;
							break;
						}

						jj+= keyword_len;
					}
	
					// advance to end of token
					while( argv[ii][jj] ) { 
						++jj;
					}

					////
					// end of page range

					PageNumber page_num_end= 0;

					if( hyphen_loc ) { // process second half of page range
						++jj; // jump over hyphen

						// digits
						for( ; argv[ii][jj] && isdigit(argv[ii][jj]); ++jj ) {
							page_num_end= page_num_end* 10+ argv[ii][jj]- '0';
						}

						// trailing text
						while( argv[ii][jj] ) {

							// read keyword
							int keyword_len= 0;
							keyword arg_keyword= is_keyword( argv[ii]+ jj, &keyword_len );

							if( page_num_end ) {
								if( arg_keyword== even_k ) {
									even_pages_b= true;
								}
								else if( arg_keyword== odd_k ) {
									odd_pages_b= true;
								}
								else { // error
									cerr << "Error: Unexpected text in page range end, here: " << endl;
									cerr << "   " << argv[ii] << endl;
									cerr << "   Exiting." << endl;
									cerr << "   Acceptable keywords, here, are: \"even\" or \"odd\"." << endl;
									fail_b= true;
									break;
								}
							}
							else { // !page_num_end
								if( arg_keyword== end_k ) {
									page_num_end= m_input_pdf[range_pdf_index].m_num_pages;
								}
								else { // error
									cerr << "Error: Unexpected text in page range end, here: " << endl;
									cerr << "   " << argv[ii] << endl;
									cerr << "   Exiting." << endl;
									cerr << "   The acceptable keyword, here, is \"end\"." << endl;
									fail_b= true;
									break;
								}
							}

							jj+= keyword_len;
						}
					}

					////
					// pack this range into our m_page_seq; 

					if( page_num_beg== 0 && page_num_end== 0 ) { // ref the entire document
						page_num_beg= 1;
						page_num_end= m_input_pdf[range_pdf_index].m_num_pages;
					}
					else if( page_num_end== 0 ) { // a single page ref
						page_num_end= page_num_beg;
					}

					vector< PageRef > temp_page_seq;
					bool reverse_sequence_b= ( page_num_end< page_num_beg );
					if( reverse_sequence_b ) { // swap
						PageNumber temp= page_num_end;
						page_num_end= page_num_beg;
						page_num_beg= temp;
					}

					for( PageNumber kk= page_num_beg; kk<= page_num_end; ++kk ) {
						if( (!even_pages_b || !(kk % 2)) &&
								(!odd_pages_b || (kk % 2)) )
							{
								if( 0<= kk && kk<= m_input_pdf[range_pdf_index].m_num_pages ) {

									// look to see if this page of this document
									// has already been referenced; if it has,
									// create a new reader; associate this page
									// with a reader;
									//
									vector< pair< set<jint>, itext::PdfReader* > >::iterator it=
										m_input_pdf[range_pdf_index].m_readers.begin();
									for( ; it!= m_input_pdf[range_pdf_index].m_readers.end(); ++it ) {
										set<jint>::iterator jt= it->first.find( kk );
										if( jt== it->first.end() ) { // kk not assoc. w/ this reader
											it->first.insert( kk ); // create assoc.
											break;
										}
									}
									//
									if( it== m_input_pdf[range_pdf_index].m_readers.end() ) {
										// need to create a new reader for kk
										if( add_reader( &(m_input_pdf[range_pdf_index]) ) ) {
											m_input_pdf[range_pdf_index].m_readers.back().first.insert( kk );
										}
										else {
											cerr << "Internal Error: unable to add reader" << endl;
											fail_b= true;
											break;
										}
									}

									//
									temp_page_seq.push_back( PageRef(range_pdf_index, kk) );

								}
								else { // error; break later to get most feedback
									cerr << "Error: Page number: " << kk << endl;
									cerr << "   does not exist in file: " << m_input_pdf[range_pdf_index].m_filename << endl;
									fail_b= true;
								}
							}
					}
					if( fail_b )
						break;

					if( reverse_sequence_b ) {
						reverse( temp_page_seq.begin(), temp_page_seq.end() );
					}

					m_page_seq.insert( m_page_seq.end(), temp_page_seq.begin(), temp_page_seq.end() );

				}
			else { // error
				cerr << "Error: expecting page ranges.  Instead, I got:" << endl;
				cerr << "   " << argv[ii] << endl;
				fail_b= true;
				break;
			}
		}
    break;

		case form_data_filename_e: {
      if( arg_keyword== none_k )
				{ // treat argv[ii] like an FDF file filename
					
					if( m_form_data_filename.empty() ) {
						m_form_data_filename= argv[ii];
					}
					else { // error
						cerr << "Error: Multiple fill_form filenames given: " << endl;
						cerr << "   " << m_form_data_filename << " and " << argv[ii] << endl;
						cerr << "Exiting." << endl;
						fail_b= true;
						break;
					}

					// advance state
					arg_state= output_e; // look for an output filename
				}
			else { // error
				cerr << "Error: expecting an FDF file filename," << endl;
				cerr << "   instead I got this keyword: " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}
		} // end: case form_data_filename_e
		break;

		case attach_file_filename_e: {
			// keep packing filenames until we reach an expected keyword

			if( arg_keyword== attach_file_to_page_k ) {
				arg_state= attach_file_pagenum_e; // advance state
			}
			else if( arg_keyword== output_k ) {
				arg_state= output_filename_e; // advance state
			}
			else if( arg_keyword== none_k ) { 
				// pack argv[ii] into our list of attachment filenames
				m_input_attach_file_filename.push_back( argv[ii] );
			}
			else { // error
				cerr << "Error: expecting an attachment filename," << endl;
				cerr << "   instead I got this keyword: " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}
		}
		break;

		case attach_file_pagenum_e: {
			if( strcmp(argv[ii], "PROMPT")== 0 ) { // query the user, later
				m_input_attach_file_pagenum= -1;
			}
			else if( strcmp(argv[ii], "end")== 0 ) { // attach to the final page
				m_input_attach_file_pagenum= -2;
			}
			else {
				m_input_attach_file_pagenum= 0;
				for( int jj= 0; argv[ii][jj]; ++jj ) {
					if( !isdigit(argv[ii][jj]) ) { // error
						cerr << "Error: expecting a (1-based) page number.  Instead, I got:" << endl;
						cerr << "   " << argv[ii] << endl;
						cerr << "Exiting." << endl;
						fail_b= true;
						break;
					}

					m_input_attach_file_pagenum= 
						m_input_attach_file_pagenum* 10+ argv[ii][jj]- '0';
				}
			}

			// advance state
			arg_state= output_e; // look for an output filename

		} // end: case attach_file_pagenum_e
		break;

		case update_info_filename_e : {
			if( arg_keyword== none_k ) {
					if( m_update_info_filename.empty() ) {
						m_update_info_filename= argv[ii];
					}
					else { // error
						cerr << "Error: Multiple update_info filenames given: " << endl;
						cerr << "   " << m_update_info_filename << " and " << argv[ii] << endl;
						cerr << "Exiting." << endl;
						fail_b= true;
						break;
					}
				}
			else { // error
				cerr << "Error: expecting an INFO file filename," << endl;
				cerr << "   instead I got this keyword: " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}

			// advance state
			arg_state= output_e; // look for an output filename

		} // end: case update_info_filename_e
		break;

		case output_e: {
			if( m_input_pdf.empty() ) { // error; remark and set fail_b
				cerr << "Error: No input files.  Exiting." << endl;
				fail_b= true;
				break;
			}

      if( arg_keyword== output_k ) {
				arg_state= output_filename_e; // advance state
      }
			else { // error
				cerr << "Error: expecting \"output\" keyword.  Instead, I got:" << endl;
				cerr << "   " << argv[ii] << endl;
				fail_b= true;
				break;
			}
		}
		break;

    case output_filename_e: {
			// we have closed all possible input operations and arguments;
			// see if we should perform any default action based on the input state
			//
			if( m_operation== none_k ) {
				if( 1< m_input_pdf.size() ) {
					// no operation given for multiple input PDF, so combine them
					m_operation= cat_k;
				}
				else {
					m_operation= filter_k;
				}
			}
			
			// try opening input PDF readers (in case they aren't already)
			if( !open_input_pdf_readers() ) { // failure
				fail_b= true;
				break;
			}

			if( m_operation== cat_k &&
					m_page_seq.empty() )
				{ // combining pages, but no sequences given; merge all input PDFs in order
					for( InputPdfIndex ii= 0; ii< m_input_pdf.size(); ++ii ) {
						InputPdf& input_pdf= m_input_pdf[ii];

						for( PageNumber jj= 1; jj<= input_pdf.m_num_pages; ++jj ) {
							m_page_seq.push_back( PageRef( ii, jj ) );
							m_input_pdf[ii].m_readers.back().first.insert( jj ); // mark our claim
						}
					}
				}

			if( m_output_filename.empty() ) {
				m_output_filename= argv[ii];

				if( m_output_filename!= "-" ) { // input and output may both be "-" (stdin and stdout)
					// simple-minded test to see if output matches an input filename
					for( vector< InputPdf >::const_iterator it= m_input_pdf.begin();
							 it!= m_input_pdf.end(); ++it )
						{
							if( it->m_filename== m_output_filename ) {
								cerr << "Error: The given output filename: " << m_output_filename << endl;
								cerr << "   matches an input filename.  Exiting." << endl;
								fail_b= true;
								break;
							}
						}
				}
			}
			else { // error
				cerr << "Error: Multiple output filenames given: " << endl;
				cerr << "   " << m_output_filename << " and " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}

			// advance state
			arg_state= output_args_e;
		}
		break;

		case output_args_e: {
			// output args are order-independent but must follow "output <fn>", if present;
			// we are expecting any of these keywords:
			// owner_pw_k, user_pw_k, user_perms_k ...
			// added output_k case in pdftk 1.10; this permits softer "output <fn>" enforcement
			//

			if( handle_some_output_options( arg_keyword, &arg_state ) ) {
				break;
			}
			else {
				cerr << "Error: Unexpected data in output section: " << endl;
				cerr << "      " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}
		}
		break;

		case output_owner_pw_e: {
			if( m_output_owner_pw.empty() ) {
				if( m_output_user_pw!= argv[ii] ) {
					m_output_owner_pw= argv[ii];
				}
				else { // error: identical user and owner password
					// are interpreted by Acrobat (per the spec.) that
					// the doc has no owner password
					cerr << "Error: The user and owner passwords are the same." << endl;
					cerr << "   PDF Viewers interpret this to mean your PDF has" << endl;
					cerr << "   no owner password, so they must be different." << endl;
					cerr << "   Or, supply no owner password to pdftk if this is" << endl;
					cerr << "   what you desire." << endl;
					cerr << "Exiting." << endl;
					fail_b= true;
					break;
				}
			}
			else { // error: we already have an output owner pw
				cerr << "Error: Multiple output owner passwords given: " << endl;
				cerr << "   " << m_output_owner_pw << " and " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}

			// revert state
			arg_state= output_args_e;
		}
		break;

		case output_user_pw_e: {
			if( m_output_user_pw.empty() ) {
				if( m_output_owner_pw!= argv[ii] ) {
					m_output_user_pw= argv[ii];
				}
				else { // error: identical user and owner password
					// are interpreted by Acrobat (per the spec.) that
					// the doc has no owner password
					cerr << "Error: The user and owner passwords are the same." << endl;
					cerr << "   PDF Viewers interpret this to mean your PDF has" << endl;
					cerr << "   no owner password, so they must be different." << endl;
					cerr << "   Or, supply no owner password to pdftk if this is" << endl;
					cerr << "   what you desire." << endl;
					cerr << "Exiting." << endl;
					fail_b= true;
					break;
				}
			}
			else { // error: we already have an output user pw
				cerr << "Error: Multiple output user passwords given: " << endl;
				cerr << "   " << m_output_user_pw << " and " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}

			// revert state
			arg_state= output_args_e;
		}
		break;

		case output_user_perms_e: {
			using com::lowagie::text::pdf::PdfWriter;

			// we may be given any number of permission arguments,
			// so keep an eye out for other, state-altering keywords
			if( handle_some_output_options( arg_keyword, &arg_state ) ) {
				break;
			}

			switch( arg_keyword ) {

				// possible permissions
			case perm_printing_k:
				// if both perm_printing_k and perm_degraded_printing_k
				// are given, then perm_printing_k wins;
				m_output_user_perms|= 
					PdfWriter::AllowPrinting;
				break;
			case perm_modify_contents_k:
				// Acrobat 5 and 6 don't set both bits, even though
				// they both respect AllowModifyContents --> AllowAssembly;
				// so, no harm in this;
				m_output_user_perms|= 
					( PdfWriter::AllowModifyContents | PdfWriter::AllowAssembly );
				break;
			case perm_copy_contents_k:
				// Acrobat 5 _does_ allow the user to allow copying contents
				// yet hold back screen reader perms; this is counter-intuitive,
				// and Acrobat 6 does not allow Copy w/o SceenReaders;
				m_output_user_perms|= 
					( PdfWriter::AllowCopy | PdfWriter::AllowScreenReaders );
				break;
			case perm_modify_annotations_k:
				m_output_user_perms|= 
					( PdfWriter::AllowModifyAnnotations | PdfWriter::AllowFillIn );
				break;
			case perm_fillin_k:
				m_output_user_perms|= 
					PdfWriter::AllowFillIn;
				break;
			case perm_screen_readers_k:
				m_output_user_perms|= 
					PdfWriter::AllowScreenReaders;
				break;
			case perm_assembly_k:
				m_output_user_perms|= 
					PdfWriter::AllowAssembly;
				break;
			case perm_degraded_printing_k:
				m_output_user_perms|= 
					PdfWriter::AllowDegradedPrinting;
				break;
			case perm_all_k:
				m_output_user_perms= 
					( PdfWriter::AllowPrinting | // top quality printing
						PdfWriter::AllowModifyContents |
						PdfWriter::AllowCopy |
						PdfWriter::AllowModifyAnnotations |
						PdfWriter::AllowFillIn |
						PdfWriter::AllowScreenReaders |
						PdfWriter::AllowAssembly );
				break;

			default: // error: unexpected matter
				cerr << "Error: Unexpected data in output section: " << endl;
				cerr << "      " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}
		}
		break;

		case background_filename_e : {
			if( arg_keyword== none_k ) {
				if( m_background_filename.empty() ) {
					m_background_filename= argv[ii];
				}
				else { // error
					cerr << "Error: Multiple background filenames given: " << endl;
					cerr << "   " << m_background_filename << " and " << argv[ii] << endl;
					cerr << "Exiting." << endl;
					fail_b= true;
					break;
				}
			}
			else { // error
				cerr << "Error: expecting a PDF filename for background operation," << endl;
				cerr << "   instead I got this keyword: " << argv[ii] << endl;
				cerr << "Exiting." << endl;
				fail_b= true;
				break;
			}

			// revert state
			// this is more liberal than used with other operations, since we want
			// to preserve backward-compatibility with pdftk 1.00 where "background"
			// was documented as an output option; in pdftk 1.10 we changed it to
			// an operation
			arg_state= output_args_e;
		}
		break;

    default: { // error
			cerr << "Internal Error: Unexpected arg_state.  Exiting." << endl;
			fail_b= true;
			break;
		}
		break;

    } // end: switch(arg_state)

  } // end: iterate over command-line arguments

	if( fail_b ) {
		cerr << "Errors encountered.  No output created." << endl;
		m_valid_b= false;

		g_dont_collect_p->clear();
		m_input_pdf.erase( m_input_pdf.begin(), m_input_pdf.end() );

		// preserve other data members for diagnostic dump
	}
	else {
		m_valid_b= true;

		if(!m_input_pdf_readers_opened_b ) {
			open_input_pdf_readers();
		}
	}
}

TK_Session::~TK_Session()
{
	g_dont_collect_p->clear();
}

static java::OutputStream*
get_output_stream( string output_filename,
									 bool ask_about_warnings_b )
{
	java::OutputStream* os_p= 0;

	if( output_filename== "PROMPT" ) {
		prompt_for_filename( "Please enter a name for the output:", 
												 output_filename );
	}
	if( output_filename== "-" ) { // stdout
		os_p= java::System::out;
	}
	else {
		if( ask_about_warnings_b ) {
			// test for existing file by this name
			ifstream ifs( output_filename.c_str() );
			if( ifs ) {
				cout << "Warning: the output file: " << output_filename << " already exists.  Overwrite? (y/n)" << endl;
				char buff[64];
				cin.getline( buff, 64 );
				if( buff[0]!= 'y' && buff[0]!= 'Y' ) {
					// recurse; try again
					return get_output_stream( "PROMPT",
																		ask_about_warnings_b );
				}
			}
		}

		// attempt to open the stream
		java::String* jv_output_filename_p=
			JvNewStringLatin1( output_filename.c_str() );
		try {
			os_p= new java::FileOutputStream( jv_output_filename_p );
		}
		catch( java::io::IOException* ioe_p ) { // file open error
			cerr << "Error: Failed to open output file: " << endl;
			cerr << "   " << output_filename << endl;
			cerr << "   No output created." << endl;
			os_p= 0;
		}
	}

	return os_p;
}

////
// when uncompressing a PDF, we add this marker to every page,
// so the PDF is easier to navigate; when compressing a PDF,
// we remove this marker

static char g_page_marker[]= "pdftk_PageNum";
static void
	add_mark_to_page( itext::PdfReader* reader_p,
										jint page_index,
										jint page_num )
{
	itext::PdfName* page_marker_p=
		new itext::PdfName( JvNewStringLatin1(g_page_marker) );
	itext::PdfDictionary* page_p= reader_p->getPageN( page_index );
	if( page_p && page_p->isDictionary() ) {
		page_p->put( page_marker_p, new itext::PdfNumber( page_num ) );
	}
}
static void
	add_marks_to_pages( itext::PdfReader* reader_p )
{
	jint num_pages= reader_p->getNumberOfPages();
	for( jint ii= 1; ii<= num_pages; ++ii ) { // 1-based page ref.s
		add_mark_to_page( reader_p, ii, ii );
	}
}
static void
	remove_mark_from_page( itext::PdfReader* reader_p,
												 jint page_num )
{
	itext::PdfName* page_marker_p=
		new itext::PdfName( JvNewStringLatin1(g_page_marker) );
	itext::PdfDictionary* page_p= reader_p->getPageN( page_num );
	if( page_p && page_p->isDictionary() ) {
		page_p->remove( page_marker_p );
	}
}
static void
	remove_marks_from_pages( itext::PdfReader* reader_p )
{
	jint num_pages= reader_p->getNumberOfPages();
	for( jint ii= 1; ii<= num_pages; ++ii ) { // 1-based page ref.s
		remove_mark_from_page( reader_p, ii );
	}
}

void
TK_Session::create_output()
{
	if( is_valid() ) {

		if( m_verbose_reporting_b ) {
			cout << endl << "Creating Output ..." << endl;
		}

		string creator= "pdftk "+ string(PDFTK_VER)+ " - www.pdftk.com";
		java::String* jv_creator_p= 
			JvNewStringLatin1( creator.c_str() );

		if( m_output_owner_pw== "PROMPT" ) {
			prompt_for_password( "owner", "the output PDF", m_output_owner_pw );
		}
		if( m_output_user_pw== "PROMPT" ) {
			prompt_for_password( "user", "the output PDF", m_output_user_pw );
		}

		jbyteArray output_owner_pw_p= JvNewByteArray( m_output_owner_pw.size() ); {
			jbyte* pw_p= elements(output_owner_pw_p);
			memcpy( pw_p, m_output_owner_pw.c_str(), m_output_owner_pw.size() ); 
		}
		jbyteArray output_user_pw_p= JvNewByteArray( m_output_user_pw.size() ); {
			jbyte* pw_p= elements(output_user_pw_p);
			memcpy( pw_p, m_output_user_pw.c_str(), m_output_user_pw.size() ); 
		}

		try {
			switch( m_operation ) {

			case cat_k : { // catenate pages
				itext::Document* output_doc_p= new itext::Document();

				java::OutputStream* ofs_p= 
					get_output_stream( m_output_filename, 
														 m_ask_about_warnings_b );

				if( !ofs_p ) { // file open error
					break;
				}
				itext::PdfCopy* writer_p= new itext::PdfCopy( output_doc_p, ofs_p );

				output_doc_p->addCreator( jv_creator_p );

				// un/compress output streams?
				if( m_output_uncompress_b ) {
					writer_p->filterStreams= true;
					writer_p->compressStreams= false;
				}
				else if( m_output_compress_b ) {
					writer_p->filterStreams= false;
					writer_p->compressStreams= true;
				}

				// encrypt output?
				if( m_output_encryption_strength!= none_enc ||
						!m_output_owner_pw.empty() || 
						!m_output_user_pw.empty() )
					{
						// if no stregth is given, default to 128 bit,
						// (which is incompatible w/ Acrobat 4)
						bool bit128_b=
							( m_output_encryption_strength!= bits40_enc );

						writer_p->setEncryption( output_user_pw_p,
																		 output_owner_pw_p,
																		 m_output_user_perms,
																		 bit128_b );
					}

				output_doc_p->open();

				int output_page_count= 0;
				for( vector< PageRef >::const_iterator it= m_page_seq.begin();
						 it!= m_page_seq.end(); ++it, ++output_page_count )
					{
						// get the reader associated with this page ref.
						if( it->m_input_pdf_index< m_input_pdf.size() ) {
							InputPdf& input_pdf= m_input_pdf[ it->m_input_pdf_index ];

							if( m_verbose_reporting_b ) {
								cout << "   Adding page " << it->m_page_num;
								cout << " from " << input_pdf.m_filename << endl;
							}

							// take the first, associated reader and then disassociate
							itext::PdfReader* input_reader_p= 0;
							vector< pair< set<jint>, itext::PdfReader* > >::iterator mt=
								input_pdf.m_readers.begin();
							for( ; mt!= input_pdf.m_readers.end(); ++mt ) {
								set<jint>::iterator nt= mt->first.find( it->m_page_num );
								if( nt!= mt->first.end() ) { // assoc. found
									input_reader_p= mt->second;
									mt->first.erase( nt ); // remove this assoc.
									break;
								}
							}

							if( input_reader_p ) {
								if( m_output_uncompress_b ) {
									add_mark_to_page( input_reader_p, it->m_page_num, output_page_count+ 1 );
								}
								else if( m_output_compress_b ) {
									remove_mark_from_page( input_reader_p, it->m_page_num );
								}

								itext::PdfImportedPage* page_p= 
									writer_p->getImportedPage( input_reader_p, it->m_page_num );

								writer_p->addPage( page_p );
							}
							else { // error
								cerr << "Internal Error: no reader found for page: ";
								cerr << it->m_page_num << " in file: " << input_pdf.m_filename << endl;
								break;
							}
						}
						else { // error
							cerr << "Internal Error: Unable to find handle in m_input_pdf." << endl;
							break;
						}
					}

				output_doc_p->close();
				writer_p->close();
			}
			break;
			
			case burst_k : { // burst input into pages

				// we should have been given only a single, input file
				if( 1< m_input_pdf.size() ) { // error
					cerr << "Error: Only one input PDF file may be given for \"burst\" op." << endl;
					cerr << "   No output created." << endl;
					break;
				}

				// grab the first reader, since there's only one
				itext::PdfReader* input_reader_p= 
					m_input_pdf.begin()->m_readers.front().second;
				jint input_num_pages= 
					m_input_pdf.begin()->m_num_pages;

				if( m_output_filename.empty() ) {
					m_output_filename= "pg_%04d.pdf";
				}
				for( jint ii= 0; ii< input_num_pages; ++ii ) {

					// the filename
					char buff[4096]= "";
					sprintf( buff, m_output_filename.c_str(), ii+ 1 );

					java::String* jv_output_filename_p= JvNewStringLatin1( buff );

					itext::Document* output_doc_p= new itext::Document();
					java::FileOutputStream* ofs_p= new java::FileOutputStream( jv_output_filename_p );
					itext::PdfCopy* writer_p= new itext::PdfCopy( output_doc_p, ofs_p );

					output_doc_p->addCreator( jv_creator_p );

					// un/compress output streams?
					if( m_output_uncompress_b ) {
						writer_p->filterStreams= true;
						writer_p->compressStreams= false;
					}
					else if( m_output_compress_b ) {
						writer_p->filterStreams= false;
						writer_p->compressStreams= true;
					}

					// encrypt output?
					if( m_output_encryption_strength!= none_enc ||
							!m_output_owner_pw.empty() || 
							!m_output_user_pw.empty() )
						{
							// if no stregth is given, default to 128 bit,
							// (which is incompatible w/ Acrobat 4)
							bool bit128_b=
								( m_output_encryption_strength!= bits40_enc );

							writer_p->setEncryption( output_user_pw_p,
																			 output_owner_pw_p,
																			 m_output_user_perms,
																			 bit128_b );
						}

					output_doc_p->open();
						
					itext::PdfImportedPage* page_p= 
						writer_p->getImportedPage( input_reader_p, ii+ 1 );
						
					writer_p->addPage( page_p );

					output_doc_p->close();
					writer_p->close();
				}

				////
				// dump document data

				ofstream ofs( "doc_data.txt" );
				if( ofs ) {
					ReportOnPdf( ofs, input_reader_p );
				}
				else { // error
					cerr << "Error: unable to open file for output: doc_data.txt" << endl;
				}

			}
			break;

			case filter_k: { // apply operations to given PDF file

				// we should have been given only a single, input file
				if( 1< m_input_pdf.size() ) { // error
					cerr << "Error: Only one input PDF file may be given for this" << endl;
					cerr << "   operation.  Maybe you meant to use the \"cat\" operator?" << endl;
					cerr << "   No output created." << endl;
					break;
				}

				// try opening the FDF file before we get too involved
				itext::FdfReader* fdf_reader_p= 0;
				if( !m_form_data_filename.empty() ) {
					if( m_form_data_filename== "PROMPT" ) {
						prompt_for_filename( "Please enter a filename for the input FDF data:", 
																 m_form_data_filename );
					}
					try {
						fdf_reader_p=
							new itext::FdfReader( JvNewStringLatin1( m_form_data_filename.c_str() ) );
					}
					catch( java::io::IOException* ioe_p ) { // file open error
						cerr << "Error: Failed to open FDF (data) file: " << endl;
						cerr << "   " << m_form_data_filename << endl;
						cerr << "   No output created." << endl;
						break;
					}
				}

				// try opening the PDF background (or watermark) before we get too involved
				itext::PdfReader* mark_p= 0;
				com::lowagie::text::pdf::PdfImportedPage* mark_page_p= 0;
				if( !m_background_filename.empty() ) {
					if( m_background_filename== "PROMPT" ) {
						prompt_for_filename( "Please enter a filename for the background PDF:", 
																 m_background_filename );
					}
					try {
						mark_p= new itext::PdfReader( JvNewStringLatin1( m_background_filename.c_str() ) );
						mark_p->removeUnusedObjects();
						mark_p->shuffleSubsetNames();
					}
					catch( java::io::IOException* ioe_p ) { // file open error
						cerr << "Error: Failed to open background PDF file: " << endl;
						cerr << "   " << m_background_filename << endl;
						cerr << "   No output created." << endl;
						break;
					}
				}

				java::OutputStream* ofs_p= 
					get_output_stream( m_output_filename,
														 m_ask_about_warnings_b );

				if( !ofs_p ) { // file open error
					break;
				}
				itext::PdfReader* input_reader_p= 
					m_input_pdf.begin()->m_readers.front().second;

				itext::PdfStamperImp* writer_p=
					new itext::PdfStamperImp( input_reader_p, ofs_p, 0 );

				// update the info?
				if( !m_update_info_filename.empty() ) {
					if( m_update_info_filename== "PROMPT" ) {
						prompt_for_filename( "Please enter an Info file filename:",
																 m_update_info_filename );
					}
					if( m_update_info_filename== "-" ) {
						if( !UpdateInfo( input_reader_p, cin ) ) {
							cerr << "Warning: no Info added to output PDF." << endl;
						}
					}
					else {
						ifstream ifs( m_update_info_filename.c_str() );
						if( ifs ) {
							if( !UpdateInfo( input_reader_p, ifs ) ) {
								cerr << "Warning: no Info added to output PDF." << endl;
							}
						}
						else { // error
							cerr << "Error: unable to open FDF file for input: " << m_update_info_filename << endl;
							break;
						}
					}
				}

				// un/compress output streams?
				if( m_output_uncompress_b ) {
					add_marks_to_pages( input_reader_p );
					writer_p->filterStreams= true;
					writer_p->compressStreams= false;
				}
				else if( m_output_compress_b ) {
					remove_marks_from_pages( input_reader_p );
					writer_p->filterStreams= false;
					writer_p->compressStreams= true;
				}

				// encrypt output?
				if( m_output_encryption_strength!= none_enc ||
						!m_output_owner_pw.empty() ||
						!m_output_user_pw.empty() )
					{

						// if no stregth is given, default to 128 bit,
						// (which is incompatible w/ Acrobat 4)
						bool bit128_b=
							( m_output_encryption_strength!= bits40_enc );

						writer_p->setEncryption( output_user_pw_p,
																			output_owner_pw_p,
																			m_output_user_perms,
																			bit128_b );
					}

				// fill form fields?
				if( fdf_reader_p ) {
					itext::AcroFields* fields_p= writer_p->getAcroFields();
					fields_p->setGenerateAppearances( true ); // have iText create field appearances
					if( fields_p->setFields( fdf_reader_p ) ) { // returs true if Rich Text input found

						// set the PDF so that Acrobat will create appearances;
						// this might appear contradictory to our setGenerateAppearances( true ) call,
						// above; setting this, here, allows us to keep the generated appearances,
						// in case the PDF is opened somewhere besides Acrobat; yet, Acrobat/Reader
						// will create the Rich Text appearance if it has a chance
						itext::PdfDictionary* catalog_p= input_reader_p->catalog;
						if( catalog_p && catalog_p->isDictionary() ) {
							
							itext::PdfDictionary* acro_form_p= (itext::PdfDictionary*)
								input_reader_p->getPdfObject( catalog_p->get( itext::PdfName::ACROFORM ) );
							if( acro_form_p && acro_form_p->isDictionary() ) {

								acro_form_p->put( itext::PdfName::NEEDAPPEARANCES, itext::PdfBoolean::PDFTRUE );
							}
						}
					}
				}

				// flatten form fields?
				writer_p->setFormFlattening( m_output_flatten_b );

				// add background/watermark?
				if( mark_p ) {
					com::lowagie::text::Rectangle* mark_page_size_p= mark_p->getCropBox( 1 );
					jint mark_page_rotation= mark_p->getPageRotation( 1 );
					for( jint mm= 0; mm< mark_page_rotation; mm+=90 ) {
						mark_page_size_p= mark_page_size_p->rotate();
					}

					// create a PdfTemplate from the first page of mark
					// (PdfImportedPage is derived from PdfTemplate)
					com::lowagie::text::pdf::PdfImportedPage* mark_page_p=
						writer_p->getImportedPage( mark_p, 1 );

          // iterate over document's pages, adding mark_page as
          // a layer 'underneath' the page content; scale mark_page
          // and move it so it fits within the document's page;
					jint num_pages= input_reader_p->getNumberOfPages();
					for( jint ii= 0; ii< num_pages; ) {
						++ii;
						com::lowagie::text::Rectangle* doc_page_size_p= 
							input_reader_p->getCropBox( ii );
						jint doc_page_rotation= input_reader_p->getPageRotation( ii );
						for( jint mm= 0; mm< doc_page_rotation; mm+=90 ) {
							doc_page_size_p= doc_page_size_p->rotate();
						}

						jfloat h_scale= doc_page_size_p->width() / mark_page_size_p->width();
						jfloat v_scale= doc_page_size_p->height() / mark_page_size_p->height();
						jfloat mark_scale= (h_scale< v_scale) ? h_scale : v_scale;

						jfloat h_trans= (jfloat)(doc_page_size_p->left()- mark_page_size_p->left()* mark_scale +
																		 (doc_page_size_p->width()- 
																			mark_page_size_p->width()* mark_scale) / 2.0);
						jfloat v_trans= (jfloat)(doc_page_size_p->bottom()- mark_page_size_p->bottom()* mark_scale +
																		 (doc_page_size_p->height()- 
																			mark_page_size_p->height()* mark_scale) / 2.0);
          
						com::lowagie::text::pdf::PdfContentByte* content_byte_p= 
							writer_p->getUnderContent( ii );
						if( mark_page_rotation== 0 ) {
							content_byte_p->addTemplate( mark_page_p, 
																					 mark_scale, 0,
																					 0, mark_scale,
																					 h_trans, 
																					 v_trans );
						}
						else if( mark_page_rotation== 90 ) {
							content_byte_p->addTemplate( mark_page_p, 
																					 0, -1* mark_scale,
																					 mark_scale, 0,
																					 h_trans, 
																					 v_trans+ mark_page_size_p->height()* mark_scale );
						}
						else if( mark_page_rotation== 180 ) {
							content_byte_p->addTemplate( mark_page_p, 
																					 -1* mark_scale, 0,
																					 0, -1* mark_scale,
																					 h_trans+ mark_page_size_p->width()* mark_scale, 
																					 v_trans+ mark_page_size_p->height()* mark_scale );
						}
						else if( mark_page_rotation== 270 ) {
							content_byte_p->addTemplate( mark_page_p, 
																					 0, mark_scale,
																					 -1* mark_scale, 0,
																					 h_trans+ mark_page_size_p->width()* mark_scale, v_trans );
						}
					}
				}

				// attach file to document?
				if( !m_input_attach_file_filename.empty() ) {
					this->attach_files( input_reader_p,
															writer_p );
				}

				// done; write output
				writer_p->close();
			}
			break;

			case dump_data_fields_k :
			case dump_data_k: { // report on input document

				// we should have been given only a single, input file
				if( 1< m_input_pdf.size() ) { // error
					cerr << "Error: Only one input PDF file may be used for the dump_data operation" << endl;
					cerr << "   No output created." << endl;
					break;
				}

				itext::PdfReader* input_reader_p= 
					m_input_pdf.begin()->m_readers.front().second;

				if( m_output_filename.empty() || m_output_filename== "-" ) {
					if( m_operation== dump_data_k ) {
						ReportOnPdf( cout, input_reader_p );
					}
					else if( m_operation== dump_data_fields_k ) {
						ReportAcroFormFields( cout, input_reader_p );
					}
				}
				else {
					ofstream ofs( m_output_filename.c_str() );
					if( ofs ) {
						if( m_operation== dump_data_k ) {
							ReportOnPdf( ofs, input_reader_p );
						}
						else if( m_operation== dump_data_fields_k ) {
							ReportAcroFormFields( ofs, input_reader_p );
						}
					}
					else { // error
						cerr << "Error: unable to open file for output: " << m_output_filename << endl;
					}
				}
			}
			break;

			case unpack_files_k: { // copy PDF file attachments into current directory

				// we should have been given only a single, input file
				if( 1< m_input_pdf.size() ) { // error
					cerr << "Error: Only one input PDF file may be given for \"unpack_files\" op." << endl;
					cerr << "   No output created." << endl;
					break;
				}

				itext::PdfReader* input_reader_p= 
					m_input_pdf.begin()->m_readers.front().second;

				this->unpack_files( input_reader_p );
			}
			break;
			}
		}
		catch( java::lang::Throwable* t_p )
			{
				cerr << "Unhandled Java Exception:" << endl;
				t_p->printStackTrace();
			}
	}
}

int main(int argc, char** argv)
{
	bool help_b= false;
	bool version_b= false;
	bool synopsis_b= ( argc== 1 );
	int ret_val= 0; // default: no error

	for( int ii= 1; ii< argc; ++ii ) {
		version_b=
			(version_b || 
			 strcmp( argv[ii], "--version" )== 0 );
		help_b= 
			(strcmp( argv[ii], "--help" )== 0 || 
			 strcmp( argv[ii], "-h" )== 0 );
	}

	if( help_b ) {
		describe_full();
	}
	else if( version_b ) {
		describe_header();
	}
	else if( synopsis_b ) {
		describe_synopsis();
	}
	else {
		try {
			JvCreateJavaVM(NULL);
			JvAttachCurrentThread(NULL, NULL);

			JvInitClass(&java::System::class$);
			JvInitClass(&java::util::ArrayList::class$);
			JvInitClass(&java::util::Iterator::class$);

			JvInitClass(&itext::PdfObject::class$);
			JvInitClass(&itext::PdfName::class$);
			JvInitClass(&itext::PdfDictionary::class$);
			JvInitClass(&itext::PdfOutline::class$);
			JvInitClass(&itext::PdfBoolean::class$);

			TK_Session tk_session( argc, argv );

			tk_session.dump_session_data();

			if( tk_session.is_valid() ) {
				tk_session.create_output();
			}
			else { // error
				cerr << "Done.  Input errors, so no output created." << endl;
				ret_val= 1;
			}

			JvDetachCurrentThread();
		}
		catch( java::lang::Throwable* t_p )
			{
				cerr << "Unhandled Java Exception:" << endl;
				t_p->printStackTrace();
				ret_val= 2;
			}
	}

	return ret_val;
}

static void
describe_header() {
	cout << endl;
	cout << "pdftk " << PDFTK_VER << " a Handy Tool for Manipulating PDF Documents" << endl;
	cout << "Copyright (C) 2003-04, Sid Steward - Please Visit: www.pdftk.com" << endl;
	cout << "This is free software; see the source code for copying conditions. There is" << endl;
	cout << "NO warranty, not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

static void
describe_synopsis() {
	cout << 
"SYNOPSIS\n\
       pdftk <input PDF files | - | PROMPT>\n\
            [input_pw <input PDF owner passwords | PROMPT>]\n\
            [<operation> <operation arguments>]\n\
            [output <output filename | - | PROMPT>]\n\
            [encrypt_40bit | encrypt_128bit]\n\
            [allow <permissions>]\n\
            [owner_pw <owner password | PROMPT>]\n\
            [user_pw <user password | PROMPT>]\n\
            [flatten] [compress | uncompress]\n\
            [verbose] [dont_ask | do_ask]\n\
       Where:\n\
            <operation> may be empty, or:\n\
            [cat | attach_files | unpack_files | burst |\n\
             fill_form | background |\n\
             dump_data | dump_data_fields | update_info]\n\
\n\
       For Complete Help: pdftk --help\n";
}

static void
describe_full() {
	describe_header();
	cout << endl;

	describe_synopsis();
	cout << endl;

	cout <<
"DESCRIPTION\n\
       If  PDF  is  electronic paper, then pdftk is an electronic\n\
       staple-remover, hole-punch,  binder,  secret-decoder-ring,\n\
       and  X-Ray-glasses.   Pdftk  is  a  simple  tool for doing\n\
       everyday things with PDF documents.  Use it to:\n\
\n\
       * Merge PDF Documents\n\
       * Split PDF Pages into a New Document\n\
       * Decrypt Input as Necessary (Password Required)\n\
       * Encrypt Output as Desired\n\
       * Fill PDF Forms with FDF Data and/or Flatten Forms\n\
       * Apply a Background Watermark\n\
       * Report PDF Metrics such as Metadata and Bookmarks\n\
       * Update PDF Metadata\n\
       * Attach Files to PDF Pages or the PDF Document\n\
       * Unpack PDF Attachments\n\
       * Burst a PDF Document into Single Pages\n\
       * Uncompress and Re-Compress Page Streams\n\
       * Repair Corrupted PDF (Where Possible)\n\
\n\
OPTIONS\n\
       A summary of options is included below.\n\
\n\
       --help, -h\n\
              Show summary of options.\n\
\n\
       <input PDF files | - | PROMPT>\n\
              A list of the input PDF files. If you plan to  com-\n\
              bine  these  PDFs (without using handles) then list\n\
              files in the order you want them combined.   Use  -\n\
              to  pass  a single PDF into pdftk via stdin.  Input\n\
              files can be associated with handles, where a  han-\n\
              dle is a single, upper-case letter:\n\
\n\
              <input PDF handle>=<input PDF filename>\n\
\n\
              Handles  are  often  omitted.  They are useful when\n\
              specifying PDF passwords or page ranges, later.\n\
\n\
              For example: A=input1.pdf B=input2.pdf\n\
\n\
       [input_pw <input PDF owner passwords | PROMPT>]\n\
              Input PDF owner passwords, if necessary, are  asso-\n\
              ciated with files by using their handles:\n\
\n\
              <input PDF handle>=<input PDF file owner password>\n\
\n\
              If  handles are not given, then passwords are asso-\n\
              ciated with input files by order.\n\
\n\
              Most pdftk features require  that  encrypted  input\n\
              PDF are accompanied by the ~owner~ password. If the\n\
              input PDF has no  owner  password,  then  the  user\n\
              password  must be given, instead.  If the input PDF\n\
              has no passwords, then no password should be given.\n\
\n\
              When  running in do_ask mode, pdftk will prompt you\n\
              for a password if the supplied password  is  incor-\n\
              rect or none was given.\n\
\n\
       [<operation> <operation arguments>]\n\
              If  this  optional  argument is omitted, then pdftk\n\
              runs in 'filter' mode.  Filter mode takes only  one\n\
              PDF  input and creates a new PDF after applying all\n\
              of the output options, like encryption and compres-\n\
              sion.\n\
\n\
              Available   operations   are:   cat,  attach_files,\n\
              unpack_files,   burst,    fill_form,    background,\n\
              dump_data,   dump_data_fields,   update_info.  Some\n\
              operations takes  additional  arguments,  described\n\
              below.\n\
\n\
          cat [<page ranges>]\n\
                 Catenates  pages from input PDFs to create a new\n\
                 PDF.  Page order in the new PDF is specified  by\n\
                 the order of the given page ranges.  Page ranges\n\
                 are described like this:\n\
\n\
                 <input  PDF  handle>[<begin  page  number>[-<end\n\
                 page number>[<qualifier>]]]\n\
\n\
                 Where the handle identifies one of the input PDF\n\
                 files, and the beginning and ending page numbers\n\
                 are  one-based  references  to  pages in the PDF\n\
                 file, and the qualifier can be even or odd.\n\
\n\
                 If the handle is omitted from  the  page  range,\n\
                 then  the  pages  are taken from the first input\n\
                 PDF.\n\
\n\
                 If no arguments are passed to  cat,  then  pdftk\n\
                 combines  all  input PDFs in the order they were\n\
                 given to create the output.\n\
\n\
                 NOTES:\n\
                 * <end page number> may be less than <begin page\n\
                   number>.\n\
                 *  The  keyword end may be used to reference the\n\
                   final page of a document  instead  of  a  page\n\
                   number.\n\
                 * Reference a single page by omitting the ending\n\
                   page number.\n\
                 * The handle may be used alone to represent  the\n\
                   entire  PDF document, e.g., B1-end is the same\n\
                   as B.\n\
\n\
                 Page range examples:\n\
                 A1-21\n\
                 Bend-1odd\n\
                 A72\n\
                 A1-21 Beven A72\n\
\n\
          attach_files <attachment filenames | PROMPT>\n\
          [to_page <page number | PROMPT>]\n\
                 Packs arbitrary files into  a  PDF  using  PDF's\n\
                 file  attachment features. More than one attach-\n\
                 ment may be listed after  attach_files.  Attach-\n\
                 ments are added at the document level unless the\n\
                 optional to_page option is given, in which  case\n\
                 the  files are attached to the given page number\n\
                 (the first page is 1, the final  page  is  end).\n\
                 For example:\n\
\n\
                 pdftk     in.pdf     attach_files    table1.html\n\
                 table2.html to_page 6 output out.pdf\n\
\n\
          unpack_files\n\
                 Copies all of the attachments from the input PDF\n\
                 into  the  current folder or to an output direc-\n\
                 tory given after output. For example:\n\
\n\
                 pdftk report.pdf unpack_files output ~/atts/\n\
\n\
                 or, interactively:\n\
\n\
                 pdftk report.pdf unpack_files output PROMPT\n\
\n\
          burst  Splits a single, input PDF document  into  indi-\n\
                 vidual   pages.  Also  creates  a  report  named\n\
                 doc_data.txt which is the  same  as  the  output\n\
                 from  dump_data.  If the output section is omit-\n\
                 ted, then  PDF  pages  are  named:  pg_%04d.pdf,\n\
                 e.g.:  pg_0001.pdf,  pg_0002.pdf,  etc.  To name\n\
                 these pages  yourself,  supply  a  printf-styled\n\
                 format string via the output section.  For exam-\n\
                 ple,  if  you  want  pages  named:  page_01.pdf,\n\
                 page_02.pdf,  etc., pass output page_%02d.pdf to\n\
                 pdftk.  Encryption can be applied to the  output\n\
                 by  appending  output  options such as owner_pw,\n\
                 e.g.:\n\
\n\
                 pdftk in.pdf burst owner_pw foopass\n\
\n\
          fill_form <FDF data filename | - | PROMPT>\n\
                 Fills the single input PDF's  form  fields  with\n\
                 the  data  from  an FDF file or stdin. Enter the\n\
                 FDF data filename after fill_form, or use  -  to\n\
                 pass the data via stdin, like so:\n\
\n\
                 pdftk   form.pdf   fill_form   data.fdf   output\n\
                 form.filled.pdf\n\
\n\
                 After filling a form,  the  form  fields  remain\n\
                 interactive unless you also use the flatten out-\n\
                 put option. flatten merges the form fields  with\n\
                 the  PDF  pages. You can use flatten alone, too,\n\
                 but only on a single PDF:\n\
\n\
                 pdftk form.pdf fill_form data.fdf output out.pdf\n\
                 flatten\n\
\n\
                 or:\n\
\n\
                 pdftk form.filled.pdf output out.pdf flatten\n\
\n\
                 If the input FDF file includes Rich Text format-\n\
                 ted data in addition to  plain  text,  then  the\n\
                 Rich Text data is packed into the form fields as\n\
                 well as the plain text.  Pdftk also sets a  flag\n\
                 that  cues  Acrobat/Reader to generate new field\n\
                 appearances based on the Rich Text  data.   That\n\
                 way,  when  the  user  opens the PDF, the viewer\n\
                 will create the Rich Text fields  on  the  spot.\n\
                 If  the  user's PDF viewer does not support Rich\n\
                 Text, then the user will see the plain text data\n\
                 instead.   If you flatten this form before Acro-\n\
                 bat has a chance to create (and save) new  field\n\
                 appearances,  then  the plain text field data is\n\
                 what you'll see.\n\
\n\
          background <background PDF filename | - | PROMPT>\n\
                 Applies a PDF watermark to the background  of  a\n\
                 single  input  PDF.   Pass  the background PDF's\n\
                 filename after background like so:\n\
\n\
                 pdftk in.pdf background back.pdf output out.pdf\n\
\n\
                 Pdftk uses only the first page  from  the  back-\n\
                 ground  PDF  and applies it to every page of the\n\
                 input PDF.  This page is scaled and  rotated  as\n\
                 needed  to fit the input page.  You can use - to\n\
                 pass a background PDF into pdftk via stdin.  For\n\
                 backward  compatibility  with  pdftk  1.0, back-\n\
                 ground can be used as an  output  option.   How-\n\
                 ever,  this  old  technique  works  only when no\n\
                 operation is given.\n\
\n\
          dump_data\n\
                 Reads a single, input PDF file and reports vari-\n\
                 ous  statistics, metadata, bookmarks (a/k/a out-\n\
                 lines), and page  labels  to  the  given  output\n\
                 filename  or  (if no output is given) to stdout.\n\
                 Does not create a new PDF.\n\
\n\
          dump_data_fields\n\
                 Reads a single, input PDF file and reports  form\n\
                 field statistics to the given output filename or\n\
                 (if no output is given)  to  stdout.   Does  not\n\
                 create a new PDF.\n\
\n\
          update_info <info data filename | - | PROMPT>\n\
                 Changes  the  metadata  stored in a single PDF's\n\
                 Info dictionary to match the  input  data  file.\n\
                 The  input data file uses the same syntax as the\n\
                 output from dump_data. This does not change  the\n\
                 metadata  stored  in the PDF's XMP stream, if it\n\
                 has one. For example:\n\
\n\
                 pdftk in.pdf update_info in.info output out.pdf\n\
\n\
       [output <output filename | - | PROMPT>]\n\
              The output PDF filename may not be set to the  name\n\
              of  an  input  filename. Use - to output to stdout.\n\
              When using the dump_data operation, use  output  to\n\
              set  the  name  of the output data file. When using\n\
              the unpack_files operation, use output to  set  the\n\
              name  of an output directory.  When using the burst\n\
              operation,  you  can  use  output  to  control  the\n\
              resulting PDF page filenames (described above).\n\
\n\
       [encrypt_40bit | encrypt_128bit]\n\
              If  an  output PDF user or owner password is given,\n\
              output PDF  encryption  strength  defaults  to  128\n\
              bits.    This   can  be  overridden  by  specifying\n\
              encrypt_40bit.\n\
\n\
       [allow <permissions>]\n\
              Permissions are applied to the output PDF  only  if\n\
              an  encryption strength is specified or an owner or\n\
              user password is given.   If  permissions  are  not\n\
              specified,  they default to 'none,' which means all\n\
              of the following features are disabled.\n\
\n\
              The permissions section may include one or more  of\n\
              the following features:\n\
\n\
              Printing\n\
                     Top Quality Printing\n\
\n\
              DegradedPrinting\n\
                     Lower Quality Printing\n\
\n\
              ModifyContents\n\
                     Also allows Assembly\n\
\n\
              Assembly\n\
\n\
              CopyContents\n\
                     Also allows ScreenReaders\n\
\n\
              ScreenReaders\n\
\n\
              ModifyAnnotations\n\
                     Also allows FillIn\n\
\n\
              FillIn\n\
\n\
              AllFeatures\n\
                     Allows the user to perform all of the above,\n\
                     and top quality printing.\n\
\n\
       [owner_pw <owner password | PROMPT>]\n\
       [user_pw <user password | PROMPT>]\n\
              If an encryption strength is given but no passwords\n\
              are  supplied,  then  the  owner and user passwords\n\
              remain empty, which means that  the  resulting  PDF\n\
              may  be  opened and its security parameters altered\n\
              by anybody.\n\
\n\
       [compress | uncompress]\n\
              These are only useful when you  want  to  edit  PDF\n\
              code  in  a  text editor like vim or emacs.  Remove\n\
              PDF page stream compression by applying the  uncom-\n\
              press  filter.  Use  the compress filter to restore\n\
              compression.\n\
\n\
       [flatten]\n\
              Use this option to merge an input PDF's interactive\n\
              form  fields (and their data) with the PDF's pages.\n\
              Only one input PDF may  be  given.  Sometimes  used\n\
              with the fill_form operation.\n\
\n\
       [verbose]\n\
              By  default,  pdftk runs quietly. Append verbose to\n\
              the end and it will speak up.\n\
\n\
       [dont_ask | do_ask]\n\
              Depending  on  the   compile-time   settings   (see\n\
              ASK_ABOUT_WARNINGS),  pdftk  might  prompt  you for\n\
              further input when it encounters a problem, such as\n\
              a  bad  password. Override this default behavior by\n\
              adding dont_ask (so pdftk won't ask you what to do)\n\
              or do_ask (so pdftk will ask you what to do).\n\
\n\
              When  running  in  dont_ask  mode, pdftk will over-\n\
              write files with its output without notice.\n\
\n\
EXAMPLES\n\
       Decrypt a PDF\n\
         pdftk secured.pdf input_pw foopass output unsecured.pdf\n\
\n\
       Encrypt a PDF using 128-bit strength (the default),  with-\n\
       hold all permissions (the default)\n\
         pdftk 1.pdf output 1.128.pdf owner_pw foopass\n\
\n\
       Same as above, except password 'baz' must also be used  to\n\
       open output PDF\n\
         pdftk 1.pdf output 1.128.pdf owner_pw foo user_pw baz\n\
\n\
       Same as above, except printing is allowed (once the PDF is\n\
       open)\n\
         pdftk 1.pdf output 1.128.pdf owner_pw  foo  user_pw  baz\n\
         allow printing\n\
\n\
       Join in1.pdf and in2.pdf into a new PDF, out1.pdf\n\
         pdftk in1.pdf in2.pdf cat output out1.pdf\n\
         or (using handles):\n\
         pdftk A=in1.pdf B=in2.pdf cat A B output out1.pdf\n\
         or (using wildcards):\n\
         pdftk *.pdf cat output combined.pdf\n\
\n\
       Remove 'page 13' from in1.pdf to create out1.pdf\n\
         pdftk in.pdf cat 1-12 14-end output out1.pdf\n\
         or:\n\
         pdftk A=in1.pdf cat A1-12 A14-end output out1.pdf\n\
\n\
       Apply  40-bit  encryption  to output, revoking all permis-\n\
       sions (the default). Set the owner PW to 'foopass'.\n\
         pdftk   1.pdf   2.pdf  cat  output  3.pdf  encrypt_40bit\n\
         owner_pw foopass\n\
\n\
       Join  two  files,  one  of  which  requires  the  password\n\
       'foopass'. The output is not encrypted.\n\
         pdftk A=secured.pdf 2.pdf input_pw A=foopass cat  output\n\
         3.pdf\n\
\n\
       Uncompress  PDF page streams for editing the PDF in a text\n\
       editor (e.g., vim, emacs)\n\
         pdftk doc.pdf output doc.unc.pdf uncompress\n\
\n\
       Repair a PDF's corrupted XREF table and stream lengths, if\n\
       possible\n\
         pdftk broken.pdf output fixed.pdf\n\
\n\
       Burst  a  single PDF document into pages and dump its data\n\
       to doc_data.txt\n\
         pdftk mydoc.pdf burst\n\
\n\
       Burst  a  single  PDF document into encrypted pages. Allow\n\
       low-quality printing\n\
         pdftk  mydoc.pdf  burst owner_pw foopass allow Degraded-\n\
         Printing\n\
\n\
       Write a report on PDF document metadata and  bookmarks  to\n\
       report.txt\n\
         pdftk mydoc.pdf dump_data output report.txt\n";
}