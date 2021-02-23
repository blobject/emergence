#! /usr/bin/env tclsh

# pdf.tcl: Build a PDF file from a LaTeX file using pdflatex.
#          The building takes place in a separate directory and the final
#          PDF file is moved to the publishing directory.

if {0 == $::argc} {
  puts "Usage: [file tail $::argv0] \[OPTS\] TEXFILE"
  exit 1
}

set cmd pdflatex
set bdir ../build
set pdir [file dirname $::argv0]
set name [lindex $::argv end]
regsub {\.tex$} $name {} name

if {{relative} eq [file pathtype $::argv0]} {
  set pdir [file normalize "[pwd]/$pdir"]
}
cd $pdir

if {![catch {exec $cmd \
             -output-directory=$bdir \
             -output-format=pdf \
             {*}$::argv}]} {
  file rename -force -- $bdir/$name.pdf $pdir
}

