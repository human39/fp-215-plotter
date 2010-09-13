#!/usr/bin/perl -w

# RSGL to screen
# Released under GPL
# Henry Palonen, h@yty.net
# Modified for RSGL (TRS-80 FP-215) support by Matt Boytim, maboytim@yahoo.com
# 
# Program draws RSGL-files to screen using Gtk-Perl.

use Gtk;
use strict;

init Gtk;
#set_locale Gtk;

my $false = 0;
my $true = 1;

my $window;
my $drawing_area;
my $button;
my $bgcolor;
my $gdkcolor;
my $vbox;
# FP-215 plotting range is 2980x2160 (11.73*254 x 8.5*254)
# set display plotting area as 2980x2160/scale and adjust plotting coordinates by scale for display only
my $scale=3;
my $dxs=int 2980/$scale;
my $dys=int 2160/$scale;
my $check;
my $separator;
my $file;
my $entry;
my $hbox;
my $coord=$false;
my $progress;
my $secs=0;

$window = new Gtk::Window( 'toplevel' );
$window->set_policy( $false, $false, $false );
$window->set_usize( $dxs+50, $dys+200 );
$window->border_width( 15 );
$window->set_title( "Draw from RSGL" );
$window->signal_connect( 'destroy', sub { Gtk->exit( 0 ); } );

$vbox = new Gtk::VBox( $false, 5 );
$vbox->border_width( 10 );
$window->add( $vbox );
$vbox->show();

$hbox = new Gtk::HBox( $true, 5 );
$hbox->border_width( 10 );
$vbox->pack_start( $hbox, $false, $false, 0 );
$hbox->show();

$button = new Gtk::Button( "Select file" );
$button->signal_connect( "clicked", \&select_file );
$hbox->pack_start( $button, $false, $true, 0 );
$button->show();

$entry = new Gtk::Entry();
$hbox->pack_start( $entry, $false, $true, 0 );
$entry->show();

$hbox = new Gtk::HBox( $true, 5 );
$hbox->border_width( 10 );
$vbox->pack_start( $hbox, $false, $false, 0 );
$hbox->show();

$button = new Gtk::Button( "Paint RSGL picture" );
$button->signal_connect( "clicked", \&draw_shapes );
$hbox->pack_start( $button, $false, $true, 0 );
$button->show();

$progress = new Gtk::ProgressBar();
$hbox->pack_start( $progress, $false, $true, 0 );
$progress->show();

$separator = new Gtk::HSeparator();
$vbox->pack_start( $separator, $false, $false, 0 );
$separator->show();

$hbox = new Gtk::HBox( $true, 5 );
$hbox->border_width( 10 );
$vbox->pack_start( $hbox, $false, $false, 0 );
$hbox->show();

$check = new Gtk::CheckButton( "Print RSGL to STDOUT");
$check->signal_connect("clicked", \&toggle_coord);
$hbox->pack_start( $check, $false, $false, 0 );
$check->show();

my $label = new Gtk::Label("Scale % (25-400)");
$hbox->pack_start( $label, $false, $false, 0);
$label->show();

my $adj = new Gtk::Adjustment( 100.0, 25.0, 400.0, 0.5, 4.0, 0.0 );
my $spinner = new Gtk::SpinButton( $adj, 0.5, 1 );
$spinner->set_wrap( $true );
$hbox->pack_start( $spinner, $false, $true, 0);
$spinner->show();

$separator = new Gtk::HSeparator();
$vbox->pack_start( $separator, $false, $false, 0 );
$separator->show();

$drawing_area = new Gtk::DrawingArea();
$drawing_area->size( $dxs, $dys );
$vbox->pack_start( $drawing_area ,$false, $false, 0);

$button->show();
$drawing_area->show();
$window->show();

main Gtk;
exit( 0 );

### Subroutines

sub select_file
{

	my $file_dialog = new Gtk::FileSelection( "Select RSGL file to plot (*.PLX)");
	$file_dialog->signal_connect( "destroy", sub { Gtk->exit( 0 ); } );
	
	# Connect the ok_button to file_ok_sel function
	$file_dialog->ok_button->signal_connect( "clicked", \&file_ok_sel, $file_dialog );
	$file_dialog->cancel_button->signal_connect( "clicked", sub { $file_dialog->hide(); } );
	$file_dialog->complete( "*.PLX" );
	$file_dialog->show();
}

sub file_ok_sel
{
 	my ( $widget, $file_sel ) = @_;
	$file = $file_sel->get_filename();
#	print( "$file\n" );
	$entry->set_text("$file");
	$file_sel->hide;
}

sub toggle_coord
{
	if ($coord eq $true)
	{
		$coord=$false;
	}
	else 
	{
		$coord=$true;
	}
}
sub draw_shapes
  {
    $bgcolor = Gtk::Gdk::Color->parse_color( 'white' );
    $bgcolor = $drawing_area->window->get_colormap()->color_alloc( $bgcolor );
    $drawing_area->window->set_background( $bgcolor );

    my $drawable = $drawing_area->window;

    my $xyS = 100 / ($adj->value);
	
    my $white_gc = $drawing_area->style->white_gc;
    my $black_gc = $drawing_area->style->black_gc;
    
    $drawable->clear; 
   
  	my $FILE = "$file";
  	open(FILE, "$FILE") or die "Can't open PLX file $FILE: $!\n";
	if ($coord eq $true) {print "F1\rI0,0\rL0\r";}
	while (<FILE>)
	{
		my $row = $_;
		my $x=0;
		my $y=0;
		my $lastX;
		my $lastY;
		my $rsx;
		my $rsy;
		my $mode="", my $comma=0;
		my $prevH=0;
		while ($row =~ m/\r/g)
		{
			my $nextH = pos $row;
			my $len = $nextH - $prevH;
			my $cmd = substr($row,$prevH,$len-1);
			if ($cmd ne "")
			{

				if (substr($cmd,0,1) eq "D") {$mode = "D";}
				elsif (substr($cmd,0,1) eq "M") {$mode = "M";}

				if ($cmd =~ m/,/g) 
				{
					$lastX = $x;
					$lastY = $y;
					$comma = pos $cmd;
					$x = substr($cmd, 1, $comma - 2);
					$y = substr($cmd, $comma);
					$x = $x / $xyS;
					$y = $y / $xyS;
					$rsx = int $x;
					$rsy = int $y;
					$x = $x / $scale;
					$y = $y / $scale;

					if ($mode eq "D")
					{
						if ($coord eq $true) {print "D$rsx,$rsy\r";}
   						$drawable->draw_line( $black_gc, $x, $dys-$y, $lastX, $dys-$lastY );
					}
					elsif($mode eq "M")
					{
						if ($coord eq $true) {print "M$rsx,$rsy\r"; }
   						$drawable->draw_point( $black_gc, $x, $dys-$y);
					}
				}
				else
				{
					$comma = 0;
				}
			}
			$prevH=$nextH;
		}
	}
	if ($coord eq $true) {print "H\r";}

	return $true;
  }
