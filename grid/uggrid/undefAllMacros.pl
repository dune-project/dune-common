#!/usr/bin/perl -w

# TODO: Detect location of UG from the DUNE build system
$UGROOT = "../../../uginst";

# Counts the total number of #undefs emitted
$counter = 0;

# open output file
$outfilename = "ug_undefs.hh";
unless (open OUTFILE, ">$outfilename") {
  die "Couldn't open $outfilename for writing!\n";
}

# Loop over all the files given in the command line and
# recursively parse them for macro definitions.
foreach $file (@ARGV) {

  # Write header comment
  print OUTFILE "/** \\file\n";
  print OUTFILE "* \\brief Contains <tt>#undef</tt>s for all preprocessor macros\n";
  print OUTFILE "* defined by UG.\n";
  print OUTFILE "*\n";
  print OUTFILE "* This file is created automatically by the perl script <tt>undefAllMacros.pl</tt>.\n";
  print OUTFILE "*/\n\n";

  # Parse the file itself
  emitUndefs($file);

  # Makedepend recursively searches for more included headers.
  # Warnings appear because the system headers will not be found.
  # Those warnings get piped to /dev/null.
  foreach $_ (`makedepend -f- -w 20 -Y$UGROOT/include $file 2>/dev/null`) {
    
    # The output of makedepend has the form
    # file.o: header.h
    # We look for the ".o:" Everything that follows must be a header file  
    if (/\.o\:\s(\S+\.h)/) {
      
      emitUndefs($1);
      
    }
    
  }

}


print "Undef'ed $counter #defines!\n";

close(OUTFILE);


##############################################################
# This subroutine reads the file given as the first argument
# and issues an #undef for each macro found.
##############################################################
sub emitUndefs {

  print "Parsing $_[0]\n";

  unless (open INFILE, "$_[0]") {
    die "Couldn't open $_[0] for reading!\n";
}

  while(<INFILE>) {
    
    # looks for lines that define a C macro
    if (/\#define\s+(\w+)/) {
      
      print OUTFILE "#undef $1\n";
      $counter = $counter + 1;
      
    }
    
  }

  close (INFILE);

}
