Purpose: XO-1 signed install 13.2.1 to SD card

\ A standalone firmware installer for OLPC OS 13.2.1 on XO-1.
\
\ Background: signed fs-update on XO-1.5 and later is an fs.zip
\ containing a payload that is executed as a forth source, and the
\ payload calls fs-update functions.  However, fs-update is not
\ present in this form on XO-1.  Adding it to XO-1 firmware is risky;
\ it may break NAND reflash.
\
\ Design: backport fs-update to XO-1, implement in this installer, and
\ then tack on the .zd.zsp.
\
\ To sign this file for XO-1: ./sign-fw.sh /keys/fs data.img fs0.zip

warning off \ several redefinitions used

\ from copynand.fth

h# 20000 to /nand-block
h#   200 to /nand-page
/nand-block /nand-page / to nand-pages/block
0 to #nand-pages
0 to #image-eblocks

: close-image-file  ( -- )
   fileih  ?dup  if  0 to fileih  close-file drop  then
;

: close-nand-ihs  ( -- )
   close-image-file
   close-nand
;

: ?nand-abort  ( flag msg$ -- )
   rot  if
      close-nand-ihs  $abort
   else
      2drop
   then
;

: set-nand-vars  ( -- )
   " size" $call-nand  /nand-page  um/mod nip to #nand-pages
;

: open-nand  ( -- )
   " ext:0" open-dev to nandih
   nandih 0=  " Can't open disk device"  ?nand-abort
   set-nand-vars
;

d# 26 value gstatus-line
h# e8 h# e8 h# e8  rgb>565 to starting-color \ very light gray

