#! /usr/bin/env tclsh

proc pdf {build_dir tex_dir args} {
  cd $tex_dir
  return "![catch {exec >&/dev/tty pdflatex \
                   -output-director=$build_dir \
                   -output-format=pdf \
                   {*}$args}]"
}

proc bib {build_dir name} {
  cd $build_dir
  return "![catch {exec >&/dev/tty bibtex $name}]"
}

set cmd pdflatex

if {0 == $::argc} {
  puts "Usage: [file tail $::argv0] \[$cmd OPTS\] TEXFILE"
  exit 1
}

set run_dir "[file dirname $::argv0]"
set build_dir [file normalize "$run_dir/../build"]
set pub_dir [file normalize "$run_dir/../pub"]
set tex_dir [file normalize "$run_dir"]
set name [lindex $::argv end]
set bib _bib.bib
regsub {\.tex$} $name {} name

# multiple executions for proper referencing
file copy -force -- $tex_dir/_$name.xmpdata $build_dir/
file copy -force -- $tex_dir/$bib $build_dir/
pdf $build_dir $tex_dir "$::argv"
bib $build_dir $name
pdf $build_dir $tex_dir "$::argv"
pdf $build_dir $tex_dir "$::argv"
file rename -force -- $build_dir/$name.pdf $pub_dir/

