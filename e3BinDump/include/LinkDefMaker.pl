#!/usr/bin/perl -Wall
#
# A little script to generate linkdef files for each of the 
# classes which needs a ROOT dictionary.
#

$argCount = $#ARGV + 1;

if($argCount < 1){
    print "You need an argument!";
}

my $fname = "LinkDefs.h";      
open(OPF, ">$fname");
print "$fname";
    
foreach $argNum (0 .. $#ARGV) 
{
    my $arg = $ARGV[$argNum];
    
    print "args $argNum    $arg";
    
    if($argNum==0){   
	print OPF "#ifdef __CINT__";
	print OPF "#pragma link off all globals;";
	print OPF "#pragma link off all classes;";
	print OPF "#pragma link off all functions;";
	print OPF "#pragma link C++ nestedclasses;";
	print OPF "#pragma link C++ nestedtypedefs;";
	print OPF "#pragma link C++ class $arg+;";
    } elsif($argNum>0) {
	print OPF "#pragma link C++ class $arg+;";
    }  
}

print OPF "#endif";
close(OPF);


