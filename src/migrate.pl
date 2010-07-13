#!/usr/bin/perl -w
use warnings;
use strict;
use File::Glob;
use File::Copy;

my @files = glob('*.[ch]') or die;
my @regexes;

my $zomg = join("====", @files);
$zomg =~ s/====SDL_extras\.[ch]//g;
print $zomg;

@files = split("====", $zomg);

print "Enter unprefixed function name or 'bye':\n";
while (my $regex = <STDIN>) {
	last if ($regex =~ m/^(bye|exit|q|q$)/);
	$regex =~ s/\s+$//; #trim trailing whitespace
	push(@regexes, $regex); #add to list of replacement candidates
	print "Enter unprefixed function name or 'bye':\n";
}

foreach my $file (@files) {
	open(INFILE, $file) or die ("Can't open $file");
	open(OUTFILE, ">", "$file.new") or die ("Can't write new file - $!\n");
	my $matches = 0;
	print "Processing $file...";
	while (my $line = <INFILE>) {
		foreach my $regex (@regexes) {
			if ($line =~ m#[^\w]($regex)\(#) {
				#print ":$line\n";
				if ($line =~ s#$regex\(#T4K_$regex\(#g ) {
					$matches++;
				}
			}
		}
		print OUTFILE "$line";
	}
	close INFILE;
	close OUTFILE;
	
	if ($matches > 0) {
		print "$matches replacements";
		copy("$file.new", $file) or die "Copy failed: $1";
	}
	print "\n";
}	

print "Done";