: gshow-init  ( #eblocks -- )
   dup set-grid-scale
   cursor-off  " erase-screen" $call-screen

   starting-color   ( #eblocks color )
   over 0  ?do  i over show-state  scale-factor +loop  ( #eblocks color )
   drop                                  ( #eblocks )
   1 gstatus-line at-xy      ." Blocks/square: " scale-factor .d
   d# 25 gstatus-line at-xy  ." Total blocks: " .d
   d# 49 gstatus-line at-xy  ." Now writing: " cr
   ."  Installing to SD card ..... please wait."
   \   Rebooting in 10 seconds ... 
;

: show-eblock#  ( eblock# -- )
    d# 62 gstatus-line at-xy
    dup .d space
    d# 100 * #image-eblocks / .d ." %"
;

: gshow-written  ( eblock# -- )
   dup  written-color  show-state
   show-eblock#
;

: gshow
   ['] gshow-init      to show-init
   ['] gshow-written   to show-written
   ['] gshow-done      to show-done
;

gshow

\ from fsupdate.fth

: get-dhex#  ( -- d )
   0 safe-parse-word
   push-hex $dnumber pop-base  " Bad number" ?nand-abort
;

0 value min-eblock#
0 value max-eblock#

: written ( eblock# -- )
   dup
   max-eblock# max to max-eblock# ( eblock# )
   min-eblock# min to min-eblock#
;

: ?all-written  ( -- )
   max-eblock# 1+ #image-eblocks <>  if
      cr
      red-letters
      ." WARNING: The file said highest block " #image-eblocks .d
      ." but wrote only as high as block " max-eblock# .d cr
      cancel
   then
   min-eblock# 0 <>  if
      cr
      red-letters
      ." WARNING: The file did not write a zero block, "
      ." but wrote only as low as block " min-eblock# .d cr
      cancel
   then
;

0 value secure-fsupdate?
d# 128 constant /spec-maxline

\ We simultaneously DMA one data buffer onto NAND while unpacking the
\ next block of data into another. The buffers exchange roles after
\ each block.

0 value dma-buffer
0 value data-buffer

: swap-buffers  ( -- )  data-buffer dma-buffer  to data-buffer to dma-buffer  ;

: force-line-delimiter  ( delimiter fd -- )
   file @                      ( delim fd fd' )
   swap file !                 ( delim fd' )
   swap line-delimiter c!      ( fd' )
   file !                      ( )
;

: ?compare-spec-line  ( -- )
   secure-fsupdate?  if
      data-buffer /spec-maxline  fileih read-line         ( len end? error? )
      " Spec line read error" ?nand-abort                 ( len end? )
      0= " Spec line too long" ?nand-abort                ( len )
      data-buffer swap                                    ( adr len )
      source $= 0=  " Spec line mismatch" ?nand-abort     ( )
   then
;

also nand-commands definitions

: zblocks:  ( "eblock-size" "#eblocks" ... -- )
   load-crypto  abort" Can't load hash routines"
   open-nand
   load-base to data-buffer
   ?compare-spec-line
   get-hex# to /nand-block
   get-hex# to #image-eblocks
   #image-eblocks to min-eblock#
   0 to max-eblock#
   " size" $call-nand  #image-eblocks /nand-block um*  d<
   " Image size is larger than output device" ?nand-abort
   #image-eblocks  show-init
   get-inflater
   \ Separate the two buffers by enough space for both the compressed
   \ and uncompressed copies of the data.  4x is overkill, but there
   \ is plenty of space at load-base
   load-base /nand-block 4 * + to dma-buffer
   /nand-block /nand-page / to nand-pages/block
   t-update  \ Handle possible timer rollover
;

: zblocks-end:  ( -- )
   " write-blocks-end" $call-nand   ( error? )
   " Write error" ?nand-abort
   release-inflater
   ."      " cr space
   show-done
   ?all-written
   close-nand-ihs
;

0. 2value file-bytes
: data:  ( "filename" -- )
   safe-parse-word            ( filename$ )
   fn-buf place               ( )
   " u:\${CN}${FN}" expand$  image-name-buf place
   image-name$                ( filename$' )
   r/o open-file  if          ( fd )
      drop ." Can't open " image-name$ type cr
      true " " ?nand-abort
   then  to fileih            ( )
   fileih file-size " Can't size file" ?nand-abort  to file-bytes
   linefeed fileih force-line-delimiter
   true to secure-fsupdate?
;

: eat-newline  ( ih -- )
   fgetc newline <>                                       ( error? )
   " Missing newline after zdata" ?nand-abort             ( )
;
: skip-zdata  ( comprlen -- )
   ?compare-spec-line                                     ( comprlen )

   secure-fsupdate?  if  fileih  else  source-id  then    ( comprlen ih )

   >r  u>d  r@ dftell                                     ( d.comprlen d.pos r: ih )
   d+  r@ dfseek                                          ( r: ih )

   r> eat-newline
;

: get-zdata  ( comprlen -- )
   ?compare-spec-line                                     ( comprlen )

   secure-fsupdate?  if  fileih  else  source-id  then    ( comprlen ih )

   >r  data-buffer /nand-block +  over  r@  fgets         ( comprlen #read r: ih )
   <>  " Short read of zdata file" ?nand-abort            ( r: ih )

   r> eat-newline

   \ The "2+" skips the Zlib header
   data-buffer /nand-block + 2+  data-buffer true  (inflate)  ( len )
   /nand-block <>  " Wrong expanded data length" ?nand-abort  ( )
;

true value check-hash?

: check-hash  ( -- )
   2>r                                ( eblock# hashname$ r: hash$ )
   data-buffer /nand-block 2swap      ( eblock# data$ hashname$ r: hash$ )
   crypto-hash                        ( eblock# r: hash$ )
   2r>  $=  0=  if                    ( eblock# )
      ." Bad hash for eblock# " .x cr cr
      ." Your USB key may be bad.  Please try a different one." cr
      ." See http://wiki.laptop.org/go/Bad_hash" cr cr
      abort
   then                               ( eblock# )
;

0 value have-crc?
0 value my-crc

: ?get-crc  ( -- )
   parse-word  dup  if                   ( eblock# hashname$ crc$ r: comprlen )
      push-hex $number pop-base  if      ( eblock# hashname$ crc$ r: comprlen )
         false to have-crc?              ( eblock# hashname$ r: comprlen )
      else                               ( eblock# hashname$ crc r: comprlen )
         to my-crc                       ( eblock# hashname$ r: comprlen )
         true to have-crc?               ( eblock# hashname$ r: comprlen )
      then                               ( eblock# hashname$ r: comprlen )
   else                                  ( eblock# hashname$ empty$ r: comprlen )
      2drop                              ( eblock# hashname$ r: comprlen )
      false to have-crc?                 ( eblock# hashname$ r: comprlen )
   then                                  ( eblock# hashname$ r: comprlen )
;
: ?check-crc  ( -- )
   have-crc?  if
   then
;

: zblock: ( "eblock#" "comprlen" "hashname" "hash-of-128KiB" -- )
   get-hex#                              ( eblock# )
   get-hex# >r                           ( eblock# r: comprlen )
   safe-parse-word                       ( eblock# hashname$ r: comprlen )
   safe-parse-word hex-decode            ( eblock# hashname$ [ hash$ ] err? r: comprlen )
   " Malformed hash string" ?nand-abort  ( eblock# hashname$ hash$ r: comprlen )

   ?get-crc                              ( eblock# hashname$ hash$ r: comprlen )
      r> get-zdata                          ( eblock# hashname$ hash$ )
      ?check-crc                            ( eblock# hashname$ hash$ )

      check-hash?  if                       ( eblock# hashname$ hash$ )
         check-hash                         ( eblock# )
      else                                  ( eblock# hashname$ hash$ )
         2drop 2drop                        ( eblock# )
      then                                  ( eblock# )

      data-buffer over nand-pages/block *  nand-pages/block  " write-blocks-start" $call-nand  ( eblock# error? )
      " Write error" ?nand-abort   ( eblock# )
      swap-buffers                          ( eblock# )

   dup written                              ( eblock# )
   show-written                             ( )
;
