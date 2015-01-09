use Module::ScanDeps;
use Data::Dumper;
use File::Path qw(make_path remove_tree);
use File::Basename;
use File::Copy;

if (-d "./perl") {
	print "perl dependencies were already computed and copied\n"
} else {
	print "Computing perl dependencies...\n";
	# standard usage
	my $hash_ref = scan_deps(
		files   => [ 'exiftool_for_scan.pl' ],
		recurse => 1,
	);
	print "Copying needed perl files...\n";

	#print Dumper($hash_ref);
	foreach my $key ( keys %$hash_ref )
	{
	  make_path(dirname("./perl/$key"));
	  copy("$$hash_ref{$key}{'file'}", "./perl/$key");
	}

	#Those two are not detected by scan_deps
	copy("C:/Strawberry/perl/lib/bytes.pm", "./perl/bytes.pm");
	copy("C:/Strawberry/perl/lib/PerlIO.pm", "./perl/PerlIO.pm");
}