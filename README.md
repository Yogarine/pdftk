# PDFtk Server

This is a fork of the original PDFtk Server project created by
[PDF Labs](https://www.pdflabs.com/tools/pdftk-server/).

PDFtk Server is a command-line tool for working with PDFs. It is commonly used
for client-side scripting or server-side processing of PDFs.

It is also used by OEMs and ISVs to give their products the ability to
manipulate PDFs.

PDFtk Server can:

  - Merge PDF documents or collate PDF page scans.
  - Split PDF pages into new documents.
  - Rotate PDF documents or pages.
  - Decrypt input as necessary (by providing a password).
  - Encrypt output as desired.
  - Fill PDF forms with X/FDF data and/or flatten forms.
  - Generate FDF Data Stencils from PDF Forms.
  - Apply a watermark or a stamp.
  - Report PDF metrics, bookmarks and metadata.
  - Add/Update PDF bookmarks or metadata.
  - Attach files to PDF pages or the PDF document.
  - Unpack PDF attachments.
  - Explode a PDF document into single pages.
  - Uncompress and compress page streams.
  - Repair corrupted PDFs (where possible).

PDFtk Server does not require Adobe Acrobat or Reader, and it runs on Windows,
Mac OS X and Linux.


## Documentation

PDFtk’s features are fully documented on its [man page](docs/pdftk.md). We also
offer some [command-line examples](docs/examples.md).


## Downloads

PDF Labs provides installers for Windows, Mac OS X, Red Hat Enterprise Linux and
CentOS here: https://www.pdflabs.com/tools/pdftk-server/#downloads

Many Linux distributions provide a PDFtk package you can download and install
using their package manager.


## Build PDFtk Server

PDFtk Server can be compiled using the GNU Compiler for Java (GCJ) which was
part of the GNU Compiler Collection (GCC). Unfortunately, GCC has dropped GCJ
since version 7, so in order to compile PDFtk Server you'll need to use an older
version of GCC.

PDFtk Server is known to compile and run on Debian, Ubuntu Linux, FreeBSD,
Slackware Linux, SuSE, Solaris and HP-UX.

 1. Clone this repository:
    
    ```bash
    git clone git@github.com:Yogarine/pdftk.git
    ```

 2. Review the Makefile provided for your platform and confirm that `TOOLPATH`
    and `VERSUFF` suit your installation of GCC/GCJ/libgcj. If you run apropos 
    GCC and it returns something like gcc-4.5, then set `VERSUFF` to `-4.5`. The
    `TOOLPATH` probably doesn’t need to be set.

 3. Change into the `pdftk` sub-directory, run
    
    ```bash
    make -f Makefile.Debian
    ```
    (substitute your platform’s Makefile filename)

We have built PDFtk using GCC/GCJ/libgcj versions 3.4.5, 4.4.1, 4.5.0 and 4.6.3.
PDFtk 1.4x fails to build on GCC 3.3.5 due to missing libgcj features. If you
are using gcc 3.3 or older, try building PDFtk 1.12 instead.
