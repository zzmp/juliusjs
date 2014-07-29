#!/usr/bin/perl
# Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
# Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
# Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
#
# Generated automatically from mkdfa.pl.in by configure. 
#

## setup
# tmpdir
$usrtmpdir = "";		# specify if any

# mkfa executable location
($thisdir) = ($0 =~ /(.*(\/|\\))[^\/\\]*$/o);
$mkfabin = "${thisdir}mkfa";

# dfa_minimize executable location
$minimizebin = "${thisdir}dfa_minimize";
# find tmpdir
@tmpdirs = ($usrtmpdir, $ENV{"TMP"}, $ENV{"TEMP"}, "/tmp", "/var/tmp", "/WINDOWS/Temp", "/WINNT/Temp");

$tmpdir="";
while (@tmpdirs) {
    $t = shift(@tmpdirs);
    next if ($t eq "");
    if (-d "$t" && -w "$t") {
	$tmpdir = $t;
	last;
    }
}
if ($tmpdir eq "") {
    die "Please set working directory in \$usrtmpdir at $0\n";
}

#############################################################

if ($#ARGV < 0 || $ARGV[0] eq "-h") {
    usage();
}

$make_dict = 1;
$make_term = 1;

$CRLF = 0;

$gramprefix = "";
foreach $arg (@ARGV) {
    if ($arg eq "-t") {
	$make_term = 1;
    } elsif ($arg eq "-n") {
	$make_dict = 0;
    } else {
	$gramprefix = $arg;
    }
}
if ($gramprefix eq "") {
    usage();
}
$gramfile = "$ARGV[$#ARGV].grammar";
$vocafile = "$ARGV[$#ARGV].voca";
$dfafile  = "$ARGV[$#ARGV].dfa";
$dictfile = "$ARGV[$#ARGV].dict";
$termfile = "$ARGV[$#ARGV].term";
$tmpprefix = "$tmpdir/g$$";
$tmpvocafile = "${tmpprefix}.voca";
$rgramfile = "${tmpprefix}.grammar";

# generate reverse grammar file
open(GRAM,"< $gramfile") || die "cannot open \"$gramfile\"";
open(RGRAM,"> $rgramfile") || die "cannot open \"$rgramfile\"";
$n = 0;
while (<GRAM>) {
    chomp;
    $CRLF = 1 if /\r$/;
    s/\r+$//g;
    s/#.*//g;
    if (/^[ \t]*$/) {next;}
    ($left, $right) = split(/\:/);
    if ($CRLF == 1) {
	print RGRAM $left, ': ', join(' ', reverse(split(/ /,$right))), "\r\n";
    } else {
	print RGRAM $left, ': ', join(' ', reverse(split(/ /,$right))), "\n";
    }
    $n ++;
}
close(GRAM);
close(RGRAM);
print "$gramfile has $n rules\n";

# make temporary voca for mkfa (include only category info)
if (! -r $vocafile) {
	die "cannot open voca file $vocafile";
}
open(VOCA,"$vocafile") || die "cannot open vocabulary file";
open(TMPVOCA,"> $tmpvocafile") || die "cannot open temporary file $tmpvocafile";
if ($make_term == 1) {
    open(GTERM, "> $termfile");
}
$n1 = 0;
$n2 = 0;
$termid = 0;
while (<VOCA>) {
    chomp;
    $CRLF = 1 if /\r$/;
    s/\r+$//g;
    s/#.*//g;
    if (/^[ \t]*$/) {next;}
    if (/^%[ \t]*([A-Za-z0-9_]*)/) {
	if ($CRLF == 1) {
	    printf(TMPVOCA "\#%s\r\n", $1);
	} else {
	    printf(TMPVOCA "\#%s\n", $1);
	}
	if ($make_term == 1) {
	    if ($CRLF == 1) {
		printf(GTERM "%d\t%s\r\n",$termid, $1);
	    } else {
		printf(GTERM "%d\t%s\n",$termid, $1);
	    }
	    $termid++;
	}
	$n1++;
    } else {
	$n2++;
    }
}
close(VOCA);
close(TMPVOCA);
if ($make_term == 1) {
    close(GTERM);
}
print "$vocafile    has $n1 categories and $n2 words\n";

print "---\n";

# call mkfa and make .dfa
if (! -x $minimizebin) {
    # no minimization
    print "Warning: dfa_minimize not found in the same place as mkdfa.pl\n";
    print "Warning: no minimization performed\n";
    if ($tmpprefix =~ /cygdrive/) {
	$status = system("$mkfabin -e1 -fg `cygpath -w $rgramfile` -fv `cygpath -w $tmpvocafile` -fo `cygpath -w $dfafile` -fh `cygpath -w ${tmpprefix}.h`");
    } else {
	$status = system("$mkfabin -e1 -fg $rgramfile -fv $tmpvocafile -fo $dfafile -fh ${tmpprefix}.h");
    }
} else {
    # minimize DFA after generation
    if ($tmpprefix =~ /cygdrive/) {
	$status = system("$mkfabin -e1 -fg `cygpath -w $rgramfile` -fv `cygpath -w $tmpvocafile` -fo `cygpath -w ${dfafile}.tmp` -fh `cygpath -w ${tmpprefix}.h`");
	system("$minimizebin `cygpath -w ${dfafile}.tmp` -o `cygpath -w $dfafile`");
    } else {
	$status = system("$mkfabin -e1 -fg $rgramfile -fv $tmpvocafile -fo ${dfafile}.tmp -fh ${tmpprefix}.h");
	system("$minimizebin ${dfafile}.tmp -o $dfafile");
    }
    unlink("${dfafile}.tmp");
}
unlink("$rgramfile");
unlink("$tmpvocafile");
unlink("${tmpprefix}.h");
print "---\n";
if ($status != 0) {
    # error
    print "no .dfa or .dict file generated\n";
    exit;
}

# convert .voca -> .dict
# terminal number should be ordered by voca at mkfa output
if ($make_dict == 1) {
    $nowid = -1;
    open(VOCA, "$vocafile")  || die "No vocafile \"$vocafile\" found.\n";
    open(DICT, "> $dictfile") || die "cannot open $dictfile for writing.\n";
    while (<VOCA>) {
	chomp;
	s/\r//g;
	s/#.*//g;
	if (/^[ \t]*$/) {next;}
	if (/^%/) {
	    $nowid++;
	    next;
	} else {
	    @a = split;
	    $name = shift(@a);
	    if ($CRLF == 1) {
		printf(DICT "%d\t[%s]\t%s\r\n", $nowid, $name, join(' ', @a));
	    } else {
		printf(DICT "%d\t[%s]\t%s\n", $nowid, $name, join(' ', @a));
	    }
	}
    }
    close(VOCA);
    close(DICT);
}

$gene = "$dfafile";
if ($make_term == 1) {
    $gene .= " $termfile";
}
if ($make_dict == 1) {
    $gene .= " $dictfile";
}
print "generated: $gene\n";

sub usage {
    print "mkdfa.pl --- DFA compiler\n";
    print "usage: $0 [-n] prefix\n";
    print "\t-n ... keep current dict, not generate\n";
    exit;
}
