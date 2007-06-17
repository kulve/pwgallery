#!/usr/bin/perl -w

use strict;

my %GALLERY;       # hash for data
my $img_count = 0; # image counter

my $TEMPL_INDEX    = "";
my $TEMPL_INDEXIMG = "";
my $TEMPL_INDEXGEN = "";
my $TEMPL_IMAGE    = "";
my $TEMPL_GEN      = ""

if (not defined $ARGV[0] or not defined $ARGV[1]) {
  print "Usage: $0 gwebgalleryfile pwgalleryfile.xml\n";
  exit(1);
}

my $GWG = $ARGV[0];
my $PWG = $ARGV[1];

print "Converting from $GWG to $PWG\n";

if (-e $PWG) {
  print "$PWG exists! Stopping.\n";
  exit(1);
}


my $gwg = load_file($GWG);

if ($gwg !~ m/<GALLERY>/ or $gwg !~ m/<\/GALLERY>/) {
  print "No GALLERY tags found in $GWG! Exiting.\n";
  exit(1);
}

my $main = $1
	if ($gwg =~ m/<MAIN>(.*)<\/MAIN>/s);

die ("Failed to parse MAIN\n")
	unless (defined $main);

parse_main($main);

die ("Failed to parse gallery name from MAIN\n")
	unless (defined $GALLERY{"MAIN"}->{"name"});

while ($gwg =~ m/<IMAGE>(.*?)<\/IMAGE>/sg) {
  parse_image($1);
}


print_pwgallery();

exit(0);


sub load_file {
  my $f = shift;

  die ("load_file: file not defined\n")
	  unless (defined $f);

  local $/;
  
  open (F, "<$f")
	  or die ("Failed to open $f: $!\n");

  my $content = <F>;

  close (F);

  return $content;
}



sub parse_main {
  my $m = shift;

  die ("parse_main: main not defined\n")
	  unless (defined $m);

  $GALLERY{"MAIN"}->{"name"} = $1
	  if ($m =~ m/name = (.*)/);
  $GALLERY{"MAIN"}->{"version"} = $1
	  if ($m =~ m/version = (.*)/);
  $GALLERY{"MAIN"}->{"thumb_width"} = $1
	  if ($m =~ m/thumb_width = (.*)/);
  $GALLERY{"MAIN"}->{"image_height"} = $1
	  if ($m =~ m/image_height = (.*)/);
  $GALLERY{"MAIN"}->{"outputdir"} = $1
	  if ($m =~ m/outputdir = (.*)/);
  $GALLERY{"MAIN"}->{"templ_index"} = $1
	  if ($m =~ m/templ_index = (.*)/);
  $GALLERY{"MAIN"}->{"templ_image"} = $1
	  if ($m =~ m/templ_image = (.*)/);
}



sub parse_image {
  my $i = shift;

  die ("parse_image: image not defined\n")
	  unless (defined $i);

  $img_count++;

  $GALLERY{"IMG"}->{$img_count}->{"filename"} = $1
	  if ($i =~ m/filename = (.*)/);
  $GALLERY{"IMG"}->{$img_count}->{"rotation"} = $1
	  if ($i =~ m/rotation = (.*)/);
  $GALLERY{"IMG"}->{$img_count}->{"gamma"} = $1
	  if ($i =~ m/gamma = (.*)/);
  $GALLERY{"IMG"}->{$img_count}->{"nomodify"} = $1
	  if ($i =~ m/nomodify = (.*)/);
  $GALLERY{"IMG"}->{$img_count}->{"desc"} = $1
	  if ($i =~  m/<DESCRIPTION>(.*)<\/DESCRIPTION>/s);

  # Remove "Add text"
  $GALLERY{"IMG"}->{$img_count}->{"desc"} =~ s/Add text//g;

  # strip
  $GALLERY{"IMG"}->{$img_count}->{"desc"} =~ s/^\s+|\s+$//g;

  # Convert "<br>" to newline
  $GALLERY{"IMG"}->{$img_count}->{"desc"} =~ s/<br>/\n/g;

  # &deg; -> &#xB0;
  $GALLERY{"IMG"}->{$img_count}->{"desc"} =~ s/\&deg;/\&\#xB0;/g;

}



