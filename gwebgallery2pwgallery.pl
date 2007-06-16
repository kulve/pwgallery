#!/usr/bin/perl -w

use strict;

my %GALLERY;       # hash for data
my $img_count = 0; # image counter

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

  # strip
   $GALLERY{"IMG"}->{$img_count}->{"desc"} =~ s/^\s+|\s+$//g;
}



sub print_pwgallery {

  print "Gallery: $GALLERY{MAIN}->{name}\n";

  print "<?xml version=\"1.0\"?>\n";
  print "<pwgallery>\n";
  print "  <settings>\n";

  print "    <version>$GALLERY{MAIN}->{version}</version>\n";
  print "    <name>$GALLERY{MAIN}->{name}</name>\n";

  my ($output_dir, $dir_name) = get_dirs($GALLERY{"MAIN"}->{"outputdir"});

  print "    <dir_name>$dir_name</dir_name>\n";
  print "    <output_dir>file://$output_dir</output_dir>\n";

  print "    <page_gen_prog></page_gen_prog>\n";
  print "    <templ_index></templ_index>\n";
  print "    <templ_indeximg></templ_indeximg>\n";
  print "    <templ_indexgen></templ_indexgen>\n";
  print "    <templ_image></templ_image>\n";
  print "    <templ_gen></templ_gen>\n";
  print "    <page_gen>1</page_gen>\n";

  print "    <thumb_w>$GALLERY{MAIN}->{thumb_width}</thumb_w>\n";
  print "    <image_h>$GALLERY{MAIN}->{image_height}</image_h>\n";
  print "    <image_h2>0</image_h2>\n";
  print "    <image_h3>0</image_h3>\n";
  print "    <image_h4>0</image_h4>\n";

  print "    <edited>false</edited>\n";
  print "    <remove_exif>true</remove_exif>\n";
  print "    <rename>false</rename>\n";
  print "  </settings>\n";
  print "  <pages>\n";

  foreach my $i (keys %{$GALLERY{"IMG"}}) {
	print "    <image>\n";
	print "      <settings>\n";
	print "        <text>$GALLERY{IMG}->{$i}->{desc}</text>\n";
	print "        <uri>file://$GALLERY{IMG}->{$i}->{filename}</uri>\n";
	print "        <gamma>$GALLERY{IMG}->{$i}->{gamma}</gamma>\n";
	print "        <rotate>$GALLERY{IMG}->{$i}->{rotation}</rotate>\n";
	my $nomodify = "false";
	$nomodify = "true"
		if ($GALLERY{"IMG"}->{$i}->{"nomodify"} == 1);
	print "        <nomodify>$nomodify</nomodify>\n";
	print "      </settings>\n";
	print "    </image>\n";
  }

  print "  </pages>\n";
  print "</pwgallery>\n";

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