sub print_pwgallery {

  open (OUTPUT, ">$PWG")
	  or die ("Failed to open $PWG: $!");

  print "Gallery: $GALLERY{MAIN}->{name}\n";

  print OUTPUT "<?xml version=\"1.0\"?>\n";
  print OUTPUT "<pwgallery>\n";
  print OUTPUT "  <settings>\n";

  print OUTPUT "    <version>$GALLERY{MAIN}->{version}</version>\n";
  print OUTPUT "    <name>$GALLERY{MAIN}->{name}</name>\n";

  my ($output_dir, $dir_name) = get_dirs($GALLERY{"MAIN"}->{"outputdir"});

  print OUTPUT "    <dir_name>$dir_name</dir_name>\n";
  print OUTPUT "    <output_dir>file://$output_dir</output_dir>\n";

  print OUTPUT "    <page_gen_prog></page_gen_prog>\n";
  print OUTPUT "    <templ_index>$TEMPL_INDEX</templ_index>\n";
  print OUTPUT "    <templ_indeximg>$TEMPL_INDEXIMG</templ_indeximg>\n";
  print OUTPUT "    <templ_indexgen>$TEMPL_INDEXGEN</templ_indexgen>\n";
  print OUTPUT "    <templ_image>$TEMPL_IMAGE</templ_image>\n";
  print OUTPUT "    <templ_gen>$TEMPL_GEN</templ_gen>\n";
  print OUTPUT "    <page_gen>1</page_gen>\n";

  print OUTPUT "    <thumb_w>$GALLERY{MAIN}->{thumb_width}</thumb_w>\n";
  print OUTPUT "    <image_h>$GALLERY{MAIN}->{image_height}</image_h>\n";
  print OUTPUT "    <image_h2>0</image_h2>\n";
  print OUTPUT "    <image_h3>0</image_h3>\n";
  print OUTPUT "    <image_h4>0</image_h4>\n";

  print OUTPUT "    <edited>false</edited>\n";
  print OUTPUT "    <remove_exif>true</remove_exif>\n";
  print OUTPUT "    <rename>false</rename>\n";
  print OUTPUT "  </settings>\n";
  print OUTPUT "  <pages>\n";

  foreach my $i (sort { $a <=> $b } keys %{$GALLERY{"IMG"}}) {
	print OUTPUT "    <image>\n";
	print OUTPUT "      <settings>\n";
	print OUTPUT "        <text>$GALLERY{IMG}->{$i}->{desc}</text>\n";
	print OUTPUT "        <uri>file://$GALLERY{IMG}->{$i}->{filename}</uri>\n";
	print OUTPUT "        <gamma>$GALLERY{IMG}->{$i}->{gamma}</gamma>\n";
	print OUTPUT "        <rotate>$GALLERY{IMG}->{$i}->{rotation}</rotate>\n";
	my $nomodify = "false";
	$nomodify = "true"
		if (defined $GALLERY{"IMG"}->{$i}->{"nomodify"} and 
			$GALLERY{"IMG"}->{$i}->{"nomodify"} == 1);
	print OUTPUT "        <nomodify>$nomodify</nomodify>\n";
	print OUTPUT "      </settings>\n";
	print OUTPUT "    </image>\n";
  }

  print OUTPUT "  </pages>\n";
  print OUTPUT "</pwgallery>\n";

}


sub get_dirs {
  my $dir = shift;

    die ("get_dirs: dir not defined\n")
	  unless (defined $dir);

  my @dirs = split('/', $dir);
  
  my $dir_name = pop(@dirs);
  my $output_dir = join('/', @dirs);
  
  return ($output_dir, $dir_name);
}
